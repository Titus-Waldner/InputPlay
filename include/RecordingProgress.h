#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <string>

enum class RecordingState
{
    Preparing,
    Armed,
    Recording,
    Paused,
    Completed,
    Cancelled,
    Failed
};

struct RecordingProgress
{
    RecordingState state =
        RecordingState::Preparing;

    std::size_t eventCount = 0;

    std::uint64_t elapsedMicroseconds = 0;

    std::string message;
};

using RecordingProgressCallback =
    std::function<
        void(const RecordingProgress&)>;

struct RecordingCallbacks
{
    RecordingProgressCallback onProgress;
};