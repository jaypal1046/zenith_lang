#include <string>
#include <vector>
#include <unordered_map>
#include <regex>
#include <future>
#include <iostream>
#include <functional>
#include "zenith_runtime.h"
#include "zenith/std/concurrency.hpp"


#ifdef __ANDROID__
const bool isAndroid = true;
#else
const bool isAndroid = false;
#endif

#ifdef __APPLE__
  #include <TargetConditionals.h>
  #if TARGET_OS_IPHONE
    const bool isIos = true;
    const bool isMac = false;
  #else
    const bool isIos = false;
    const bool isMac = true;
  #endif
#else
  const bool isIos = false;
  const bool isMac = false;
#endif

#ifdef __linux__
  #ifndef __ANDROID__
    const bool isLinux = true;
  #else
    const bool isLinux = false;
  #endif
#else
  const bool isLinux = false;
#endif

#ifdef _WIN32
const bool isWindows = true;
#else
const bool isWindows = false;
#endif

const bool isWeb = false;

inline void print(std::string msg) { std::cout << msg; }
inline void println(std::string msg) { std::cout << msg << std::endl; }
inline std::string httpGet(std::string url) { return zenith::httpGet(url); }
inline std::string httpPost(std::string url, std::string json_body) { return zenith::httpPost(url, json_body); }
inline std::string gcStats() { return zenith::mem::gcStatsString(); }

extern "C" std::string encrypt(std::string plaintext, std::string key);

extern "C" std::string decrypt(std::string ciphertext, std::string key);

std::string deriveKey(std::string password, std::string salt) {
    std::cerr << "[Warning] JS interop function 'deriveKey' is not supported on desktop platform." << std::endl;
    return "";
}

class VaultService {
private:
public:
    std::string masterKey = "";

    VaultService()  {}

    void unlock(std::string password) {
        std::string salt = "zenith_securevault_salt";
        masterKey = deriveKey(password, salt);
    }

    std::string save(std::string noteText) {
        return encrypt(noteText, masterKey);
    }

    std::string open(std::string cipherText) {
        return decrypt(cipherText, masterKey);
    }

    std::string summarize(std::string noteText) {
        // --- AUTO-GENERATED AGENTIC BINDING ---
        std::string prompt = R"(Summarize this note in 2 sentences: {noteText})";
        prompt = std::regex_replace(prompt, std::regex("\\{noteText\\}"), noteText);
        zenith::LLMClient client("http://localhost:11434/api/generate");
        std::string response = client.promptStream(prompt, [](const std::string& chunk) { std::cout << chunk << std::flush; });
        return response;
    }

    void triggerCallback(std::string name, std::string val = "") {
        if (name == "unlock") { this->unlock(val); return; }
        if (name == "save") { this->save(val); return; }
        if (name == "open") { this->open(val); return; }
        if (name == "summarize") { this->summarize(val); return; }
    }

};

class NotesApp {
private:
public:
    std::string currentNote = "";
    std::string passwordInput = "";
    std::string noteInput = "";
    std::string status = "Locked";
    std::string decryptedNote = "";
    std::string aiSummary = "";
    std::string encryptedSavedNote = "";

    NotesApp()  {}

    zenith::UIElement build() {
        return zenith::UI::Column(zenith::make_children(zenith::UI::Text("SecureVault - Encrypted Notes", {{"fontWeight", zenith::toString("bold")}, {"color", zenith::toString("cyan")}}), zenith::UI::Text(zenith::concat("Status: ", status), {{"color", zenith::toString("green")}}), zenith::UI::Card(zenith::make_children(zenith::UI::Text("1. Authenticate / Unlock:", {}), zenith::UI::Row(zenith::make_children(zenith::UI::Text("Password:", {}), zenith::UI::TextField("", {{"value", zenith::toString(passwordInput)}, {"placeholder", zenith::toString("Enter Password")}, {"onChange", zenith::toString("handlePasswordChange")}}), zenith::UI::Button("Unlock", {{"onClick", zenith::toString("handleUnlock")}})), {})), {{"padding", zenith::toString(1)}}), zenith::UI::Card(zenith::make_children(zenith::UI::Text("2. Secure Notes Editor:", {}), zenith::UI::Row(zenith::make_children(zenith::UI::Text("Note text:", {}), zenith::UI::TextField("", {{"value", zenith::toString(noteInput)}, {"placeholder", zenith::toString("Secret content...")}, {"onChange", zenith::toString("handleNoteChange")}})), {}), zenith::UI::Row(zenith::make_children(zenith::UI::Button("Save Encrypted", {{"onClick", zenith::toString("handleSave")}}), zenith::UI::Button("Decrypt Note", {{"onClick", zenith::toString("handleDecrypt")}}), zenith::UI::Button("Summarize (AI)", {{"onClick", zenith::toString("handleSummarize")}})), {})), {{"padding", zenith::toString(1)}}), zenith::UI::Card(zenith::make_children(zenith::UI::Text("Encrypted Payload:", {}), zenith::UI::Text(encryptedSavedNote, {{"color", zenith::toString("yellow")}}), zenith::UI::Text("Decrypted Note:", {}), zenith::UI::Text(decryptedNote, {{"color", zenith::toString("green")}}), zenith::UI::Text("AI Summary:", {}), zenith::UI::Text(aiSummary, {{"color", zenith::toString("magenta")}})), {{"padding", zenith::toString(1)}})), {});
    }

    void handlePasswordChange(std::string val) {
        passwordInput = val;
    }

    void handleNoteChange(std::string val) {
        noteInput = val;
    }

    void handleUnlock() {
        VaultService vault = VaultService();
        vault.unlock(passwordInput);
        {
            status = "Unlocked";
            std::cout << "\n[Runtime] setState: Re-rendering UI Layout...\n";
            this->build().render();
        }
    }

    zenith::stdlib::Future<void> handleSave() {
        auto _promise = std::make_shared<zenith::stdlib::Promise<void>>();
        std::thread([_promise, this]() mutable {
            try {
                VaultService vault = VaultService();
                vault.unlock(passwordInput);
                std::string encrypted = vault.save(noteInput);
                {
                    encryptedSavedNote = encrypted;
                    decryptedNote = "";
                    aiSummary = "";
                    std::cout << "\n[Runtime] setState: Re-rendering UI Layout...\n";
                    this->build().render();
                }
                println(zenith::concat("Saved Encrypted: ", encrypted));
                _promise->set_value();
            } catch (...) {
                _promise->set_exception(std::current_exception());
            }
        }).detach();
        return _promise->get_future();
    }

    zenith::stdlib::Future<void> handleDecrypt() {
        auto _promise = std::make_shared<zenith::stdlib::Promise<void>>();
        std::thread([_promise, this]() mutable {
            try {
                VaultService vault = VaultService();
                vault.unlock(passwordInput);
                std::string decrypted = vault.open(encryptedSavedNote);
                {
                    decryptedNote = decrypted;
                    std::cout << "\n[Runtime] setState: Re-rendering UI Layout...\n";
                    this->build().render();
                }
                _promise->set_value();
            } catch (...) {
                _promise->set_exception(std::current_exception());
            }
        }).detach();
        return _promise->get_future();
    }

    zenith::stdlib::Future<void> handleSummarize() {
        auto _promise = std::make_shared<zenith::stdlib::Promise<void>>();
        std::thread([_promise, this]() mutable {
            try {
                VaultService vault = VaultService();
                std::string summary = zenith::await_val(vault.summarize(noteInput));
                {
                    aiSummary = summary;
                    std::cout << "\n[Runtime] setState: Re-rendering UI Layout...\n";
                    this->build().render();
                }
                _promise->set_value();
            } catch (...) {
                _promise->set_exception(std::current_exception());
            }
        }).detach();
        return _promise->get_future();
    }

    void triggerCallback(std::string name, std::string val = "") {
        if (name == "handlePasswordChange") { this->handlePasswordChange(val); return; }
        if (name == "handleNoteChange") { this->handleNoteChange(val); return; }
        if (name == "handleUnlock") { this->handleUnlock(); return; }
        if (name == "handleSave") { this->handleSave(); return; }
        if (name == "handleDecrypt") { this->handleDecrypt(); return; }
        if (name == "handleSummarize") { this->handleSummarize(); return; }
    }

};

int main() {
    // --- Zenith RC+GC Memory Manager: Start background cycle collector ---
    zenith::mem::GcHeap::instance().start_background_gc(5000);

    NotesApp app = NotesApp();
    zenith::runInteractiveLoop(app);

// --- Zenith RC+GC Memory Manager: Shutdown ---
zenith::mem::GcHeap::instance().stop_background_gc();
zenith::mem::GcHeap::instance().collect(); // Final cycle sweep
#ifdef ZENITH_GC_STATS
std::cout << zenith::mem::gcStatsString() << std::endl;
#endif
}

