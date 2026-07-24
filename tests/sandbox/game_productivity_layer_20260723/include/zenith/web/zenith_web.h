#ifndef ZENITH_WEB_H
#define ZENITH_WEB_H

#include "../common/zenith_common.h"

namespace zenith {

inline char get_keyboard_char() {
    return '\0'; // Keyboard synchronous console loops are disabled on Web/WASM targets
}

inline std::string httpGet(const std::string& url) {
    // HTTP requests on web targets are handled inside generated JavaScript/WASM wrappers
    return "{\"web_fetch\": true}";
}

inline std::string httpPost(const std::string& url, const std::string& json_body) {
    return "{\"web_fetch\": true}";
}

} // namespace zenith

#endif // ZENITH_WEB_H
