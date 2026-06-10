# Zenith Core Runtime
// Cross-platform runtime engine for executing Zenith bytecode
// Supports Android (JNI), iOS (Swift), Web (Wasm), and Desktop

#ifndef ZENITH_RUNTIME_H
#define ZENITH_RUNTIME_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// MARK: - Core Types

typedef enum {
    ZENITH_OK = 0,
    ZENITH_ERROR_INVALID_BYTECODE = 1,
    ZENITH_ERROR_PLUGIN_NOT_FOUND = 2,
    ZENITH_ERROR_EXECUTION_FAILED = 3,
    ZENITH_ERROR_MEMORY_ALLOC = 4,
    ZENITH_ERROR_IO = 5
} ZenithResult;

typedef enum {
    ZENITH_NODE_CONTAINER,
    ZENITH_NODE_TEXT,
    ZENITH_NODE_BUTTON,
    ZENITH_NODE_CARD,
    ZENITH_NODE_INPUT,
    ZENITH_NODE_IMAGE,
    ZENITH_NODE_LIST
} ZenithNodeType;

typedef struct ZenithNode {
    ZenithNodeType type;
    char* value;
    char* label;
    char* placeholder;
    char* action;
    struct ZenithNode** children;
    size_t children_count;
} ZenithNode;

typedef struct ZenithUI {
    ZenithNode** nodes;
    size_t node_count;
} ZenithUI;

typedef struct {
    char* key;
    void* value;
    size_t value_size;
} ZenithStateEntry;

typedef struct {
    ZenithStateEntry* entries;
    size_t entry_count;
} ZenithState;

// MARK: - Runtime Context

typedef struct ZenithRuntime ZenithRuntime;

// Create new runtime instance
ZenithRuntime* zenith_runtime_create(void);

// Destroy runtime instance
void zenith_runtime_destroy(ZenithRuntime* runtime);

// Load Zenith bytecode
ZenithResult zenith_runtime_load(ZenithRuntime* runtime, const uint8_t* bytecode, size_t size);

// Execute Zenith function
ZenithResult zenith_runtime_execute(ZenithRuntime* runtime, const char* func_name, 
                                    const char* args_json, char** result_json);

// Render UI to native components
ZenithResult zenith_runtime_render(ZenithRuntime* runtime, ZenithUI** ui);

// Free UI tree
void zenith_ui_free(ZenithUI* ui);

// MARK: - Plugin System

typedef ZenithResult (*ZenithPluginFunc)(const char* args_json, char** result_json, void* user_data);

typedef struct {
    char* name;
    ZenithPluginFunc func;
    void* user_data;
} ZenithPlugin;

// Register plugin implementation
ZenithResult zenith_runtime_register_plugin(ZenithRuntime* runtime, const char* name, 
                                            ZenithPluginFunc func, void* user_data);

// Call plugin by name
ZenithResult zenith_runtime_call_plugin(ZenithRuntime* runtime, const char* plugin_name,
                                        const char* args_json, char** result_json);

// MARK: - State Management

// Set state value
ZenithResult zenith_runtime_set_state(ZenithRuntime* runtime, const char* key, 
                                      const void* value, size_t size);

// Get state value
ZenithResult zenith_runtime_get_state(ZenithRuntime* runtime, const char* key, 
                                      void** value, size_t* size);

// Clear all state
void zenith_runtime_clear_state(ZenithRuntime* runtime);

// MARK: - Hot Module Replacement

typedef void (*ZenithHMRCallback)(const char* module_path, void* user_data);

// Enable HMR with callback for live updates
ZenithResult zenith_runtime_enable_hmr(ZenithRuntime* runtime, ZenithHMRCallback callback, void* user_data);

// Push module update via HMR
ZenithResult zenith_runtime_hmr_update(ZenithRuntime* runtime, const char* module_path,
                                       const uint8_t* new_bytecode, size_t size);

// MARK: - Compiler Interface

// Compile .zen source to bytecode
ZenithResult zenith_compile(const char* source_path, uint8_t** bytecode, size_t* bytecode_size);

// Compile .zen source to AST (for transpiler)
ZenithResult zenith_compile_to_ast(const char* source_path, ZenithUI** ast);

// MARK: - Utility Functions

// Get error message for result code
const char* zenith_result_str(ZenithResult result);

// Get runtime version
const char* zenith_version(void);

// Initialize logging
void zenith_init_logging(int level);

#ifdef __cplusplus
}
#endif

#endif // ZENITH_RUNTIME_H

// ============================================================================
// IMPLEMENTATION NOTES:
// 
// Android: Compile to shared library, access via JNI
//   gcc -shared -fPIC -o libzenith.so zenith_runtime.c
//   Java: System.loadLibrary("zenith")
//
// iOS: Compile to static library, link with Swift
//   gcc -c -o zenith_runtime.o zenith_runtime.c
//   Swift: import ZenithRuntime (via bridging header)
//
// Web: Compile to WebAssembly
//   emcc zenith_runtime.c -o zenith.wasm -s EXPORTED_FUNCTIONS='["_zenith_runtime_create", ...]'
//   JS: WebAssembly.instantiateStreaming(fetch('zenith.wasm'))
//
// Desktop: Link as static/shared library
//   gcc -c zenith_runtime.c
//   Link with your C/C++/Rust application
// ============================================================================
