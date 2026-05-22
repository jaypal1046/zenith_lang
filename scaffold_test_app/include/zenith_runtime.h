#ifndef ZENITH_RUNTIME_H
#define ZENITH_RUNTIME_H

// Include common Definitions
#include "zenith/common/zenith_common.h"

// Platform-Specific Dispatcher
#if defined(__ANDROID__)
    #include "zenith/android/zenith_android.h"
#elif defined(__APPLE__)
    #include <TargetConditionals.h>
    #if TARGET_OS_IPHONE
        #include "zenith/ios/zenith_ios.h"
    #else
        #include "zenith/desktop/mac/zenith_mac.h"
    #endif
#elif defined(_WIN32)
    #include "zenith/desktop/windows/zenith_windows.h"
#elif defined(__linux__)
    #include "zenith/desktop/linux/zenith_linux.h"
#elif defined(__EMSCRIPTEN__) || defined(__wasm__)
    #include "zenith/web/zenith_web.h"
#else
    // Generic POSIX Fallback
    #include "zenith/desktop/posix/zenith_posix.h"
#endif

#endif // ZENITH_RUNTIME_H
