#ifndef ZENITH_NATIVE_H
#define ZENITH_NATIVE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>

// Initialize the Zenith runtime
int zenith_native_init(const char* app_data_path);

// Execute Zenith code and return result (caller must free)
char* zenith_native_execute(const char* code);

// Call a Rust function by name with arguments
char* zenith_native_call_function(const char* func_name, const char* args_json);

// Process a touch event
void zenith_native_on_touch(float x, float y, int action);

// Process a touch move event
void zenith_native_on_touch_move(float x, float y);

// Render the UI to a buffer (returns pointer to pixel data)
void* zenith_native_render(int width, int height, int* out_stride);

// Free memory allocated by native functions
void zenith_native_free(void* ptr);

// Get the current UI state as JSON
char* zenith_native_get_state();

// Set the UI state from JSON
int zenith_native_set_state(const char* state_json);

// Cleanup and shutdown
void zenith_native_shutdown();

#ifdef __cplusplus
}
#endif

#endif // ZENITH_NATIVE_H
