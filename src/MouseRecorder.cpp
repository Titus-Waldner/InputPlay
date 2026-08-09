#include "MouseRecorder.h"
#include "DisplayMetadata.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <chrono>
#include <cstdint>
#include <iostream>

namespace
{
constexpr int LeftMouseButton = 1;
constexpr int RightMouseButton = 2;
constexpr int MiddleMouseButton = 3;

Recording* activeRecording = nullptr;
std::chrono::steady_clock::time_point recordingStart;

std::uint64_t currentTimestampMicroseconds()
{
    const auto elapsed =
        std::chrono::steady_clock::now()
        - recordingStart;

    return static_cast<std::uint64_t>(
        std::chrono::duration_cast<
            std::chrono::microseconds>(
            elapsed)
            .count());
}

void setCursorPosition(InputEvent& event)
{
    POINT cursorPosition{};

    if (GetCursorPos(&cursorPosition))
    {
        event.mouseX = cursorPosition.x;
        event.mouseY = cursorPosition.y;
    }
}

void addMouseButtonEvent(
    EventType type,
    int mouseButton,
    std::uint64_t timestamp)
{
    InputEvent event;
    event.timestampMicroseconds = timestamp;
    event.type = type;
    event.mouseButton = mouseButton;

    setCursorPosition(event);
    activeRecording->addEvent(event);
}

void processRawMouseInput(const RAWMOUSE& mouse)
{
    if (activeRecording == nullptr)
    {
        return;
    }

    const std::uint64_t timestamp =
        currentTimestampMicroseconds();

    const bool isRelativeMovement =
        (mouse.usFlags & MOUSE_MOVE_ABSOLUTE) == 0;

    if (isRelativeMovement
        && (mouse.lLastX != 0 || mouse.lLastY != 0))
    {
        InputEvent movementEvent;
        movementEvent.timestampMicroseconds = timestamp;
        movementEvent.type = EventType::MouseMove;
        movementEvent.mouseDeltaX =
            static_cast<int>(mouse.lLastX);
        movementEvent.mouseDeltaY =
            static_cast<int>(mouse.lLastY);

        setCursorPosition(movementEvent);
        activeRecording->addEvent(movementEvent);
    }

    const USHORT buttonFlags =
        mouse.usButtonFlags;

    if ((buttonFlags & RI_MOUSE_LEFT_BUTTON_DOWN) != 0)
    {
        addMouseButtonEvent(
            EventType::MouseButtonDown,
            LeftMouseButton,
            timestamp);
    }

    if ((buttonFlags & RI_MOUSE_LEFT_BUTTON_UP) != 0)
    {
        addMouseButtonEvent(
            EventType::MouseButtonUp,
            LeftMouseButton,
            timestamp);
    }

    if ((buttonFlags & RI_MOUSE_RIGHT_BUTTON_DOWN) != 0)
    {
        addMouseButtonEvent(
            EventType::MouseButtonDown,
            RightMouseButton,
            timestamp);
    }

    if ((buttonFlags & RI_MOUSE_RIGHT_BUTTON_UP) != 0)
    {
        addMouseButtonEvent(
            EventType::MouseButtonUp,
            RightMouseButton,
            timestamp);
    }

    if ((buttonFlags & RI_MOUSE_MIDDLE_BUTTON_DOWN) != 0)
    {
        addMouseButtonEvent(
            EventType::MouseButtonDown,
            MiddleMouseButton,
            timestamp);
    }

    if ((buttonFlags & RI_MOUSE_MIDDLE_BUTTON_UP) != 0)
    {
        addMouseButtonEvent(
            EventType::MouseButtonUp,
            MiddleMouseButton,
            timestamp);
    }

    if ((buttonFlags & RI_MOUSE_WHEEL) != 0)
    {
        InputEvent wheelEvent;
        wheelEvent.timestampMicroseconds = timestamp;
        wheelEvent.type = EventType::MouseWheel;

        wheelEvent.mouseWheelDelta =
            static_cast<int>(
                static_cast<SHORT>(
                    mouse.usButtonData));

        setCursorPosition(wheelEvent);
        activeRecording->addEvent(wheelEvent);
    }
}

LRESULT CALLBACK recorderWindowProcedure(
    HWND window,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    if (message == WM_INPUT && activeRecording != nullptr)
    {
        UINT dataSize = 0;

        GetRawInputData(
            reinterpret_cast<HRAWINPUT>(lParam),
            RID_INPUT,
            nullptr,
            &dataSize,
            sizeof(RAWINPUTHEADER));

        if (dataSize == 0)
        {
            return 0;
        }

        BYTE buffer[sizeof(RAWINPUT)];

        if (dataSize > sizeof(buffer))
        {
            return 0;
        }

        const UINT bytesRead = GetRawInputData(
            reinterpret_cast<HRAWINPUT>(lParam),
            RID_INPUT,
            buffer,
            &dataSize,
            sizeof(RAWINPUTHEADER));

        if (bytesRead != dataSize)
        {
            return 0;
        }

        const RAWINPUT* rawInput =
            reinterpret_cast<const RAWINPUT*>(buffer);

        if (rawInput->header.dwType == RIM_TYPEMOUSE)
        {
            processRawMouseInput(
                rawInput->data.mouse);
        }

        return 0;
    }

    return DefWindowProc(
        window,
        message,
        wParam,
        lParam);
}
}

bool MouseRecorder::record(
    Recording& recording,
    std::string& errorMessage)
{
    const HINSTANCE instanceHandle =
        GetModuleHandle(nullptr);

    const wchar_t* windowClassName =
        L"InputPlayRawMouseRecorder";

    WNDCLASSW windowClass{};
    windowClass.lpfnWndProc = recorderWindowProcedure;
    windowClass.hInstance = instanceHandle;
    windowClass.lpszClassName = windowClassName;

    if (RegisterClassW(&windowClass) == 0)
    {
        const DWORD error = GetLastError();

        if (error != ERROR_CLASS_ALREADY_EXISTS)
        {
            errorMessage =
                "Unable to register the recording window.";

            return false;
        }
    }

    HWND window = CreateWindowExW(
        0,
        windowClassName,
        L"InputPlay Recorder",
        0,
        0,
        0,
        0,
        0,
        HWND_MESSAGE,
        nullptr,
        instanceHandle,
        nullptr);

    if (window == nullptr)
    {
        errorMessage =
            "Unable to create the recording window.";

        return false;
    }

    RAWINPUTDEVICE mouseDevice{};
    mouseDevice.usUsagePage = 0x01;
    mouseDevice.usUsage = 0x02;
    mouseDevice.dwFlags = RIDEV_INPUTSINK;
    mouseDevice.hwndTarget = window;

    if (!RegisterRawInputDevices(
            &mouseDevice,
            1,
            sizeof(mouseDevice)))
    {
        DestroyWindow(window);

        errorMessage =
            "Unable to register for raw mouse input.";

        return false;
    }

    recording.clear();

	POINT startingCursorPosition{};

	if (GetCursorPos(&startingCursorPosition))
	{
		recording.setStartingCursorPosition(
			startingCursorPosition.x,
			startingCursorPosition.y);
	}
	
	DisplayMetadata displayMetadata;
	std::string displayError;

	if (!captureDisplayMetadata(
			displayMetadata,
			displayError))
	{
		DestroyWindow(window);
		activeRecording = nullptr;

		errorMessage =
			"Unable to capture display configuration: "
			+ displayError;

		return false;
	}

	recording.setDisplayMetadata(displayMetadata);
	
	

	activeRecording = &recording;
	recordingStart = std::chrono::steady_clock::now();

    std::cout << "Recording mouse input\n";
    std::cout << "Movement, buttons, and wheel are enabled\n";
    std::cout << "Press F12 to stop recording\n";

    MSG message{};
    bool recordingActive = true;

    while (recordingActive)
    {
        while (PeekMessage(
            &message,
            nullptr,
            0,
            0,
            PM_REMOVE))
        {
            TranslateMessage(&message);
            DispatchMessage(&message);
        }

        if ((GetAsyncKeyState(VK_F12) & 0x8000) != 0)
        {
            recordingActive = false;
        }

        Sleep(1);
    }

    activeRecording = nullptr;
    DestroyWindow(window);

    while ((GetAsyncKeyState(VK_F12) & 0x8000) != 0)
    {
        Sleep(1);
    }

    errorMessage.clear();
    return true;
}