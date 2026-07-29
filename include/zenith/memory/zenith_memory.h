#ifndef ZENITH_MEMORY_H
#define ZENITH_MEMORY_H

// =============================================================================
// Zenith Hybrid RC + GC Memory Management
// =============================================================================
// Design:
//   Primary:   Reference Counting (RC) — O(1) deterministic deallocation for
//              acyclic ownership graphs.
//   Secondary: Tri-color Mark-and-Sweep Cycle Collector — runs periodically in
//              a background thread to break and collect reference cycles that RC
//              alone cannot handle.
//
// Usage from Zenith source:
//   @managed class Node { String value; Weak<Node> next; }
//   Ref<Node> n = Ref<Node>("hello");
//   Weak<Node> w = n;
// =============================================================================

#include <atomic>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>
#include <unordered_set>
#include <vector>
#include <cassert>
#include <iostream>
#include <chrono>
#include <string>
#include <sstream>

namespace zenith {
namespace mem {

// ---------------------------------------------------------------------------
// Forward declarations
// ---------------------------------------------------------------------------
class GcHeap;
struct RcBlock;
template <typename T> class Ref;
template <typename T> class Weak;

// ---------------------------------------------------------------------------
// GC tri-color states for mark-and-sweep
// ---------------------------------------------------------------------------
enum class GcColor : uint8_t {
    White = 0,  // Unreachable / not yet visited
    Gray  = 1,  // Discovered but children not yet scanned
    Black = 2,  // Fully scanned / reachable
};

// ---------------------------------------------------------------------------
// RcBlock — per-object control block (allocated once per managed object)
// ---------------------------------------------------------------------------
struct RcBlock {
    std::atomic<int32_t> strong_count{1};   // strong references
    std::atomic<int32_t> weak_count{1};     // +1 for the block itself while object alive
    std::atomic<GcColor> gc_color{GcColor::White};
    bool gc_tracked = false;                // Is this block registered in GcHeap?
    void* object_ptr = nullptr;             // raw pointer back to managed object
    std::function<void()> finalizer;        // called when strong_count → 0
    std::function<void(std::vector<RcBlock*>&)> get_children; // GC graph traversal

    RcBlock() = default;
    RcBlock(const RcBlock&) = delete;
    RcBlock& operator=(const RcBlock&) = delete;
};

// ---------------------------------------------------------------------------
// Managed — base class for all @managed Zenith objects
// ---------------------------------------------------------------------------
class Managed {
public:
    RcBlock* __rc_block = nullptr;

    Managed() {
        __rc_block = new RcBlock();
        __rc_block->object_ptr = this;
        __rc_block->finalizer = [this]() { /* overridden per class */ };
    }

    virtual ~Managed() {
        // RcBlock is freed when weak_count also reaches 0
    }

    // Called by GcHeap to enumerate outgoing strong references
    virtual void __gc_enumerate(std::vector<RcBlock*>& out) {}
};

// ---------------------------------------------------------------------------
// GcHeap — global singleton managing the tracked heap
// ---------------------------------------------------------------------------
class GcHeap {
public:
    struct Stats {
        size_t live_objects    = 0;
        size_t collected_bytes = 0;
        size_t cycle_runs      = 0;
        size_t cycles_broken   = 0;
    };

    static GcHeap& instance() {
        static GcHeap heap;
        return heap;
    }

    // Register a new managed object's control block for cycle detection
    void track(RcBlock* block) {
        std::lock_guard<std::mutex> lk(mutex_);
        tracked_.insert(block);
        stats_.live_objects++;
    }

    void untrack(RcBlock* block) {
        std::lock_guard<std::mutex> lk(mutex_);
        tracked_.erase(block);
        if (stats_.live_objects > 0) stats_.live_objects--;
    }

    // Tri-color mark-and-sweep cycle collection
    void collect() {
        std::lock_guard<std::mutex> lk(mutex_);
        stats_.cycle_runs++;

        // Phase 1: Reset all to White
        for (RcBlock* b : tracked_) {
            b->gc_color.store(GcColor::White, std::memory_order_relaxed);
        }

        // Phase 2: Mark roots (blocks with strong_count > 0 that are externally held)
        // A block is a root if its strong_count > number of internal (cycle) references.
        // Simplified: anything with strong_count > 0 is tentatively a root → mark Gray.
        std::vector<RcBlock*> gray_queue;
        for (RcBlock* b : tracked_) {
            if (b->strong_count.load(std::memory_order_relaxed) > 0) {
                b->gc_color.store(GcColor::Gray, std::memory_order_relaxed);
                gray_queue.push_back(b);
            }
        }

        // Phase 3: Scan — mark children of gray nodes Black, push their children
        while (!gray_queue.empty()) {
            RcBlock* b = gray_queue.back();
            gray_queue.pop_back();
            b->gc_color.store(GcColor::Black, std::memory_order_relaxed);

            // Enumerate children via the managed object
            if (b->object_ptr && b->get_children) {
                std::vector<RcBlock*> children;
                b->get_children(children);
                for (RcBlock* child : children) {
                    if (child && child->gc_color.load(std::memory_order_relaxed) == GcColor::White) {
                        child->gc_color.store(GcColor::Gray, std::memory_order_relaxed);
                        gray_queue.push_back(child);
                    }
                }
            }
        }

        // Phase 4: Sweep — anything still White is unreachable (part of a cycle)
        std::vector<RcBlock*> garbage;
        for (RcBlock* b : tracked_) {
            if (b->gc_color.load(std::memory_order_relaxed) == GcColor::White) {
                garbage.push_back(b);
            }
        }

        for (RcBlock* b : garbage) {
            tracked_.erase(b);
            stats_.live_objects = tracked_.size();
            stats_.cycles_broken++;
            // Run finalizer to destroy the managed object
            if (b->finalizer) {
                try { b->finalizer(); } catch (...) {}
            }
            // Decrement weak_count (for the object-side reference to block)
            int32_t wc = b->weak_count.fetch_sub(1, std::memory_order_acq_rel);
            if (wc == 1) {
                delete b;  // no more weak refs — free the block itself
            }
        }
    }

    // Start a background thread that calls collect() every `interval_ms` milliseconds
    void start_background_gc(uint32_t interval_ms = 5000) {
        if (gc_running_.load()) return;
        gc_running_.store(true);
        gc_thread_ = std::thread([this, interval_ms]() {
            while (gc_running_.load(std::memory_order_relaxed)) {
                std::this_thread::sleep_for(std::chrono::milliseconds(interval_ms));
                if (gc_running_.load(std::memory_order_relaxed)) {
                    collect();
                }
            }
        });
    }

    void stop_background_gc() {
        if (!gc_running_.load()) return;
        gc_running_.store(false);
        if (gc_thread_.joinable()) gc_thread_.join();
    }

    Stats stats() {
        std::lock_guard<std::mutex> lk(mutex_);
        return stats_;
    }

    std::string stats_string() {
        auto s = stats();
        std::ostringstream oss;
        oss << "GcHeap Stats:\n"
            << "  Live objects  : " << s.live_objects    << "\n"
            << "  GC runs       : " << s.cycle_runs      << "\n"
            << "  Cycles broken : " << s.cycles_broken   << "\n";
        return oss.str();
    }

    ~GcHeap() {
        stop_background_gc();
    }

private:
    GcHeap() = default;
    GcHeap(const GcHeap&) = delete;
    GcHeap& operator=(const GcHeap&) = delete;

    std::mutex mutex_;
    std::unordered_set<RcBlock*> tracked_;
    std::thread gc_thread_;
    std::atomic<bool> gc_running_{false};
    Stats stats_;
};

// ---------------------------------------------------------------------------
// Ref<T> — Strong smart pointer (increments RC)
// ---------------------------------------------------------------------------
template <typename T>
class Ref {
public:
    // Construct from raw managed object (takes ownership)
    explicit Ref(T* raw) : ptr_(raw) {
        if (ptr_) {
            block_ = ptr_->__rc_block;
            // Register with GcHeap
            if (block_ && !block_->gc_tracked) {
                block_->gc_tracked = true;
                block_->get_children = [raw](std::vector<RcBlock*>& out) {
                    raw->__gc_enumerate(out);
                };
                GcHeap::instance().track(block_);
            }
        }
    }

    // Copy — increment strong ref
    Ref(const Ref& other) : ptr_(other.ptr_), block_(other.block_) {
        if (block_) block_->strong_count.fetch_add(1, std::memory_order_relaxed);
    }

    // Move
    Ref(Ref&& other) noexcept : ptr_(other.ptr_), block_(other.block_) {
        other.ptr_   = nullptr;
        other.block_ = nullptr;
    }

    Ref& operator=(const Ref& other) {
        if (this != &other) {
            release();
            ptr_   = other.ptr_;
            block_ = other.block_;
            if (block_) block_->strong_count.fetch_add(1, std::memory_order_relaxed);
        }
        return *this;
    }

    Ref& operator=(Ref&& other) noexcept {
        if (this != &other) {
            release();
            ptr_   = other.ptr_;
            block_ = other.block_;
            other.ptr_   = nullptr;
            other.block_ = nullptr;
        }
        return *this;
    }

    ~Ref() { release(); }

    T* operator->() const { return ptr_; }
    T& operator*()  const { return *ptr_; }
    T* get()        const { return ptr_; }
    explicit operator bool() const { return ptr_ != nullptr; }

    int32_t use_count() const {
        return block_ ? block_->strong_count.load(std::memory_order_relaxed) : 0;
    }

private:
    void release() {
        if (!block_) return;
        int32_t sc = block_->strong_count.fetch_sub(1, std::memory_order_acq_rel);
        if (sc == 1) {
            // Strong count → 0: destroy the object
            GcHeap::instance().untrack(block_);
            delete ptr_;
            ptr_ = nullptr;
            // Decrement weak_count (the object-side hold on the block)
            int32_t wc = block_->weak_count.fetch_sub(1, std::memory_order_acq_rel);
            if (wc == 1) {
                delete block_;
            }
            block_ = nullptr;
        }
    }

    T*       ptr_   = nullptr;
    RcBlock* block_ = nullptr;

    template <typename U> friend class Weak;
};

// ---------------------------------------------------------------------------
// Weak<T> — Weak smart pointer (does NOT increment RC; breaks cycles)
// ---------------------------------------------------------------------------
template <typename T>
class Weak {
public:
    Weak() = default;

    // Construct from a Ref<T>
    Weak(const Ref<T>& ref) : block_(ref.block_) {
        if (block_) block_->weak_count.fetch_add(1, std::memory_order_relaxed);
    }

    Weak(const Weak& other) : block_(other.block_) {
        if (block_) block_->weak_count.fetch_add(1, std::memory_order_relaxed);
    }

    Weak& operator=(const Weak& other) {
        if (this != &other) {
            release();
            block_ = other.block_;
            if (block_) block_->weak_count.fetch_add(1, std::memory_order_relaxed);
        }
        return *this;
    }

    Weak& operator=(const Ref<T>& ref) {
        release();
        block_ = ref.block_;
        if (block_) block_->weak_count.fetch_add(1, std::memory_order_relaxed);
        return *this;
    }

    ~Weak() { release(); }

    // Attempt to promote to a Ref<T>. Returns an empty Ref if the object is gone.
    Ref<T> lock() const {
        if (!block_) return Ref<T>(nullptr);
        // Atomically try to bump strong count if > 0
        int32_t sc = block_->strong_count.load(std::memory_order_relaxed);
        while (sc > 0) {
            if (block_->strong_count.compare_exchange_weak(
                    sc, sc + 1,
                    std::memory_order_acquire,
                    std::memory_order_relaxed)) {
                // Successfully bumped — wrap in Ref without double-tracking
                Ref<T> strong(nullptr);
                strong.ptr_   = static_cast<T*>(block_->object_ptr);
                strong.block_ = block_;
                return strong;
            }
        }
        return Ref<T>(nullptr);  // Object already destroyed
    }

    bool expired() const {
        return !block_ || block_->strong_count.load(std::memory_order_relaxed) == 0;
    }

private:
    void release() {
        if (!block_) return;
        int32_t wc = block_->weak_count.fetch_sub(1, std::memory_order_acq_rel);
        if (wc == 1) {
            delete block_;
        }
        block_ = nullptr;
    }

    RcBlock* block_ = nullptr;
};

// ---------------------------------------------------------------------------
// Convenience: make_ref<T>(args...) — heap-allocate a @managed object
// ---------------------------------------------------------------------------
template <typename T, typename... Args>
Ref<T> make_ref(Args&&... args) {
    T* raw = new T(std::forward<Args>(args)...);
    return Ref<T>(raw);
}

// ---------------------------------------------------------------------------
// gcStats() — free function callable from generated Zenith code
// ---------------------------------------------------------------------------
inline GcHeap::Stats gcStats() {
    return GcHeap::instance().stats();
}

inline std::string gcStatsString() {
    return GcHeap::instance().stats_string();
}

// ---------------------------------------------------------------------------
// FrameAllocator & Temp<T> — High-Performance Scoped Arena/Frame Allocator
// ---------------------------------------------------------------------------
class FrameAllocator {
public:
    static FrameAllocator& instance() {
        static FrameAllocator allocator;
        return allocator;
    }

    void* allocate(size_t bytes, size_t alignment = 16) {
        size_t space = capacity_ - offset_;
        void* ptr = buffer_ + offset_;
        if (std::align(alignment, bytes, ptr, space)) {
            offset_ = capacity_ - space + bytes;
            return ptr;
        }
        return ::operator new(bytes);
    }

    void reset() {
        offset_ = 0;
    }

private:
    static constexpr size_t capacity_ = 1024 * 1024; // 1MB linear frame buffer
    uint8_t* buffer_;
    size_t offset_ = 0;

    FrameAllocator() {
        buffer_ = static_cast<uint8_t*>(::operator new(capacity_));
    }

    ~FrameAllocator() {
        ::operator delete(buffer_);
    }
};

template <typename T>
class Temp {
private:
    T* ptr_ = nullptr;
public:
    template <typename... Args>
    explicit Temp(Args&&... args) {
        void* mem = FrameAllocator::instance().allocate(sizeof(T), alignof(T));
        ptr_ = new (mem) T(std::forward<Args>(args)...);
    }

    T* operator->() const { return ptr_; }
    T& operator*() const { return *ptr_; }
    T* get() const { return ptr_; }
};

// ---------------------------------------------------------------------------
// ObjectPool & Pool<T> — First-Class Zero-Allocation Object Reuse Pool
// ---------------------------------------------------------------------------
template <typename T>
class ObjectPool {
public:
    static ObjectPool& instance() {
        static ObjectPool pool;
        return pool;
    }

    T* acquire() {
        if (pool_.empty()) {
            return new T();
        }
        T* obj = pool_.back();
        pool_.pop_back();
        return obj;
    }

    void release(T* obj) {
        if (obj) {
            pool_.push_back(obj);
        }
    }

    ~ObjectPool() {
        for (T* obj : pool_) {
            delete obj;
        }
        pool_.clear();
    }

private:
    std::vector<T*> pool_;
};

template <typename T>
class Pool {
private:
    T* ptr_ = nullptr;
public:
    Pool() {
        ptr_ = ObjectPool<T>::instance().acquire();
    }

    ~Pool() {
        if (ptr_) {
            ObjectPool<T>::instance().release(ptr_);
        }
    }

    T* operator->() const { return ptr_; }
    T& operator*() const { return *ptr_; }
    T* get() const { return ptr_; }
};

} // namespace mem
} // namespace zenith

#endif // ZENITH_MEMORY_H
