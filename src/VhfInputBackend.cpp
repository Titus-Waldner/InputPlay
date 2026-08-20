#include "VhfInputBackend.h"

#include "VhfDeviceProtocol.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <algorithm>
#include <cstddef>
#include <sstream>

namespace
{

constexpr unsigned char LeftButtonMask =
    0x01;

constexpr unsigned char RightButtonMask =
    0x02;

constexpr unsigned char MiddleButtonMask =
    0x04;

constexpr unsigned char BackButtonMask =
    0x08;

constexpr unsigned char ForwardButtonMask =
    0x10;

std::string makeWindowsError(
    const char* operation,
    DWORD errorCode)
{
    std::ostringstream message;

    message
        << operation
        << " Windows error: "
        << errorCode;

    return message.str();
}

int limitedReportValue(
    int value)
{
    return std::clamp(
        value,
        -127,
        127);
}

}

VhfInputBackend::VhfInputBackend(
    InputBackendType backendType)
    : backendType_(
          backendType)
{
}

VhfInputBackend::~VhfInputBackend()
{
    releaseAll();
    closeDriver();
}

bool VhfInputBackend::driverIsOpen() const
{
    return deviceHandle_ != nullptr
        && deviceHandle_
            != INVALID_HANDLE_VALUE;
}

bool VhfInputBackend::open(
    std::string& errorMessage)
{
    if (driverIsOpen())
    {
        errorMessage.clear();

        return true;
    }

    if (backendType_
		!= InputBackendType::VhfCorrectedRelative
		&& backendType_
			!= InputBackendType::VhfAbsolute
		&& backendType_
			!= InputBackendType::VhfNativeRelative)
    {
        errorMessage =
            "VhfInputBackend received an unsupported backend type.";

        return false;
    }

    HANDLE device =
        CreateFileW(
            L"\\\\.\\DriverLevelInputSimulator",
            GENERIC_READ
                | GENERIC_WRITE,
            0,
            nullptr,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            nullptr);

    if (device == INVALID_HANDLE_VALUE)
    {
        errorMessage =
            makeWindowsError(
                "Unable to open the Virtual HID driver.",
                GetLastError());

        deviceHandle_ =
            nullptr;

        return false;
    }

    deviceHandle_ =
        device;

    mouseButtons_ =
        0;

    keyboardModifiers_ =
        0;

    keyboardUsages_.fill(
        0);

    heldScanCodes_.clear();

    previousRecordedX_ =
        0;

    previousRecordedY_ =
        0;

    hasPreviousRecordedPosition_ =
        false;

    errorMessage.clear();

    return true;
}

void VhfInputBackend::closeDriver()
{
    if (!driverIsOpen())
    {
        deviceHandle_ =
            nullptr;

        return;
    }

    CloseHandle(
        static_cast<HANDLE>(
            deviceHandle_));

    deviceHandle_ =
        nullptr;
}

bool VhfInputBackend::submitMouseCommand(
    int movementX,
    int movementY,
    int verticalWheel,
    int horizontalWheel,
    std::string& errorMessage)
{
    if (!driverIsOpen()
        && !open(errorMessage))
    {
        return false;
    }

    const VhfDeviceProtocol::MouseCommand command
    {
        mouseButtons_,
        static_cast<signed char>(
            limitedReportValue(
                movementX)),
        static_cast<signed char>(
            limitedReportValue(
                movementY)),
        static_cast<signed char>(
            limitedReportValue(
                verticalWheel)),
        static_cast<signed char>(
            limitedReportValue(
                horizontalWheel))
    };

    DWORD bytesReturned =
        0;

    const BOOL result =
        DeviceIoControl(
            static_cast<HANDLE>(
                deviceHandle_),
            VhfDeviceProtocol::
                IoctlSubmitMouseReport,
            const_cast<
                VhfDeviceProtocol::MouseCommand*>(
                    &command),
            static_cast<DWORD>(
                sizeof(command)),
            nullptr,
            0,
            &bytesReturned,
            nullptr);

    if (!result)
    {
        errorMessage =
            makeWindowsError(
                "The Virtual HID driver rejected a mouse report.",
                GetLastError());

        return false;
    }

    errorMessage.clear();

    return true;
}

bool VhfInputBackend::submitRelativeMovement(
    int movementX,
    int movementY,
    std::string& errorMessage)
{
    int remainingX =
        movementX;

    int remainingY =
        movementY;

    if (remainingX == 0
        && remainingY == 0)
    {
        errorMessage.clear();

        return true;
    }

    while (remainingX != 0
           || remainingY != 0)
    {
        const int reportX =
            limitedReportValue(
                remainingX);

        const int reportY =
            limitedReportValue(
                remainingY);

        if (!submitMouseCommand(
                reportX,
                reportY,
                0,
                0,
                errorMessage))
        {
            return false;
        }

        remainingX -=
            reportX;

        remainingY -=
            reportY;
    }

    return true;
}

bool VhfInputBackend::submitAbsolutePosition(
    int screenX,
    int screenY,
    std::string& errorMessage)
{
    if (!driverIsOpen()
        && !open(errorMessage))
    {
        return false;
    }

    const int screenWidth =
        GetSystemMetrics(
            SM_CXSCREEN);

    const int screenHeight =
        GetSystemMetrics(
            SM_CYSCREEN);

    if (screenWidth <= 1
        || screenHeight <= 1)
    {
        errorMessage =
            "Windows reported an invalid primary monitor size.";

        return false;
    }

    if (screenX < 0
        || screenX >= screenWidth
        || screenY < 0
        || screenY >= screenHeight)
    {
        errorMessage =
            "The recorded cursor position is outside the "
            "current primary monitor.";

        return false;
    }

    constexpr unsigned long long absoluteMaximum =
        32767ULL;

    const unsigned long long xDenominator =
        static_cast<unsigned long long>(
            screenWidth - 1);

    const unsigned long long yDenominator =
        static_cast<unsigned long long>(
            screenHeight - 1);

    unsigned long long normalizedX =
        (static_cast<unsigned long long>(
             screenX)
             * absoluteMaximum
         + xDenominator / 2)
        / xDenominator;

    unsigned long long normalizedY =
        (static_cast<unsigned long long>(
             screenY)
             * absoluteMaximum
         + yDenominator / 2)
        / yDenominator;

    /*
     * The driver reserves zero so the top-left pixel is represented
     * by one, matching its move-to controller implementation.
     */
    if (normalizedX == 0)
    {
        normalizedX =
            1;
    }

    if (normalizedY == 0)
    {
        normalizedY =
            1;
    }

    const VhfDeviceProtocol::AbsoluteMouseCommand command
    {
        mouseButtons_,
        static_cast<unsigned short>(
            normalizedX),
        static_cast<unsigned short>(
            normalizedY)
    };

    DWORD bytesReturned =
        0;

    const BOOL result =
        DeviceIoControl(
            static_cast<HANDLE>(
                deviceHandle_),
            VhfDeviceProtocol::
                IoctlSubmitAbsoluteMouseReport,
            const_cast<
                VhfDeviceProtocol::
                    AbsoluteMouseCommand*>(
                        &command),
            static_cast<DWORD>(
                sizeof(command)),
            nullptr,
            0,
            &bytesReturned,
            nullptr);

    if (!result)
    {
        errorMessage =
            makeWindowsError(
                "The Virtual HID driver rejected an absolute "
                "mouse report.",
                GetLastError());

        return false;
    }

    errorMessage.clear();

    return true;
}

bool VhfInputBackend::sendMouseMovement(
    const InputEvent& event,
    std::string& errorMessage)
{
    if (backendType_
        == InputBackendType::VhfAbsolute)
    {
        return submitAbsolutePosition(
            event.mouseX,
            event.mouseY,
            errorMessage);
    }

    if (backendType_
        == InputBackendType::VhfNativeRelative)
    {
        return submitRelativeMovement(
            event.mouseDeltaX,
            event.mouseDeltaY,
            errorMessage);
    }

    if (backendType_
        != InputBackendType::VhfCorrectedRelative)
    {
        errorMessage =
            "VhfInputBackend received an unsupported backend type.";

        return false;
    }

    int movementX =
        event.mouseDeltaX;

    int movementY =
        event.mouseDeltaY;

    if (hasPreviousRecordedPosition_)
    {
        POINT currentPosition {};

        if (!GetCursorPos(
                &currentPosition))
        {
            errorMessage =
                makeWindowsError(
                    "Unable to read the current cursor position.",
                    GetLastError());

            return false;
        }

        const int errorX =
            previousRecordedX_
            - static_cast<int>(
                currentPosition.x);

        const int errorY =
            previousRecordedY_
            - static_cast<int>(
                currentPosition.y);

        movementX +=
            std::clamp(
                errorX,
                -4,
                4);

        movementY +=
            std::clamp(
                errorY,
                -4,
                4);
    }

    if (!submitRelativeMovement(
            movementX,
            movementY,
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

bool VhfInputBackend::prepareMousePosition(
    const InputEvent& event,
    std::string& errorMessage)
{
    if (backendType_
        == InputBackendType::VhfAbsolute)
    {
        return submitAbsolutePosition(
            event.mouseX,
            event.mouseY,
            errorMessage);
    }

    if (backendType_
        == InputBackendType::VhfNativeRelative)
    {
        errorMessage.clear();

        return true;
    }

    if (backendType_
        != InputBackendType::VhfCorrectedRelative)
    {
        errorMessage =
            "VhfInputBackend received an unsupported backend type.";

        return false;
    }

    POINT currentPosition {};

    if (!GetCursorPos(
            &currentPosition))
    {
        errorMessage =
            makeWindowsError(
                "Unable to read the current cursor position.",
                GetLastError());

        return false;
    }

    const int correctionX =
        std::clamp(
            event.mouseX
                - static_cast<int>(
                    currentPosition.x),
            -4,
            4);

    const int correctionY =
        std::clamp(
            event.mouseY
                - static_cast<int>(
                    currentPosition.y),
            -4,
            4);

    if (!submitRelativeMovement(
            correctionX,
            correctionY,
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

bool VhfInputBackend::updateMouseButton(
    int mouseButton,
    bool pressed,
    int screenX,
    int screenY,
    std::string& errorMessage)
{
    unsigned char buttonMask =
        0;

    switch (mouseButton)
    {
        case 1:
            buttonMask =
                LeftButtonMask;
            break;

        case 2:
            buttonMask =
                RightButtonMask;
            break;

        case 3:
            buttonMask =
                MiddleButtonMask;
            break;

        case 4:
            buttonMask =
                BackButtonMask;
            break;

        case 5:
            buttonMask =
                ForwardButtonMask;
            break;

        default:
            errorMessage =
                "The recording contains an unsupported mouse button.";

            return false;
    }

    if (pressed)
    {
        mouseButtons_ |=
            buttonMask;
    }
    else
    {
        mouseButtons_ &=
            static_cast<unsigned char>(
                ~buttonMask);
    }

    if (backendType_
        == InputBackendType::VhfAbsolute)
    {
        return submitAbsolutePosition(
            screenX,
            screenY,
            errorMessage);
    }

    return submitMouseCommand(
        0,
        0,
        0,
        0,
        errorMessage);
}

bool VhfInputBackend::translateScanCode(
    unsigned int packedScanCode,
    unsigned char& usage,
    unsigned char& modifierMask) const
{
    const bool extended =
        (packedScanCode & 0x100U)
        != 0;

    const unsigned int scanCode =
        packedScanCode
        & 0xFFU;

    const UINT mappedScanCode =
        extended
        ? scanCode | 0xE000U
        : scanCode;

    const UINT virtualKey =
        MapVirtualKeyW(
            mappedScanCode,
            MAPVK_VSC_TO_VK_EX);

    usage =
        0;

    modifierMask =
        0;

    switch (virtualKey)
    {
        case VK_LCONTROL:
            modifierMask = 0x01;
            return true;

        case VK_LSHIFT:
            modifierMask = 0x02;
            return true;

        case VK_LMENU:
            modifierMask = 0x04;
            return true;

        case VK_LWIN:
            modifierMask = 0x08;
            return true;

        case VK_RCONTROL:
            modifierMask = 0x10;
            return true;

        case VK_RSHIFT:
            modifierMask = 0x20;
            return true;

        case VK_RMENU:
            modifierMask = 0x40;
            return true;

        case VK_RWIN:
            modifierMask = 0x80;
            return true;

        default:
            break;
    }

    if (virtualKey >= 'A'
        && virtualKey <= 'Z')
    {
        usage =
            static_cast<unsigned char>(
                0x04
                + virtualKey
                - 'A');

        return true;
    }

    if (virtualKey >= '1'
        && virtualKey <= '9')
    {
        usage =
            static_cast<unsigned char>(
                0x1E
                + virtualKey
                - '1');

        return true;
    }

    if (virtualKey == '0')
    {
        usage =
            0x27;

        return true;
    }

    if (virtualKey >= VK_F1
        && virtualKey <= VK_F12)
    {
        usage =
            static_cast<unsigned char>(
                0x3A
                + virtualKey
                - VK_F1);

        return true;
    }

    switch (virtualKey)
    {
        case VK_RETURN:
            usage = 0x28;
            return true;

        case VK_ESCAPE:
            usage = 0x29;
            return true;

        case VK_BACK:
            usage = 0x2A;
            return true;

        case VK_TAB:
            usage = 0x2B;
            return true;

        case VK_SPACE:
            usage = 0x2C;
            return true;

        case VK_OEM_MINUS:
            usage = 0x2D;
            return true;

        case VK_OEM_PLUS:
            usage = 0x2E;
            return true;

        case VK_OEM_4:
            usage = 0x2F;
            return true;

        case VK_OEM_6:
            usage = 0x30;
            return true;

        case VK_OEM_5:
            usage = 0x31;
            return true;

        case VK_OEM_1:
            usage = 0x33;
            return true;

        case VK_OEM_7:
            usage = 0x34;
            return true;

        case VK_OEM_3:
            usage = 0x35;
            return true;

        case VK_OEM_COMMA:
            usage = 0x36;
            return true;

        case VK_OEM_PERIOD:
            usage = 0x37;
            return true;

        case VK_OEM_2:
            usage = 0x38;
            return true;

        case VK_CAPITAL:
            usage = 0x39;
            return true;

        case VK_SNAPSHOT:
            usage = 0x46;
            return true;

        case VK_SCROLL:
            usage = 0x47;
            return true;

        case VK_PAUSE:
            usage = 0x48;
            return true;

        case VK_INSERT:
            usage = 0x49;
            return true;

        case VK_HOME:
            usage = 0x4A;
            return true;

        case VK_PRIOR:
            usage = 0x4B;
            return true;

        case VK_DELETE:
            usage = 0x4C;
            return true;

        case VK_END:
            usage = 0x4D;
            return true;

        case VK_NEXT:
            usage = 0x4E;
            return true;

        case VK_RIGHT:
            usage = 0x4F;
            return true;

        case VK_LEFT:
            usage = 0x50;
            return true;

        case VK_DOWN:
            usage = 0x51;
            return true;

        case VK_UP:
            usage = 0x52;
            return true;

        case VK_NUMLOCK:
            usage = 0x53;
            return true;

        case VK_DIVIDE:
            usage = 0x54;
            return true;

        case VK_MULTIPLY:
            usage = 0x55;
            return true;

        case VK_SUBTRACT:
            usage = 0x56;
            return true;

        case VK_ADD:
            usage = 0x57;
            return true;

        case VK_DECIMAL:
            usage = 0x63;
            return true;

        default:
            return false;
    }
}

bool VhfInputBackend::submitKeyboardState(
    std::string& errorMessage)
{
    if (!driverIsOpen()
        && !open(errorMessage))
    {
        return false;
    }

    VhfDeviceProtocol::KeyboardCommand command {};

    command.modifiers =
        keyboardModifiers_;

    for (std::size_t index = 0;
         index < keyboardUsages_.size();
         ++index)
    {
        command.keys[index] =
            keyboardUsages_[index];
    }

    DWORD bytesReturned =
        0;

    const BOOL result =
        DeviceIoControl(
            static_cast<HANDLE>(
                deviceHandle_),
            VhfDeviceProtocol::
                IoctlSubmitKeyboardReport,
            &command,
            static_cast<DWORD>(
                sizeof(command)),
            nullptr,
            0,
            &bytesReturned,
            nullptr);

    if (!result)
    {
        errorMessage =
            makeWindowsError(
                "The Virtual HID driver rejected a keyboard report.",
                GetLastError());

        return false;
    }

    errorMessage.clear();

    return true;
}

bool VhfInputBackend::updateKeyboardState(
    unsigned int packedScanCode,
    bool pressed,
    std::string& errorMessage)
{
    unsigned char usage =
        0;

    unsigned char modifierMask =
        0;

    if (!translateScanCode(
            packedScanCode,
            usage,
            modifierMask))
    {
        errorMessage =
            "The recording contains a keyboard scan code that "
            "cannot yet be translated to a HID usage.";

        return false;
    }

    if (pressed)
    {
        if (heldScanCodes_.contains(
                packedScanCode))
        {
            errorMessage.clear();

            return true;
        }

        if (modifierMask != 0)
        {
            keyboardModifiers_ |=
                modifierMask;
        }
        else
        {
            const auto emptyPosition =
                std::find(
                    keyboardUsages_.begin(),
                    keyboardUsages_.end(),
                    static_cast<unsigned char>(
                        0));

            if (emptyPosition
                == keyboardUsages_.end())
            {
                errorMessage =
                    "The Virtual HID keyboard cannot hold more "
                    "than six non-modifier keys simultaneously.";

                return false;
            }

            *emptyPosition =
                usage;
        }

        heldScanCodes_.insert(
            packedScanCode);
    }
    else
    {
        if (modifierMask != 0)
        {
            keyboardModifiers_ &=
                static_cast<unsigned char>(
                    ~modifierMask);
        }
        else
        {
            const auto usagePosition =
                std::find(
                    keyboardUsages_.begin(),
                    keyboardUsages_.end(),
                    usage);

            if (usagePosition
                != keyboardUsages_.end())
            {
                *usagePosition =
                    0;
            }
        }

        heldScanCodes_.erase(
            packedScanCode);
    }

    return submitKeyboardState(
        errorMessage);
}

bool VhfInputBackend::execute(
    const InputEvent& event,
    std::string& errorMessage)
{
    switch (event.type)
    {
        case EventType::MouseMove:
            return sendMouseMovement(
                event,
                errorMessage);

        case EventType::MouseTeleport:
            if (backendType_
                == InputBackendType::VhfAbsolute)
            {
                return submitAbsolutePosition(
                    event.mouseX,
                    event.mouseY,
                    errorMessage);
            }

            if (!SetCursorPos(
                    event.mouseX,
                    event.mouseY))
            {
                errorMessage =
                    makeWindowsError(
                        "Windows was unable to teleport the cursor.",
                        GetLastError());

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

            return updateMouseButton(
                event.mouseButton,
                true,
                event.mouseX,
                event.mouseY,
                errorMessage);

        case EventType::MouseButtonUp:
            if (!prepareMousePosition(
                    event,
                    errorMessage))
            {
                return false;
            }

            return updateMouseButton(
                event.mouseButton,
                false,
                event.mouseX,
                event.mouseY,
                errorMessage);

        case EventType::MouseWheel:
            if (!prepareMousePosition(
                    event,
                    errorMessage))
            {
                return false;
            }

            /*
             * The absolute report does not contain wheel fields.
             * Submit the wheel through the relative report after
             * anchoring the cursor with the selected positioning mode.
             */
            return submitMouseCommand(
                0,
                0,
                event.mouseWheelDelta,
                0,
                errorMessage);

        case EventType::KeyDown:
            return updateKeyboardState(
                event.keyCode,
                true,
                errorMessage);

        case EventType::KeyUp:
            return updateKeyboardState(
                event.keyCode,
                false,
                errorMessage);

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

void VhfInputBackend::releaseAll()
{
    if (!driverIsOpen())
    {
        mouseButtons_ =
            0;

        keyboardModifiers_ =
            0;

        keyboardUsages_.fill(
            0);

        heldScanCodes_.clear();

        hasPreviousRecordedPosition_ =
            false;

        return;
    }

    std::string ignoredError;

    mouseButtons_ =
        0;

    if (backendType_
        == InputBackendType::VhfAbsolute)
    {
        POINT currentPosition {};

        if (GetCursorPos(
                &currentPosition))
        {
            submitAbsolutePosition(
                currentPosition.x,
                currentPosition.y,
                ignoredError);
        }
    }

    submitMouseCommand(
        0,
        0,
        0,
        0,
        ignoredError);

    keyboardModifiers_ =
        0;

    keyboardUsages_.fill(
        0);

    heldScanCodes_.clear();

    submitKeyboardState(
        ignoredError);

    hasPreviousRecordedPosition_ =
        false;
}