#include "MouseRecorder.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <chrono>
#include <iostream>

namespace
{
Recording* activeRecording = nullptr;
std::chrono::steady_clock::time_point recordingStart;

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

        if (rawInput->header.dwType != RIM_TYPEMOUSE)
        {
            return 0;
        }

        const LONG deltaX =
            rawInput->data.mouse.lLastX;

        const LONG deltaY =
            rawInput->data.mouse.lLastY;

        if (deltaX == 0 && deltaY == 0)
        {
            return 0;
        }

        const auto elapsed =
            std::chrono::steady_clock::now()
            - recordingStart;

        InputEvent event;
        event.timestampMicroseconds =
            static_cast<std::uint64_t>(
                std::chrono::duration_cast<
                    std::chrono::microseconds>(
                    elapsed)
                    .count());

        event.type = EventType::MouseMove;
        event.mouseDeltaX = static_cast<int>(deltaX);
        event.mouseDeltaY = static_cast<int>(deltaY);

        POINT cursorPosition{};

        if (GetCursorPos(&cursorPosition))
        {
            event.mouseX = cursorPosition.x;
            event.mouseY = cursorPosition.y;
        }

        activeRecording->addEvent(event);
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
    activeRecording = &recording;
    recordingStart = std::chrono::steady_clock::now();

    std::cout << "Recording mouse movement\n";
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

    // Wait for F12 to be released before returning.
    while ((GetAsyncKeyState(VK_F12) & 0x8000) != 0)
    {
        Sleep(1);
    }

    errorMessage.clear();
    return true;
}