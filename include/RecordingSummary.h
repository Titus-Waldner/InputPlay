#pragma once

#include "Recording.h"

#include <cstddef>
#include <cstdint>

struct RecordingSummary
{
    std::size_t totalEvents = 0;

    std::size_t mouseMovements = 0;
    std::size_t mouseTeleports = 0;
    std::size_t mouseButtonEvents = 0;
    std::size_t mouseWheelEvents = 0;
    std::size_t keyboardEvents = 0;
    std::size_t waitEvents = 0;

    std::uint64_t eventDurationMicroseconds = 0;
    std::uint64_t explicitWaitMicroseconds = 0;
    std::uint64_t totalDurationMicroseconds = 0;
};

RecordingSummary summarizeRecording(
    const Recording& recording);
