// zenith_plugin_host.h
// Defines the interface for Zenith to call Dart and Rust plugins

#ifndef ZENITH_PLUGIN_HOST_H
#define ZENITH_PLUGIN_HOST_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// ------------------------------------------------------------------
// Plugin Registration
// ------------------------------------------------------------------

// Register a Dart plugin (e.g., "location", "camera")
// name: The name used in Zenith code (e.g., "location")
// dart_method: The method channel name in Dart
bool zenith_register_dart_plugin(const char* name, const char* dart_channel);

// Register a Rust plugin (e.g., "crypto", "image_proc")
// name: The name used in Zenith code
// rust_func_prefix: The prefix of C-exported functions in Rust
bool zenith_register_rust_plugin(const char* name, const char* rust_prefix);

// ------------------------------------------------------------------
// Execution Interface (Called from Zenith VM)
// ------------------------------------------------------------------

// Call a Dart plugin method
// plugin_name: e.g., "location"
// method: e.g., "getCurrentLocation"
// args_json: JSON string of arguments
// returns: JSON string result (caller must free)
char* zenith_call_dart_plugin(const char* plugin_name, const char* method, const char* args_json);

// Call a Rust plugin function
// plugin_name: e.g., "crypto"
// function: e.g., "hash_sha256"
// args_json: JSON string of arguments
// returns: JSON string result (caller must free)
char* zenith_call_rust_plugin(const char* plugin_name, const char* function, const char* args_json);

// ------------------------------------------------------------------
// Event Handling (Native -> Zenith)
// ------------------------------------------------------------------

// Push an event from Dart/Rust to Zenith (e.g., Location Update, Camera Frame)
// event_name: e.g., "onLocationChanged"
// data_json: JSON payload
void zenith_push_event_to_vm(const char* event_name, const char* data_json);

#ifdef __cplusplus
}
#endif

#endif // ZENITH_PLUGIN_HOST_H
