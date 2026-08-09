#include "SendInputBackend.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

bool SendInputBackend::execute(
    const InputEvent& event,
    std::string& errorMessage)
{
    if (event.type != EventType::MouseMove)
    {
        errorMessage =
            "Live playback currently supports mouse movement only.";

        return false;
    }

    INPUT input{};
    input.type = INPUT_MOUSE;
    input.mi.dx = event.mouseDeltaX;
    input.mi.dy = event.mouseDeltaY;
    input.mi.dwFlags = MOUSEEVENTF_MOVE;

    const UINT sentCount = SendInput(
        1,
        &input,
        sizeof(INPUT));

    if (sentCount != 1)
    {
        errorMessage =
            "Windows was unable to send the mouse movement.";

        return false;
    }

    errorMessage.clear();
    return true;
}

void SendInputBackend::releaseAll()
{
    // This version only supports movement, so there are
    // no mouse buttons or keyboard keys to release yet.
}