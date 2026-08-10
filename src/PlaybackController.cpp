#include "PlaybackController.h"

void PlaybackController::requestPause()
{
    paused_.store(
        true,
        std::memory_order_release);
}

void PlaybackController::requestResume()
{
    paused_.store(
        false,
        std::memory_order_release);
}

void PlaybackController::togglePause()
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
        // If another thread changed the value, expected is
        // automatically updated and the operation retries.
    }
}

void PlaybackController::requestCancel()
{
    cancellationRequested_.store(
        true,
        std::memory_order_release);
}

bool PlaybackController::paused() const
{
    return paused_.load(
        std::memory_order_acquire);
}

bool PlaybackController::cancellationRequested() const
{
    return cancellationRequested_.load(
        std::memory_order_acquire);
}

void PlaybackController::reset()
{
    paused_.store(
        false,
        std::memory_order_release);

    cancellationRequested_.store(
        false,
        std::memory_order_release);
}