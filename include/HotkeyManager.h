#pragma once

#include <string>

enum class HotkeyAction
{
    None,
    Start,
    Pause,
    StopOrCancel
};

class HotkeyManager
{
public:
    HotkeyManager() = default;
    ~HotkeyManager();

    HotkeyManager(const HotkeyManager&) = delete;
    HotkeyManager& operator=(const HotkeyManager&) = delete;

    bool registerPlaybackHotkeys(
        int startKey,
        int pauseKey,
        int cancelKey,
        std::string& errorMessage);

    bool registerRecordingHotkeys(
        int startKey,
        int pauseKey,
        int stopKey,
        std::string& errorMessage);

    HotkeyAction poll();

    void unregisterAll();

private:
    bool registerHotkey(
        int identifier,
        int virtualKey,
        const std::string& description,
        std::string& errorMessage);

    bool startRegistered_ = false;
    bool pauseRegistered_ = false;
    bool stopRegistered_ = false;
};