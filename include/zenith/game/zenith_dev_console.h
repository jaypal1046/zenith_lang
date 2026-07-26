#ifndef ZENITH_DEV_CONSOLE_H
#define ZENITH_DEV_CONSOLE_H

#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <iostream>

namespace zenith {

using CommandCallback = std::function<void(const std::vector<std::string>& args)>;

class DevConsole {
private:
    static inline std::unordered_map<std::string, CommandCallback> s_commands;
    static inline std::vector<std::string> s_logHistory;
    static inline bool s_isOpen = false;

public:
    static void registerCommand(const std::string& commandName, CommandCallback callback) {
        s_commands[commandName] = callback;
    }

    static bool executeCommand(const std::string& fullCommand) {
        s_logHistory.push_back("> " + fullCommand);

        std::vector<std::string> tokens;
        std::string token;
        for (char c : fullCommand) {
            if (c == ' ') {
                if (!token.empty()) { tokens.push_back(token); token.clear(); }
            } else {
                token += c;
            }
        }
        if (!token.empty()) tokens.push_back(token);

        if (tokens.empty()) return false;

        std::string cmd = tokens[0];
        auto it = s_commands.find(cmd);
        if (it != s_commands.end()) {
            std::vector<std::string> args(tokens.begin() + 1, tokens.end());
            it->second(args);
            return true;
        }

        s_logHistory.push_back("Unknown command: " + cmd);
        return false;
    }

    static void toggle() { s_isOpen = !s_isOpen; }
    static bool isOpen() { return s_isOpen; }
    static std::size_t getLogCount() { return s_logHistory.size(); }
};

} // namespace zenith

#endif // ZENITH_DEV_CONSOLE_H
