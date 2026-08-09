#include "HotkeyManager.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace
{
constexpr int StartHotkeyId = 1;
constexpr int PauseHotkeyId = 2;
constexpr int StopHotkeyId = 3;
}

HotkeyManager::~HotkeyManager()
{
    unregisterAll();
}

bool HotkeyManager::registerHotkey(
    int identifier,
    int virtualKey,
    const std::string& description,
    std::string& errorMessage)
{
    if (!RegisterHotKey(
            nullptr,
            identifier,
            MOD_NOREPEAT,
            static_cast<UINT>(virtualKey)))
    {
        errorMessage =
            "Unable to register the "
            + description
            + " shortcut. The shortcut may already be in use.";

        return false;
    }

    errorMessage.clear();
    return true;
}

bool HotkeyManager::registerPlaybackHotkeys(
    int startKey,
    int pauseKey,
    int cancelKey,
    std::string& errorMessage)
{
    unregisterAll();

    if (!registerHotkey(
            StartHotkeyId,
            startKey,
            "play-start",
            errorMessage))
    {
        return false;
    }

    startRegistered_ = true;

    if (!registerHotkey(
            PauseHotkeyId,
            pauseKey,
            "play-pause",
            errorMessage))
    {
        unregisterAll();
        return false;
    }

    pauseRegistered_ = true;

    if (!registerHotkey(
            StopHotkeyId,
            cancelKey,
            "play-cancel",
            errorMessage))
    {
        unregisterAll();
        return false;
    }

    stopRegistered_ = true;

    return true;
}

bool HotkeyManager::registerRecordingHotkeys(
    int startKey,
    int pauseKey,
    int stopKey,
    std::string& errorMessage)
{
    unregisterAll();

    if (!registerHotkey(
            StartHotkeyId,
            startKey,
            "record-start",
            errorMessage))
    {
        return false;
    }

    startRegistered_ = true;

    if (!registerHotkey(
            PauseHotkeyId,
            pauseKey,
            "record-pause",
            errorMessage))
    {
        unregisterAll();
        return false;
    }

    pauseRegistered_ = true;

    if (!registerHotkey(
            StopHotkeyId,
            stopKey,
            "record-stop",
            errorMessage))
    {
        unregisterAll();
        return false;
    }

    stopRegistered_ = true;

    return true;
}

HotkeyAction HotkeyManager::poll()
{
    MSG message{};

    while (PeekMessage(
        &message,
        nullptr,
        WM_HOTKEY,
        WM_HOTKEY,
        PM_REMOVE))
    {
        if (message.wParam == StartHotkeyId)
        {
            return HotkeyAction::Start;
        }

        if (message.wParam == PauseHotkeyId)
        {
            return HotkeyAction::Pause;
        }

        if (message.wParam == StopHotkeyId)
        {
            return HotkeyAction::StopOrCancel;
        }
    }

    return HotkeyAction::None;
}

void HotkeyManager::unregisterAll()
{
    if (startRegistered_)
    {
        UnregisterHotKey(
            nullptr,
            StartHotkeyId);

        startRegistered_ = false;
    }

    if (pauseRegistered_)
    {
        UnregisterHotKey(
            nullptr,
            PauseHotkeyId);

        pauseRegistered_ = false;
    }

    if (stopRegistered_)
    {
        UnregisterHotKey(
            nullptr,
            StopHotkeyId);

        stopRegistered_ = false;
    }
}