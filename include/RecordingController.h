#pragma once

#include <atomic>

class RecordingController
{
public:
    void requestStart();

    void requestPause();
    void requestResume();
    void togglePause();

    void requestStop();
    void requestCancel();

    [[nodiscard]] bool startRequested() const;
    [[nodiscard]] bool paused() const;
    [[nodiscard]] bool stopRequested() const;
    [[nodiscard]] bool cancellationRequested() const;

    void reset();

private:
    std::atomic_bool startRequested_{false};
    std::atomic_bool paused_{false};
    std::atomic_bool stopRequested_{false};
    std::atomic_bool cancellationRequested_{false};
};