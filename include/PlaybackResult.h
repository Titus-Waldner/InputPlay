#pragma once

#include <cstddef>
#include <string>

enum class PlaybackResultCode
{
    Completed,
    Cancelled,
    TimedOut,
    RecordingLoadFailed,
    DisplayIncompatible,
    BackendFailed,
    InternalError
};

struct PlaybackResult
{
    PlaybackResultCode code =
        PlaybackResultCode::InternalError;

    std::string message;

    unsigned int completedLoops = 0;
    std::size_t completedEvents = 0;

    [[nodiscard]] bool succeeded() const
    {
        return code == PlaybackResultCode::Completed;
    }
};