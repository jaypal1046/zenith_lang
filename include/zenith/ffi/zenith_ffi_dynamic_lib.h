#ifndef ZENITH_FFI_DYNAMIC_LIB_H
#define ZENITH_FFI_DYNAMIC_LIB_H

#include <string>
#include <unordered_map>
#include <mutex>
#include <stdexcept>
#include <memory>

#ifdef _WIN32
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
    #include <libloaderapi.h>
#else
    #include <dlfcn.h>
#endif

namespace zenith {
namespace ffi {

/**
 * DynamicLibrary - Cross-platform dynamic library loader
 * 
 * Provides runtime loading of FFI libraries (bridge.dll, bridge.so, etc.)
 * using LoadLibrary on Windows and dlopen on POSIX systems.
 */
class DynamicLibrary {
public:
    /**
     * Load a dynamic library at the specified path
     * @param path Path to the library file (.dll, .so, .dylib)
     * @throws std::runtime_error if loading fails
     */
    explicit DynamicLibrary(const std::string& path) : handle_(nullptr), loaded_(false) {
        load(path);
    }

    ~DynamicLibrary() {
        unload();
    }

    // Non-copyable but movable
    DynamicLibrary(const DynamicLibrary&) = delete;
    DynamicLibrary& operator=(const DynamicLibrary&) = delete;
    
    DynamicLibrary(DynamicLibrary&& other) noexcept 
        : handle_(other.handle_), loaded_(other.loaded_) {
        other.handle_ = nullptr;
        other.loaded_ = false;
    }

    DynamicLibrary& operator=(DynamicLibrary&& other) noexcept {
        if (this != &other) {
            unload();
            handle_ = other.handle_;
            loaded_ = other.loaded_;
            other.handle_ = nullptr;
            other.loaded_ = false;
        }
        return *this;
    }

    /**
     * Load a library at the specified path
     * @param path Path to the library file
     * @throws std::runtime_error if loading fails
     */
    void load(const std::string& path) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        if (loaded_) {
            unload();
        }

#ifdef _WIN32
        // Windows: Use LoadLibraryW for Unicode support
        std::wstring wide_path(path.begin(), path.end());
        handle_ = LoadLibraryW(wide_path.c_str());
        if (!handle_) {
            DWORD error = GetLastError();
            throw std::runtime_error("Failed to load library '" + path + 
                                   "' (Windows error code: " + std::to_string(error) + ")");
        }
#else
        // POSIX: Use dlopen with RTLD_NOW for immediate symbol resolution
        // RTLD_LOCAL keeps symbols local to this library
        handle_ = dlopen(path.c_str(), RTLD_NOW | RTLD_LOCAL);
        if (!handle_) {
            const char* error_msg = dlerror();
            throw std::runtime_error("Failed to load library '" + path + 
                                   "': " + std::string(error_msg ? error_msg : "unknown error"));
        }
#endif
        loaded_ = true;
        path_ = path;
    }

    /**
     * Unload the currently loaded library
     */
    void unload() {
        std::lock_guard<std::mutex> lock(mutex_);
        
        if (handle_ && loaded_) {
#ifdef _WIN32
            FreeLibrary(static_cast<HMODULE>(handle_));
#else
            dlclose(handle_);
#endif
            handle_ = nullptr;
            loaded_ = false;
        }
    }

    /**
     * Get a function pointer from the loaded library
     * @param name Name of the function to lookup
     * @return Function pointer, or nullptr if not found
     */
    template<typename T>
    T getFunction(const std::string& name) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        if (!handle_ || !loaded_) {
            throw std::runtime_error("Library not loaded: " + path_);
        }

#ifdef _WIN32
        FARPROC ptr = GetProcAddress(static_cast<HMODULE>(handle_), name.c_str());
        if (!ptr) {
            return nullptr;
        }
        return reinterpret_cast<T>(ptr);
#else
        void* ptr = dlsym(handle_, name.c_str());
        if (!ptr) {
            return nullptr;
        }
        return reinterpret_cast<T>(ptr);
#endif
    }

    /**
     * Check if a function exists in the loaded library
     * @param name Name of the function to check
     * @return true if function exists, false otherwise
     */
    bool hasFunction(const std::string& name) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        if (!handle_ || !loaded_) {
            return false;
        }

#ifdef _WIN32
        return GetProcAddress(static_cast<HMODULE>(handle_), name.c_str()) != nullptr;
#else
        // Clear any existing error before calling dlsym
        dlerror();
        void* ptr = dlsym(handle_, name.c_str());
        return ptr != nullptr;
#endif
    }

    /**
     * Check if a library is currently loaded
     * @return true if loaded, false otherwise
     */
    bool isLoaded() const {
        return loaded_;
    }

    /**
     * Get the path of the loaded library
     * @return Library path
     */
    const std::string& getPath() const {
        return path_;
    }

private:
    void* handle_;
    bool loaded_;
    std::string path_;
    mutable std::mutex mutex_;
};

/**
 * LibraryManager - Singleton manager for dynamic libraries
 * 
 * Provides centralized management of loaded FFI libraries with
 * automatic cleanup and reference counting.
 */
class LibraryManager {
public:
    static LibraryManager& getInstance() {
        static LibraryManager instance;
        return instance;
    }

    /**
     * Load or retrieve a cached library
     * @param path Path to the library
     * @return Reference to the DynamicLibrary
     */
    DynamicLibrary& loadLibrary(const std::string& path) {
        std::lock_guard<std::mutex> lock(cache_mutex_);
        
        auto it = libraries_.find(path);
        if (it != libraries_.end()) {
            return *(it->second);
        }

        libraries_[path] = std::make_unique<DynamicLibrary>(path);
        return *(libraries_[path]);
    }

    /**
     * Unload a library by path
     * @param path Path to the library to unload
     */
    void unloadLibrary(const std::string& path) {
        std::lock_guard<std::mutex> lock(cache_mutex_);
        libraries_.erase(path);
    }

    /**
     * Unload all libraries
     */
    void unloadAll() {
        std::lock_guard<std::mutex> lock(cache_mutex_);
        libraries_.clear();
    }

    /**
     * Check if a library is loaded
     * @param path Path to check
     * @return true if loaded, false otherwise
     */
    bool isLoaded(const std::string& path) {
        std::lock_guard<std::mutex> lock(cache_mutex_);
        auto it = libraries_.find(path);
        return it != libraries_.end() && it->second->isLoaded();
    }

private:
    LibraryManager() = default;
    ~LibraryManager() {
        unloadAll();
    }

    std::unordered_map<std::string, std::unique_ptr<DynamicLibrary>> libraries_;
    std::mutex cache_mutex_;
};

} // namespace ffi
} // namespace zenith

#endif // ZENITH_FFI_DYNAMIC_LIB_H
