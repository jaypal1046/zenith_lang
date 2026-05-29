#ifndef ZENITH_STD_CONCURRENCY_HPP
#define ZENITH_STD_CONCURRENCY_HPP

#include <functional>
#include <future>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <memory>
#include <vector>
#include <string>
#include <optional>
#include <utility>
#include <stdexcept>
#include <type_traits>
#include <chrono>

namespace zenith { namespace stdlib {

// Result - error handling
template<typename T> class Result {
public:
    Result(const T& v) : data_(v), has_error_(false) {}
    Result(T&& v) : data_(std::move(v)), has_error_(false) {}
    static Result error(const std::string& msg) { Result r; r.error_msg_ = msg; r.has_error_ = true; return r; }
    bool is_ok() const { return !has_error_; }
    bool is_error() const { return has_error_; }
    T& value() { if(has_error_) throw std::runtime_error(error_msg_); return data_; }
    const T& value() const { if(has_error_) throw std::runtime_error(error_msg_); return data_; }
    T value_or(const T& def) const { return has_error_ ? def : data_; }
    const std::string& error() const { return error_msg_; }
private:
    Result() = default;
    T data_{};
    std::string error_msg_;
    bool has_error_ = false;
};

template<> class Result<void> {
public:
    static Result ok() { return Result(); }
    static Result error(const std::string& msg) { Result r; r.error_msg_ = msg; r.has_error_ = true; return r; }
    bool is_ok() const { return !has_error_; }
    bool is_error() const { return has_error_; }
    const std::string& error() const { return error_msg_; }
private:
    Result() = default;
    std::string error_msg_;
    bool has_error_ = false;
};

// Option - nullable values
template<typename T> class Option {
public:
    Option() : has_value_(false) {}
    Option(const T& v) : data_(v), has_value_(true) {}
    Option(T&& v) : data_(std::move(v)), has_value_(true) {}
    static Option none() { return Option(); }
    static Option some(const T& v) { return Option(v); }
    static Option some(T&& v) { return Option(std::move(v)); }
    bool is_some() const { return has_value_; }
    bool is_none() const { return !has_value_; }
    T& value() { if(!has_value_) throw std::runtime_error("Option is None"); return data_; }
    const T& value() const { if(!has_value_) throw std::runtime_error("Option is None"); return data_; }
    T value_or(const T& def) const { return has_value_ ? data_ : def; }
private:
    T data_{};
    bool has_value_;
};

// Promise/Future - async coordination
struct PromiseStateBase {
    std::exception_ptr exception;
    bool ready = false;
    std::mutex mutex;
    std::condition_variable cv;
};

template<typename T> struct TypedPromiseState : PromiseStateBase { T data{}; };

// Forward declarations
template<typename T> class Promise;
template<> class Promise<void>;

// 1. Generic Future
template<typename T> class Future {
    std::shared_ptr<TypedPromiseState<T>> state_;
public:
    Future() = default;
    bool is_ready() const { std::lock_guard<std::mutex> lock(state_->mutex); return state_->ready; }
    void wait() const { std::unique_lock<std::mutex> lock(state_->mutex); state_->cv.wait(lock, [this]{return state_->ready;}); }
    T get() { wait(); if(state_->exception) std::rethrow_exception(state_->exception); return std::move(state_->data); }
    friend class Promise<T>;
    explicit Future(std::shared_ptr<TypedPromiseState<T>> s) : state_(s) {}
};

// 2. Specialized Future for void
template<> class Future<void> {
    std::shared_ptr<PromiseStateBase> state_;
public:
    Future() = default;
    bool is_ready() const { std::lock_guard<std::mutex> lock(state_->mutex); return state_->ready; }
    void wait() const { std::unique_lock<std::mutex> lock(state_->mutex); state_->cv.wait(lock, [this]{return state_->ready;}); }
    void get() { wait(); if(state_->exception) std::rethrow_exception(state_->exception); }
    friend class Promise<void>;
    explicit Future(std::shared_ptr<PromiseStateBase> s) : state_(s) {}
};

// 3. Generic Promise
template<typename T> class Promise {
    std::shared_ptr<TypedPromiseState<T>> state_;
public:
    Promise() : state_(std::make_shared<TypedPromiseState<T>>()) {}
    void set_value(const T& v) {
        std::unique_lock<std::mutex> lock(state_->mutex);
        if(state_->ready) throw std::runtime_error("Promise fulfilled");
        state_->data = v; state_->ready = true; state_->cv.notify_all();
    }
    void set_value(T&& v) {
        std::unique_lock<std::mutex> lock(state_->mutex);
        if(state_->ready) throw std::runtime_error("Promise fulfilled");
        state_->data = std::move(v); state_->ready = true; state_->cv.notify_all();
    }
    void set_exception(std::exception_ptr ex) {
        std::unique_lock<std::mutex> lock(state_->mutex);
        if(state_->ready) throw std::runtime_error("Promise fulfilled");
        state_->exception = ex; state_->ready = true; state_->cv.notify_all();
    }
    Future<T> get_future() { return Future<T>(state_); }
    friend class Future<T>;
};

// 4. Specialized Promise for void
template<> class Promise<void> {
    std::shared_ptr<PromiseStateBase> state_;
public:
    Promise() : state_(std::make_shared<PromiseStateBase>()) {}
    void set_value() {
        std::unique_lock<std::mutex> lock(state_->mutex);
        if(state_->ready) throw std::runtime_error("Promise fulfilled");
        state_->ready = true; state_->cv.notify_all();
    }
    void set_exception(std::exception_ptr ex) {
        std::unique_lock<std::mutex> lock(state_->mutex);
        if(state_->ready) throw std::runtime_error("Promise fulfilled");
        state_->exception = ex; state_->ready = true; state_->cv.notify_all();
    }
    Future<void> get_future() { return Future<void>(state_); }
    friend class Future<void>;
};

// Channel - message passing
template<typename T> class Channel {
    std::queue<T> queue_;
    size_t capacity_;
    bool closed_ = false;
    mutable std::mutex mutex_;
    std::condition_variable not_empty_, not_full_;
public:
    explicit Channel(size_t cap = 0) : capacity_(cap) {}
    void send(const T& v) {
        std::unique_lock<std::mutex> lock(mutex_);
        not_full_.wait(lock, [this]{return queue_.size() < capacity_ || capacity_ == 0 || closed_;});
        if(closed_) throw std::runtime_error("Channel closed");
        queue_.push(v); not_empty_.notify_one();
    }
    T receive() {
        std::unique_lock<std::mutex> lock(mutex_);
        not_empty_.wait(lock, [this]{return !queue_.empty() || closed_;});
        if(queue_.empty() && closed_) throw std::runtime_error("Channel closed");
        T v = std::move(queue_.front()); queue_.pop(); not_full_.notify_one(); return v;
    }
    bool try_send(const T& v) {
        std::unique_lock<std::mutex> lock(mutex_);
        if(closed_ || (capacity_ > 0 && queue_.size() >= capacity_)) return false;
        queue_.push(v); not_empty_.notify_one(); return true;
    }
    std::optional<T> try_receive() {
        std::unique_lock<std::mutex> lock(mutex_);
        if(queue_.empty()) return std::nullopt;
        T v = std::move(queue_.front()); queue_.pop(); not_full_.notify_one(); return v;
    }
    void close() { std::unique_lock<std::mutex> lock(mutex_); closed_ = true; not_empty_.notify_all(); not_full_.notify_all(); }
    bool is_closed() const { std::lock_guard<std::mutex> lock(mutex_); return closed_; }
    size_t size() const { std::lock_guard<std::mutex> lock(mutex_); return queue_.size(); }
};

// AsyncTaskExecutor - thread pool
class AsyncTaskExecutor {
    std::vector<std::thread> workers_;
    std::queue<std::function<void()>> tasks_;
    mutable std::mutex queue_mutex_;
    std::condition_variable condition_;
    bool stop_ = false;
public:
    explicit AsyncTaskExecutor(size_t n = std::thread::hardware_concurrency()) {
        for(size_t i = 0; i < n; ++i) workers_.emplace_back([this]{
            while(true) {
                std::function<void()> task;
                { std::unique_lock<std::mutex> lock(queue_mutex_);
                  condition_.wait(lock, [this]{return stop_ || !tasks_.empty();});
                  if(stop_ && tasks_.empty()) return;
                  task = std::move(tasks_.front()); tasks_.pop(); }
                task();
            }
        });
    }
    ~AsyncTaskExecutor() { shutdown(); }
    template<typename F, typename... Args> auto submit(F&& f, Args&&... args) {
        using R = std::invoke_result_t<F, Args...>;
        Promise<R> promise;
        auto bound_task = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
        auto task = [promise, bound_task = std::move(bound_task)]() mutable {
            try {
                if constexpr (std::is_void_v<R>) {
                    bound_task();
                    promise.set_value();
                } else {
                    promise.set_value(bound_task());
                }
            } catch (...) {
                promise.set_exception(std::current_exception());
            }
        };
        { std::lock_guard<std::mutex> lock(queue_mutex_); tasks_.push(std::move(task)); }
        condition_.notify_one();
        return promise.get_future();
    }
    void shutdown() { {std::lock_guard<std::mutex> lock(queue_mutex_); stop_ = true;} condition_.notify_all(); for(auto& w : workers_) if(w.joinable()) w.join(); }
    size_t pending_tasks() const { std::lock_guard<std::mutex> lock(queue_mutex_); return tasks_.size(); }
};

// Actor - message-based concurrency
template<typename T> class Actor {
    Channel<T> mailbox_{100};
    std::function<void(const T&, Actor<T>&)> handler_;
    std::thread thread_;
    std::atomic<bool> running_{true};
public:
    Actor(std::function<void(const T&, Actor<T>&)> h) : handler_(std::move(h)) {
        thread_ = std::thread([this]{
            while(running_) {
                try { T msg = mailbox_.receive(); handler_(msg, *this); }
                catch(const std::runtime_error& e) { if(std::string(e.what()).find("Channel closed")!=std::string::npos) break; }
            }
        });
    }
    ~Actor() { stop(); }
    void send(const T& msg) { mailbox_.send(msg); }
    void stop() { running_ = false; mailbox_.close(); if(thread_.joinable()) thread_.join(); }
    bool is_running() const { return running_; }
};

// Convenience functions
template<typename T> Result<T> make_ok(const T& v) { return Result<T>(v); }
template<typename T> Result<T> make_error(const std::string& msg) { return Result<T>::error(msg); }
template<typename T> Option<T> make_some(const T& v) { return Option<T>::some(v); }
template<typename T> Option<T> make_none() { return Option<T>::none(); }
inline std::shared_ptr<AsyncTaskExecutor> create_executor(size_t n = 0) {
    if(n == 0) n = std::thread::hardware_concurrency();
    return std::make_shared<AsyncTaskExecutor>(n);
}

}} // namespace zenith::stdlib
#endif
