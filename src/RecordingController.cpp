#include "RecordingController.h"

void RecordingController::requestStart()
{
    startRequested_.store(
        true,
        std::memory_order_release);
}

void RecordingController::requestPause()
{
    paused_.store(
        true,
        std::memory_order_release);
}

void RecordingController::requestResume()
{
    paused_.store(
        false,
        std::memory_order_release);
}

void RecordingController::togglePause()
{
    bool expected =
        paused_.load(
            std::memory_order_acquire);

    while (!paused_.compare_exchange_weak(
        expected,
        !expected,
        std::memory_order_acq_rel,
        std::memory_order_acquire))
    {
        // Retry if another thread changed the value.
    }
}

void RecordingController::requestStop()
{
    stopRequested_.store(
        true,
        std::memory_order_release);
}

void RecordingController::requestCancel()
{
    cancellationRequested_.store(
        true,
        std::memory_order_release);
}

bool RecordingController::startRequested() const
{
    return startRequested_.load(
        std::memory_order_acquire);
}

bool RecordingController::paused() const
{
    return paused_.load(
        std::memory_order_acquire);
}

bool RecordingController::stopRequested() const
{
    return stopRequested_.load(
        std::memory_order_acquire);
}

bool RecordingController::cancellationRequested() const
{
    return cancellationRequested_.load(
        std::memory_order_acquire);
}

void RecordingController::reset()
{
    startRequested_.store(
        false,
        std::memory_order_release);

    paused_.store(
        false,
        std::memory_order_release);

    stopRequested_.store(
        false,
        std::memory_order_release);

    cancellationRequested_.store(
        false,
        std::memory_order_release);
}