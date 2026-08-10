#pragma once

#include <atomic>

class PlaybackController
{
public:
    void requestPause();
    void requestResume();
    void togglePause();
    void requestCancel();

    [[nodiscard]] bool paused() const;
    [[nodiscard]] bool cancellationRequested() const;

    void reset();

private:
    std::atomic_bool paused_{false};
    std::atomic_bool cancellationRequested_{false};
};