#pragma once

#include <cstddef>
#include <functional>
#include <string>

enum class PlaybackState
{
    Preparing,
    Armed,
    Playing,
    Paused,
    Progress,
    LoopCompleted,
    Message,
    Completed,
    Cancelled,
    TimedOut,
    Failed
};

struct PlaybackProgress
{
    PlaybackState state =
        PlaybackState::Preparing;

    unsigned int currentLoop = 0;
    unsigned int totalLoops = 0;

    bool infiniteLoops = false;

    std::size_t completedEvents = 0;
    std::size_t totalEvents = 0;

    std::string message;
};

using PlaybackProgressCallback =
    std::function<
        void(const PlaybackProgress&)>;

struct PlaybackCallbacks
{
    PlaybackProgressCallback onProgress;
};