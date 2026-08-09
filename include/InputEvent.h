#pragma once

#include <cstdint>

enum class EventType
{
    MouseMove,
    MouseButtonDown,
    MouseButtonUp,
    MouseWheel,
    KeyDown,
    KeyUp,
    Wait
};

struct InputEvent
{
    std::uint64_t timestampMicroseconds = 0;
    EventType type = EventType::MouseMove;

    int mouseX = 0;
    int mouseY = 0;
    int mouseDeltaX = 0;
    int mouseDeltaY = 0;
    int mouseButton = 0;
    int mouseWheelDelta = 0;

    unsigned int keyCode = 0;

    std::uint64_t waitMicroseconds = 0;
};