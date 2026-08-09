#include "MouseRecorder.h"
#include "DisplayMetadata.h"
#include "Settings.h"

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

using RecordingClock = std::chrono::steady_clock;

Recording* activeRecording = nullptr;

RecordingClock::time_point recordingStart;
RecordingClock::time_point pauseStart;

RecordingClock::duration accumulatedPausedTime{};

int recordStartKey = 0;
int recordPauseKey = 0;
int recordStopKey = 0;

bool recordingStarted = false;
bool recordingPaused = false;

std::uint64_t currentTimestampMicroseconds()
{
    const RecordingClock::time_point now =
        RecordingClock::now();

    const RecordingClock::duration elapsed =
        now
        - recordingStart
        - accumulatedPausedTime;

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
    if (activeRecording == nullptr
    || !recordingStarted
    || recordingPaused)
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

bool isRecordingControlKey(
    unsigned int virtualKey)
{
    return
        virtualKey
            == static_cast<unsigned int>(
                recordStartKey)
        || virtualKey
            == static_cast<unsigned int>(
                recordPauseKey)
        || virtualKey
            == static_cast<unsigned int>(
                recordStopKey);
}

void processRawKeyboardInput(const RAWKEYBOARD& keyboard)
{
    if (activeRecording == nullptr || !recordingStarted || recordingPaused)
	{
		return;
	}

    // F12 is reserved for stopping the recording.
    if (isRecordingControlKey(keyboard.VKey))
	{
		return;
	}

    const bool keyReleased =
        (keyboard.Flags & RI_KEY_BREAK) != 0;

    const bool extendedKey =
        (keyboard.Flags & RI_KEY_E0) != 0;

    unsigned int packedScanCode =
        static_cast<unsigned int>(keyboard.MakeCode);

    if (extendedKey)
    {
        packedScanCode |= 0x100;
    }

    InputEvent event;
    event.timestampMicroseconds =
        currentTimestampMicroseconds();

    event.type =
        keyReleased
        ? EventType::KeyUp
        : EventType::KeyDown;

    event.keyCode = packedScanCode;

    activeRecording->addEvent(event);
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
		else if (rawInput->header.dwType == RIM_TYPEKEYBOARD)
		{
			processRawKeyboardInput(
				rawInput->data.keyboard);
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
    const Settings& settings,
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

    RAWINPUTDEVICE inputDevices[2]{};

	inputDevices[0].usUsagePage = 0x01;
	inputDevices[0].usUsage = 0x02;
	inputDevices[0].dwFlags = RIDEV_INPUTSINK;
	inputDevices[0].hwndTarget = window;

	inputDevices[1].usUsagePage = 0x01;
	inputDevices[1].usUsage = 0x06;
	inputDevices[1].dwFlags = RIDEV_INPUTSINK;
	inputDevices[1].hwndTarget = window;

	if (!RegisterRawInputDevices(
			inputDevices,
			2,
			sizeof(RAWINPUTDEVICE)))
	{
		DestroyWindow(window);

		errorMessage =
			"Unable to register for raw mouse and keyboard input.";

		return false;
	}
	
	recording.clear();

	recordStartKey = settings.recordStartKey;
	recordPauseKey = settings.recordPauseKey;
	recordStopKey = settings.recordStopKey;

	recordingStarted = false;
	recordingPaused = false;

	activeRecording = &recording;

	std::cout << "Recording armed\n";
	std::cout
		<< "Press "
		<< keyNameFromVirtualKey(recordStartKey)
		<< " to start recording\n";

	std::cout
		<< "Press "
		<< keyNameFromVirtualKey(recordStopKey)
		<< " to cancel before recording starts\n";

	MSG message{};

	bool startKeyWasDown =
		(GetAsyncKeyState(recordStartKey) & 0x8000) != 0;

	bool stopKeyWasDown =
		(GetAsyncKeyState(recordStopKey) & 0x8000) != 0;

	bool cancelledBeforeStart = false;

	while (!recordingStarted)
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

		const bool startKeyIsDown =
			(GetAsyncKeyState(recordStartKey) & 0x8000) != 0;

		const bool stopKeyIsDown =
			(GetAsyncKeyState(recordStopKey) & 0x8000) != 0;

		if (stopKeyIsDown && !stopKeyWasDown)
		{
			cancelledBeforeStart = true;
			break;
		}

		if (startKeyIsDown && !startKeyWasDown)
		{
			while ((GetAsyncKeyState(recordStartKey)
					& 0x8000) != 0)
			{
				Sleep(1);
			}

			recordingStarted = true;
			break;
		}

		startKeyWasDown = startKeyIsDown;
		stopKeyWasDown = stopKeyIsDown;

		Sleep(1);
	}

	if (cancelledBeforeStart)
	{
		activeRecording = nullptr;
		DestroyWindow(window);

		while ((GetAsyncKeyState(recordStopKey)
				& 0x8000) != 0)
		{
			Sleep(1);
		}

		errorMessage =
			"Recording was cancelled before it started.";

		return false;
	}

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
		activeRecording = nullptr;
		DestroyWindow(window);

		errorMessage =
			"Unable to capture display configuration: "
			+ displayError;

		return false;
	}

	recording.setDisplayMetadata(displayMetadata);

	recordingStart = RecordingClock::now();

	accumulatedPausedTime =
		RecordingClock::duration::zero();

	recordingPaused = false;

	std::cout << "Recording started\n";
	std::cout
		<< "Press "
		<< keyNameFromVirtualKey(recordPauseKey)
		<< " to pause or resume\n";

	std::cout
		<< "Press "
		<< keyNameFromVirtualKey(recordStopKey)
		<< " to stop and save\n";

	bool recordingActive = true;

	stopKeyWasDown =
		(GetAsyncKeyState(recordStopKey) & 0x8000) != 0;

	bool pauseKeyWasDown =
		(GetAsyncKeyState(recordPauseKey) & 0x8000) != 0;

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

		const bool stopKeyIsDown =
			(GetAsyncKeyState(recordStopKey) & 0x8000) != 0;

		if (stopKeyIsDown && !stopKeyWasDown)
		{
			recordingActive = false;
		}

		stopKeyWasDown = stopKeyIsDown;

		if (!recordingActive)
		{
			break;
		}

		const bool pauseKeyIsDown =
			(GetAsyncKeyState(recordPauseKey) & 0x8000) != 0;

		if (pauseKeyIsDown && !pauseKeyWasDown)
		{
			if (!recordingPaused)
			{
				recordingPaused = true;
				pauseStart = RecordingClock::now();

				std::cout
					<< "Recording paused\n";

				std::cout
					<< "Press "
					<< keyNameFromVirtualKey(recordPauseKey)
					<< " to resume or "
					<< keyNameFromVirtualKey(recordStopKey)
					<< " to stop and save\n";
			}
			else
			{
				const RecordingClock::time_point resumeTime =
					RecordingClock::now();

				accumulatedPausedTime +=
					resumeTime - pauseStart;

				recordingPaused = false;

				POINT resumedCursorPosition{};

				if (GetCursorPos(&resumedCursorPosition))
				{
					InputEvent teleportEvent;

					teleportEvent.timestampMicroseconds =
						currentTimestampMicroseconds();

					teleportEvent.type =
						EventType::MouseTeleport;

					teleportEvent.mouseX =
						resumedCursorPosition.x;

					teleportEvent.mouseY =
						resumedCursorPosition.y;

					activeRecording->addEvent(
						teleportEvent);
				}

				std::cout
					<< "Recording resumed\n";
			}
		}

		pauseKeyWasDown = pauseKeyIsDown;

		Sleep(1);
		
		
	}

	if (recordingPaused)
	{
		accumulatedPausedTime +=
			RecordingClock::now() - pauseStart;
	}

	recordingStarted = false;
	recordingPaused = false;
	activeRecording = nullptr;

	DestroyWindow(window);

	while ((GetAsyncKeyState(recordStopKey)
			& 0x8000) != 0)
	{
		Sleep(1);
	}

	errorMessage.clear();
	return true;
}