#ifndef ZENITH_PLUGIN_PROTOCOL_H
#define ZENITH_PLUGIN_PROTOCOL_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Plugin Registration Types
typedef enum {
    ZENITH_PLUGIN_DART,   // Uses Flutter MethodChannel
    ZENITH_PLUGIN_RUST    // Uses Direct FFI
} ZenithPluginType;

// Plugin Call Result
typedef struct {
    char* json_result;
    int error_code;
    char* error_message;
} ZenithPluginResult;

// ------------------------------------------------------------------
// C++ Runtime Exports (Called by Dart to register plugins)
// ------------------------------------------------------------------

/**
 * Registers a Dart-based plugin with the Zenith Runtime.
 * @param name: The name used in Zenith code (e.g., "location")
 * @param channel_name: The Flutter MethodChannel name
 */
void zenith_register_dart_plugin(const char* name, const char* channel_name);

/**
 * Registers a Rust-based plugin with the Zenith Runtime.
 * @param name: The name used in Zenith code (e.g., "crypto")
 * @param lib_handle: Pointer to the loaded .so symbol table
 */
void zenith_register_rust_plugin(const char* name, void* lib_handle);

// ------------------------------------------------------------------
// Dart Host Exports (Called by C++ to invoke plugins)
// ------------------------------------------------------------------

/**
 * Invokes a method on a registered Dart plugin.
 * @param plugin_name: Name registered in Zenith
 * @param method: Method name to call
 * @param args_json: JSON arguments from Zenith
 * @return: ZenithPluginResult (must be freed)
 */
ZenithPluginResult zenith_invoke_dart_plugin(
    const char* plugin_name, 
    const char* method, 
    const char* args_json
);

/**
 * Invokes a function on a registered Rust plugin.
 * @param plugin_name: Name registered in Zenith
 * @param function: Function symbol name
 * @param args_json: JSON arguments
 * @return: ZenithPluginResult
 */
ZenithPluginResult zenith_invoke_rust_plugin(
    const char* plugin_name, 
    const char* function, 
    const char* args_json
);

/**
 * Frees memory allocated for plugin results.
 */
void zenith_free_plugin_result(ZenithPluginResult* result);

#ifdef __cplusplus
}
#endif

#endif // ZENITH_PLUGIN_PROTOCOL_H
