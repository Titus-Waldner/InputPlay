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


bool sendAbsoluteMouseInput(
    int screenX,
    int screenY,
    DWORD mouseData,
    DWORD additionalFlags,
    std::string& errorMessage)
{
    const int virtualLeft =
        GetSystemMetrics(
            SM_XVIRTUALSCREEN);

    const int virtualTop =
        GetSystemMetrics(
            SM_YVIRTUALSCREEN);

    const int virtualWidth =
        GetSystemMetrics(
            SM_CXVIRTUALSCREEN);

    const int virtualHeight =
        GetSystemMetrics(
            SM_CYVIRTUALSCREEN);

    if (virtualWidth <= 1
        || virtualHeight <= 1)
    {
        errorMessage =
            "Windows reported an invalid virtual desktop size.";

        return false;
    }

    int relativeX =
        screenX - virtualLeft;

    int relativeY =
        screenY - virtualTop;

    if (relativeX < 0)
    {
        relativeX =
            0;
    }
    else if (relativeX >= virtualWidth)
    {
        relativeX =
            virtualWidth - 1;
    }

    if (relativeY < 0)
    {
        relativeY =
            0;
    }
    else if (relativeY >= virtualHeight)
    {
        relativeY =
            virtualHeight - 1;
    }

    const LONG normalizedX =
        MulDiv(
            relativeX,
            65535,
            virtualWidth - 1);

    const LONG normalizedY =
        MulDiv(
            relativeY,
            65535,
            virtualHeight - 1);

    return sendMouseInput(
        normalizedX,
        normalizedY,
        mouseData,
        MOUSEEVENTF_MOVE
            | MOUSEEVENTF_ABSOLUTE
            | MOUSEEVENTF_VIRTUALDESK
            | additionalFlags,
        errorMessage);
}


bool sendKeyboardInput(
    unsigned int packedScanCode,
    bool keyReleased,
    std::string& errorMessage)
{
    const bool extendedKey =
        (packedScanCode & 0x100) != 0;

    const WORD scanCode =
        static_cast<WORD>(
            packedScanCode & 0xFF);

    INPUT input{};
    input.type = INPUT_KEYBOARD;
    input.ki.wVk = 0;
    input.ki.wScan = scanCode;
    input.ki.dwFlags = KEYEVENTF_SCANCODE;

    if (extendedKey)
    {
        input.ki.dwFlags |= KEYEVENTF_EXTENDEDKEY;
    }

    if (keyReleased)
    {
        input.ki.dwFlags |= KEYEVENTF_KEYUP;
    }

    const UINT sentCount = SendInput(
        1,
        &input,
        sizeof(INPUT));

    if (sentCount != 1)
    {
        errorMessage =
            "Windows was unable to send the keyboard input.";

        return false;
    }

    errorMessage.clear();
    return true;
}
}

SendInputBackend::SendInputBackend()
    : backendType_(
          InputBackendType::SendInputAbsolute)
{
}

SendInputBackend::SendInputBackend(
    InputBackendType backendType)
    : backendType_(
          backendType)
{
}

bool SendInputBackend::correctToPosition(
    int destinationX,
    int destinationY,
    std::string& errorMessage)
{
    POINT currentPosition {};

    if (!GetCursorPos(
            &currentPosition))
    {
        errorMessage =
            "Windows was unable to read the cursor position.";

        return false;
    }

    const LONG correctionX =
        static_cast<LONG>(
            destinationX
            - currentPosition.x);

    const LONG correctionY =
        static_cast<LONG>(
            destinationY
            - currentPosition.y);

    if (correctionX == 0
        && correctionY == 0)
    {
        errorMessage.clear();

        return true;
    }

    return sendMouseInput(
        correctionX,
        correctionY,
        0,
        MOUSEEVENTF_MOVE,
        errorMessage);
}

bool SendInputBackend::sendCorrectedRelativeMove(
    const InputEvent& event,
    std::string& errorMessage)
{
    LONG correctedDeltaX =
        static_cast<LONG>(
            event.mouseDeltaX);

    LONG correctedDeltaY =
        static_cast<LONG>(
            event.mouseDeltaY);

    /*
     * Measure the confirmed error left by the preceding movement
     * and incorporate that error into the current relative report.
     *
     * This adds no sleeps and does not change playback timestamps.
     */
    if (hasPreviousRecordedPosition_)
    {
        POINT currentPosition {};

        if (!GetCursorPos(
                &currentPosition))
        {
            errorMessage =
                "Windows was unable to read the cursor position.";

            return false;
        }

        correctedDeltaX +=
            static_cast<LONG>(
                previousRecordedX_
                - currentPosition.x);

        correctedDeltaY +=
            static_cast<LONG>(
                previousRecordedY_
                - currentPosition.y);
    }

    if (!sendMouseInput(
            correctedDeltaX,
            correctedDeltaY,
            0,
            MOUSEEVENTF_MOVE,
            errorMessage))
    {
        return false;
    }

    previousRecordedX_ =
        event.mouseX;

    previousRecordedY_ =
        event.mouseY;

    hasPreviousRecordedPosition_ =
        true;

    return true;
}

bool SendInputBackend::prepareMousePosition(
    const InputEvent& event,
    std::string& errorMessage)
{
    if (backendType_
        == InputBackendType::SendInputAbsolute)
    {
        return sendAbsoluteMouseInput(
            event.mouseX,
            event.mouseY,
            0,
            0,
            errorMessage);
    }

    if (backendType_
        == InputBackendType::
            SendInputCorrectedRelative)
    {
        if (!correctToPosition(
                event.mouseX,
                event.mouseY,
                errorMessage))
        {
            return false;
        }

        previousRecordedX_ =
            event.mouseX;

        previousRecordedY_ =
            event.mouseY;

        hasPreviousRecordedPosition_ =
            true;

        return true;
    }

    errorMessage =
        "SendInputBackend received an unsupported backend type.";

    return false;
}

bool SendInputBackend::execute(
    const InputEvent& event,
    std::string& errorMessage)
{
    switch (event.type)
    {
        case EventType::MouseMove:
            if (backendType_
                == InputBackendType::SendInputAbsolute)
            {
                return sendAbsoluteMouseInput(
                    event.mouseX,
                    event.mouseY,
                    0,
                    0,
                    errorMessage);
            }

            if (backendType_
                == InputBackendType::
                    SendInputCorrectedRelative)
            {
                return sendCorrectedRelativeMove(
                    event,
                    errorMessage);
            }

            errorMessage =
                "SendInputBackend received an unsupported backend type.";

            return false;

        case EventType::MouseTeleport:
            if (!SetCursorPos(
                    event.mouseX,
                    event.mouseY))
            {
                errorMessage =
                    "Windows was unable to teleport the cursor.";

                return false;
            }

            previousRecordedX_ =
                event.mouseX;

            previousRecordedY_ =
                event.mouseY;

            hasPreviousRecordedPosition_ =
                true;

            errorMessage.clear();

            return true;

        case EventType::MouseButtonDown:
            if (!prepareMousePosition(
                    event,
                    errorMessage))
            {
                return false;
            }

            switch (event.mouseButton)
            {
                case 1:
                    if (!sendMouseInput(
                            0,
                            0,
                            0,
                            MOUSEEVENTF_LEFTDOWN,
                            errorMessage))
                    {
                        return false;
                    }

                    leftButtonHeld_ =
                        true;

                    return true;

                case 2:
                    if (!sendMouseInput(
                            0,
                            0,
                            0,
                            MOUSEEVENTF_RIGHTDOWN,
                            errorMessage))
                    {
                        return false;
                    }

                    rightButtonHeld_ =
                        true;

                    return true;

                case 3:
                    if (!sendMouseInput(
                            0,
                            0,
                            0,
                            MOUSEEVENTF_MIDDLEDOWN,
                            errorMessage))
                    {
                        return false;
                    }

                    middleButtonHeld_ =
                        true;

                    return true;

                default:
                    errorMessage =
                        "The recording contains an unsupported mouse button.";

                    return false;
            }

        case EventType::MouseButtonUp:
            if (!prepareMousePosition(
                    event,
                    errorMessage))
            {
                return false;
            }

            switch (event.mouseButton)
            {
                case 1:
                    if (!sendMouseInput(
                            0,
                            0,
                            0,
                            MOUSEEVENTF_LEFTUP,
                            errorMessage))
                    {
                        return false;
                    }

                    leftButtonHeld_ =
                        false;

                    return true;

                case 2:
                    if (!sendMouseInput(
                            0,
                            0,
                            0,
                            MOUSEEVENTF_RIGHTUP,
                            errorMessage))
                    {
                        return false;
                    }

                    rightButtonHeld_ =
                        false;

                    return true;

                case 3:
                    if (!sendMouseInput(
                            0,
                            0,
                            0,
                            MOUSEEVENTF_MIDDLEUP,
                            errorMessage))
                    {
                        return false;
                    }

                    middleButtonHeld_ =
                        false;

                    return true;

                default:
                    errorMessage =
                        "The recording contains an unsupported mouse button.";

                    return false;
            }

        case EventType::MouseWheel:
            if (!prepareMousePosition(
                    event,
                    errorMessage))
            {
                return false;
            }

            return sendMouseInput(
                0,
                0,
                static_cast<DWORD>(
                    event.mouseWheelDelta),
                MOUSEEVENTF_WHEEL,
                errorMessage);

        case EventType::KeyDown:
            if (!sendKeyboardInput(
                    event.keyCode,
                    false,
                    errorMessage))
            {
                return false;
            }

            heldKeys_.insert(
                event.keyCode);

            return true;

        case EventType::KeyUp:
            if (!sendKeyboardInput(
                    event.keyCode,
                    true,
                    errorMessage))
            {
                return false;
            }

            heldKeys_.erase(
                event.keyCode);

            return true;

        case EventType::Wait:
            errorMessage =
                "Wait events must be handled by the playback scheduler.";

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
	
	for (const unsigned int keyCode : heldKeys_)
	{
		sendKeyboardInput(
			keyCode,
			true,
			ignoredError);
	}

	heldKeys_.clear();
}
