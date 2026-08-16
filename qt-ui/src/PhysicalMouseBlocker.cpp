#include "PhysicalMouseBlocker.h"

#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace
{
HHOOK mouseHook =
    nullptr;

LRESULT CALLBACK physicalMouseHook(
    int code,
    WPARAM message,
    LPARAM data)
{
    if (code < 0)
    {
        return CallNextHookEx(
            mouseHook,
            code,
            message,
            data);
    }

    const MSLLHOOKSTRUCT* mouseData =
        reinterpret_cast<const MSLLHOOKSTRUCT*>(
            data);

    if (!mouseData)
    {
        return CallNextHookEx(
            mouseHook,
            code,
            message,
            data);
    }

    /*
     * Allow injected input from SendInput so playback can continue.
     * Suppress physical mouse movement, buttons, and wheel input.
     */
    if ((mouseData->flags & LLMHF_INJECTED) != 0)
    {
        return CallNextHookEx(
            mouseHook,
            code,
            message,
            data);
    }

    return 1;
}
}

#endif

bool PhysicalMouseBlocker::enable()
{
#ifdef _WIN32
    if (mouseHook)
    {
        return true;
    }

    mouseHook =
        SetWindowsHookExW(
            WH_MOUSE_LL,
            physicalMouseHook,
            GetModuleHandleW(nullptr),
            0);

    return mouseHook != nullptr;
#else
    return false;
#endif
}

void PhysicalMouseBlocker::disable()
{
#ifdef _WIN32
    if (!mouseHook)
    {
        return;
    }

    UnhookWindowsHookEx(
        mouseHook);

    mouseHook =
        nullptr;
#endif
}

bool PhysicalMouseBlocker::isEnabled()
{
#ifdef _WIN32
    return mouseHook != nullptr;
#else
    return false;
#endif
}