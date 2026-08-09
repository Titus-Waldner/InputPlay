#include "SendInputBackend.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <string>

namespace
{
constexpr int LeftMouseButton = 1;
constexpr int RightMouseButton = 2;
constexpr int MiddleMouseButton = 3;

bool sendMouseInput(
    LONG deltaX,
    LONG deltaY,
    DWORD mouseData,
    DWORD flags,
    std::string& errorMessage)
{
    INPUT input{};
    input.type = INPUT_MOUSE;
    input.mi.dx = deltaX;
    input.mi.dy = deltaY;
    input.mi.mouseData = mouseData;
    input.mi.dwFlags = flags;

    const UINT sentCount = SendInput(
        1,
        &input,
        sizeof(INPUT));

    if (sentCount != 1)
    {
        errorMessage =
            "Windows was unable to send the mouse input.";

        return false;
    }

    errorMessage.clear();
    return true;
}
}

bool SendInputBackend::execute(
    const InputEvent& event,
    std::string& errorMessage)
{
    switch (event.type)
    {
        case EventType::MouseMove:
            return sendMouseInput(
                event.mouseDeltaX,
                event.mouseDeltaY,
                0,
                MOUSEEVENTF_MOVE,
                errorMessage);

        case EventType::MouseButtonDown:
            switch (event.mouseButton)
            {
                case LeftMouseButton:
                    leftButtonHeld_ = true;

                    return sendMouseInput(
                        0,
                        0,
                        0,
                        MOUSEEVENTF_LEFTDOWN,
                        errorMessage);

                case RightMouseButton:
                    rightButtonHeld_ = true;

                    return sendMouseInput(
                        0,
                        0,
                        0,
                        MOUSEEVENTF_RIGHTDOWN,
                        errorMessage);

                case MiddleMouseButton:
                    middleButtonHeld_ = true;

                    return sendMouseInput(
                        0,
                        0,
                        0,
                        MOUSEEVENTF_MIDDLEDOWN,
                        errorMessage);

                default:
                    errorMessage =
                        "The recording contains an unsupported mouse button.";

                    return false;
            }

        case EventType::MouseButtonUp:
            switch (event.mouseButton)
            {
                case LeftMouseButton:
                    leftButtonHeld_ = false;

                    return sendMouseInput(
                        0,
                        0,
                        0,
                        MOUSEEVENTF_LEFTUP,
                        errorMessage);

                case RightMouseButton:
                    rightButtonHeld_ = false;

                    return sendMouseInput(
                        0,
                        0,
                        0,
                        MOUSEEVENTF_RIGHTUP,
                        errorMessage);

                case MiddleMouseButton:
                    middleButtonHeld_ = false;

                    return sendMouseInput(
                        0,
                        0,
                        0,
                        MOUSEEVENTF_MIDDLEUP,
                        errorMessage);

                default:
                    errorMessage =
                        "The recording contains an unsupported mouse button.";

                    return false;
            }

        case EventType::MouseWheel:
            return sendMouseInput(
                0,
                0,
                static_cast<DWORD>(event.mouseWheelDelta),
                MOUSEEVENTF_WHEEL,
                errorMessage);

        case EventType::KeyDown:
        case EventType::KeyUp:
            errorMessage =
                "Live keyboard playback is not implemented yet.";

            return false;

        default:
            errorMessage =
                "The recording contains an unknown input event.";

            return false;
    }
}

void SendInputBackend::releaseAll()
{
    std::string ignoredError;

    if (leftButtonHeld_)
    {
        sendMouseInput(
            0,
            0,
            0,
            MOUSEEVENTF_LEFTUP,
            ignoredError);

        leftButtonHeld_ = false;
    }

    if (rightButtonHeld_)
    {
        sendMouseInput(
            0,
            0,
            0,
            MOUSEEVENTF_RIGHTUP,
            ignoredError);

        rightButtonHeld_ = false;
    }

    if (middleButtonHeld_)
    {
        sendMouseInput(
            0,
            0,
            0,
            MOUSEEVENTF_MIDDLEUP,
            ignoredError);

        middleButtonHeld_ = false;
    }
}
