// Zenith Native Android Entry Driver (NDK)
// This file bridges the NativeActivity lifecycle to the Zenith app.
#include <android/native_activity.h>
#include "../include/zenith_runtime.h"

// Called by NativeActivity when the app starts.
// In a pure NDK binary (not via NativeActivity) this is unused;
// instead main() in main.cpp is the entry point.
extern "C" void ANativeActivity_onCreate(
    ANativeActivity* /*activity*/,
    void*            /*savedState*/,
    size_t           /*savedStateSize*/
) {
    // Lifecycle hook placeholder (native-app-glue extends this).
}
