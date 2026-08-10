#include "PlaybackEngine.h"

#include "CancellationSession.h"
#include "DisplayMetadata.h"
#include "Recording.h"
#include "RecordingFile.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <sstream>
#include <string>

namespace
{
void reportPlaybackProgress(
    const PlaybackCallbacks& callbacks,
    PlaybackState state,
    unsigned int currentLoop,
    unsigned int totalLoops,
    bool infiniteLoops,
    std::size_t completedEvents,
    std::size_t totalEvents,
    const std::string& message)
{
    if (!callbacks.onProgress)
    {
        return;
    }

    PlaybackProgress progress;

    progress.state = state;
    progress.currentLoop = currentLoop;
    progress.totalLoops = totalLoops;
    progress.infiniteLoops = infiniteLoops;
    progress.completedEvents = completedEvents;
    progress.totalEvents = totalEvents;
    progress.message = message;

    callbacks.onProgress(progress);
}

void reportMessage(
    const PlaybackCallbacks& callbacks,
    unsigned int currentLoop,
    const PlaybackOptions& options,
    std::size_t completedEvents,
    std::size_t totalEvents,
    const std::string& message)
{
    reportPlaybackProgress(
        callbacks,
        PlaybackState::Message,
        currentLoop,
        options.loopCount,
        options.infiniteLoops,
        completedEvents,
        totalEvents,
        message);
}

void reportFailure(
    const PlaybackCallbacks& callbacks,
    unsigned int currentLoop,
    const PlaybackOptions& options,
    std::size_t completedEvents,
    std::size_t totalEvents,
    const std::string& message)
{
    reportPlaybackProgress(
        callbacks,
        PlaybackState::Failed,
        currentLoop,
        options.loopCount,
        options.infiniteLoops,
        completedEvents,
        totalEvents,
        message);
}

bool isKeyPressed(int virtualKey)
{
    return
        (GetAsyncKeyState(virtualKey) & 0x8000) != 0;
}

void waitForKeyRelease(int virtualKey)
{
    while (isKeyPressed(virtualKey))
    {
        Sleep(1);
    }
}

bool waitForPlaybackStart(
    const Settings& settings,
    const CancellationSession* cancellationSession,
    PlaybackController& controller,
    const PlaybackCallbacks& callbacks,
    const PlaybackOptions& options,
    std::size_t totalEvents)
{
    reportPlaybackProgress(
        callbacks,
        PlaybackState::Armed,
        0,
        options.loopCount,
        options.infiniteLoops,
        0,
        totalEvents,
        "Playback armed.");

    std::ostringstream instructions;

    instructions
        << "Press "
        << keyNameFromVirtualKey(settings.playStartKey)
        << " to start or "
        << keyNameFromVirtualKey(settings.playCancelKey)
        << " to cancel";

    reportMessage(
        callbacks,
        0,
        options,
        0,
        totalEvents,
        instructions.str());

    bool startWasDown =
        isKeyPressed(settings.playStartKey);

    bool cancelWasDown =
        isKeyPressed(settings.playCancelKey);

    while (true)
    {
        if (controller.cancellationRequested())
        {
            return false;
        }

        if (cancellationSession != nullptr
            && cancellationSession
                ->isCancellationRequested())
        {
            controller.requestCancel();
            return false;
        }

        const bool startIsDown =
            isKeyPressed(settings.playStartKey);

        const bool cancelIsDown =
            isKeyPressed(settings.playCancelKey);

        if (cancelIsDown && !cancelWasDown)
        {
            controller.requestCancel();

            waitForKeyRelease(
                settings.playCancelKey);

            return false;
        }

        if (startIsDown && !startWasDown)
        {
            waitForKeyRelease(
                settings.playStartKey);

            return true;
        }

        startWasDown = startIsDown;
        cancelWasDown = cancelIsDown;

        Sleep(1);
    }
}

bool checkDisplayCompatibility(
    const Recording& recording,
    const PlaybackOptions& options,
    const PlaybackCallbacks& callbacks,
    std::string& errorMessage)
{
    if (options.ignoreDisplay)
    {
        return true;
    }

    if (!recording.hasDisplayMetadata())
    {
        reportMessage(
            callbacks,
            0,
            options,
            0,
            recording.eventCount(),
            "Display compatibility: Unknown");

        reportMessage(
            callbacks,
            0,
            options,
            0,
            recording.eventCount(),
            "The recording does not contain display metadata.");

        if (options.strictDisplay)
        {
            errorMessage =
                "Strict display validation requires display metadata.";

            return false;
        }

        return true;
    }

    DisplayMetadata currentDisplay;
    std::string captureError;

    if (!captureDisplayMetadata(
            currentDisplay,
            captureError))
    {
        errorMessage =
            "Unable to inspect the current display configuration: "
            + captureError;

        return false;
    }

    std::string compatibilityMessage;

    const DisplayCompatibility compatibility =
        compareDisplayMetadata(
            recording.displayMetadata(),
            currentDisplay,
            compatibilityMessage);

    switch (compatibility)
    {
        case DisplayCompatibility::Exact:
            reportMessage(
                callbacks,
                0,
                options,
                0,
                recording.eventCount(),
                "Display compatibility: Exact");

            return true;

        case DisplayCompatibility::CompatibleWithWarnings:
            reportMessage(
                callbacks,
                0,
                options,
                0,
                recording.eventCount(),
                "Display compatibility warning: "
                    + compatibilityMessage);

            if (options.strictDisplay)
            {
                errorMessage =
                    "Strict display validation rejected "
                    "the current display configuration.";

                return false;
            }

            return true;

        case DisplayCompatibility::Incompatible:
            reportMessage(
                callbacks,
                0,
                options,
                0,
                recording.eventCount(),
                "Display compatibility warning: "
                    + compatibilityMessage);

            if (options.strictDisplay)
            {
                errorMessage =
                    "The current display configuration is "
                    "incompatible with the recording.";

                return false;
            }

            return true;

        case DisplayCompatibility::Unknown:
            reportMessage(
                callbacks,
                0,
                options,
                0,
                recording.eventCount(),
                "Display compatibility: Unknown");

            if (!compatibilityMessage.empty())
            {
                reportMessage(
                    callbacks,
                    0,
                    options,
                    0,
                    recording.eventCount(),
                    compatibilityMessage);
            }

            if (options.strictDisplay)
            {
                errorMessage =
                    "Strict display validation could not "
                    "determine compatibility.";

                return false;
            }

            return true;
    }

    errorMessage =
        "An unknown display compatibility result occurred.";

    return false;
}

void processExternalSessionControls(
    CancellationSession* cancellationSession,
    PlaybackController& controller,
    const PlaybackCallbacks& callbacks,
    unsigned int currentLoop,
    const PlaybackOptions& options,
    std::size_t completedEvents,
    std::size_t totalEvents)
{
    if (cancellationSession == nullptr)
    {
        return;
    }

    if (cancellationSession
        ->isCancellationRequested())
    {
        controller.requestCancel();
        return;
    }

    if (cancellationSession
        ->consumePauseRequest())
    {
        if (!controller.paused())
        {
            controller.requestPause();

            reportPlaybackProgress(
                callbacks,
                PlaybackState::Paused,
                currentLoop,
                options.loopCount,
                options.infiniteLoops,
                completedEvents,
                totalEvents,
                "Playback paused.");
        }
    }

    if (cancellationSession
        ->consumeResumeRequest())
    {
        if (controller.paused())
        {
            controller.requestResume();

            reportPlaybackProgress(
                callbacks,
                PlaybackState::Playing,
                currentLoop,
                options.loopCount,
                options.infiniteLoops,
                completedEvents,
                totalEvents,
                "Playback resumed.");
        }
    }
}

PlaybackResult makePlaybackResult(
    PlaybackResultCode code,
    const std::string& message,
    unsigned int completedLoops,
    std::size_t completedEvents)
{
    PlaybackResult result;

    result.code = code;
    result.message = message;
    result.completedLoops = completedLoops;
    result.completedEvents = completedEvents;

    return result;
}
}

PlaybackResult runPlayback(
    const std::string& filePath,
    IInputBackend& backend,
    const Settings& settings,
    const PlaybackOptions& options,
    PlaybackController& controller,
    const PlaybackCallbacks& callbacks)
{
    Recording recording;
    std::string errorMessage;

    reportPlaybackProgress(
        callbacks,
        PlaybackState::Preparing,
        0,
        options.loopCount,
        options.infiniteLoops,
        0,
        0,
        "Preparing playback.");

    CancellationSession cancellationSession;

    if (!options.sessionName.empty())
    {
        if (!cancellationSession.create(
                options.sessionName,
                errorMessage))
        {
            reportFailure(
                callbacks,
                0,
                options,
                0,
                0,
                errorMessage);

            return makePlaybackResult(
                PlaybackResultCode::InternalError,
                errorMessage,
                0,
                0);
        }

        reportMessage(
            callbacks,
            0,
            options,
            0,
            0,
            "Playback session: "
                + options.sessionName);
    }

    if (!RecordingFile::load(
            filePath,
            recording,
            errorMessage))
    {
        reportFailure(
            callbacks,
            0,
            options,
            0,
            0,
            errorMessage);

        return makePlaybackResult(
            PlaybackResultCode::RecordingLoadFailed,
            errorMessage,
            0,
            0);
    }

    if (recording.empty())
    {
        reportMessage(
            callbacks,
            0,
            options,
            0,
            0,
            "Recording contains no events");

        reportPlaybackProgress(
            callbacks,
            PlaybackState::Completed,
            0,
            options.loopCount,
            options.infiniteLoops,
            0,
            0,
            "Playback completed.");

        return makePlaybackResult(
            PlaybackResultCode::Completed,
            "Recording contains no events.",
            0,
            0);
    }

    if (!checkDisplayCompatibility(
            recording,
            options,
            callbacks,
            errorMessage))
    {
        reportFailure(
            callbacks,
            0,
            options,
            0,
            recording.eventCount(),
            errorMessage);

        return makePlaybackResult(
            PlaybackResultCode::DisplayIncompatible,
            errorMessage,
            0,
            0);
    }

    if (options.alignStart
        && !recording.hasStartingCursorPosition())
    {
        const std::string message =
            "Recording does not contain a starting cursor position.";

        reportFailure(
            callbacks,
            0,
            options,
            0,
            recording.eventCount(),
            message);

        return makePlaybackResult(
            PlaybackResultCode::InternalError,
            message,
            0,
            0);
    }

    const CancellationSession* sessionPointer =
        options.sessionName.empty()
        ? nullptr
        : &cancellationSession;

    if (!options.startImmediately)
    {
        if (!waitForPlaybackStart(
                settings,
                sessionPointer,
                controller,
                callbacks,
                options,
                recording.eventCount()))
        {
            backend.releaseAll();

            reportPlaybackProgress(
                callbacks,
                PlaybackState::Cancelled,
                0,
                options.loopCount,
                options.infiniteLoops,
                0,
                recording.eventCount(),
                "Playback cancelled before start.");

            return makePlaybackResult(
                PlaybackResultCode::Cancelled,
                "Playback cancelled before start.",
                0,
                0);
        }
    }
    else
    {
        reportMessage(
            callbacks,
            0,
            options,
            0,
            recording.eventCount(),
            "Playback starting immediately");
    }

    {
        std::ostringstream controlsMessage;

        controlsMessage
            << "Playback controls: "
            << keyNameFromVirtualKey(settings.playPauseKey)
            << " pauses/resumes, "
            << keyNameFromVirtualKey(settings.playCancelKey)
            << " cancels";

        reportMessage(
            callbacks,
            0,
            options,
            0,
            recording.eventCount(),
            controlsMessage.str());
    }

    if (options.infiniteLoops)
    {
        reportMessage(
            callbacks,
            0,
            options,
            0,
            recording.eventCount(),
            "Loops: infinite");
    }
    else
    {
        std::ostringstream loopsMessage;

        loopsMessage
            << "Loops: "
            << options.loopCount;

        reportMessage(
            callbacks,
            0,
            options,
            0,
            recording.eventCount(),
            loopsMessage.str());
    }

    using Clock = std::chrono::steady_clock;

    const Clock::time_point operationStart =
        Clock::now();

    Clock::time_point timeoutDeadline =
        Clock::time_point::max();

    if (options.timeoutEnabled)
    {
        timeoutDeadline =
            operationStart
            + std::chrono::seconds(
                options.timeoutSeconds);

        std::ostringstream timeoutMessage;

        timeoutMessage
            << "Timeout: "
            << options.timeoutSeconds
            << " seconds";

        reportMessage(
            callbacks,
            0,
            options,
            0,
            recording.eventCount(),
            timeoutMessage.str());
    }

    unsigned int completedLoops = 0;
    std::size_t completedEvents = 0;

    bool timedOut = false;

    while (options.infiniteLoops
           || completedLoops < options.loopCount)
    {
        if (options.timeoutEnabled
            && Clock::now() >= timeoutDeadline)
        {
            timedOut = true;
            break;
        }

        if (controller.cancellationRequested())
        {
            break;
        }

        processExternalSessionControls(
            &cancellationSession,
            controller,
            callbacks,
            completedLoops + 1,
            options,
            completedEvents,
            recording.eventCount());

        if (controller.cancellationRequested())
        {
            break;
        }

        if (options.alignStart)
        {
            if (!SetCursorPos(
                    recording.startingCursorX(),
                    recording.startingCursorY()))
            {
                backend.releaseAll();

                const std::string message =
                    "Windows was unable to align the cursor.";

                reportFailure(
                    callbacks,
                    completedLoops + 1,
                    options,
                    completedEvents,
                    recording.eventCount(),
                    message);

                return makePlaybackResult(
                    PlaybackResultCode::InternalError,
                    message,
                    completedLoops,
                    completedEvents);
            }

            std::ostringstream alignmentMessage;

            alignmentMessage
                << "Cursor aligned to "
                << recording.startingCursorX()
                << ", "
                << recording.startingCursorY();

            reportMessage(
                callbacks,
                completedLoops + 1,
                options,
                completedEvents,
                recording.eventCount(),
                alignmentMessage.str());
        }

        reportPlaybackProgress(
            callbacks,
            PlaybackState::Playing,
            completedLoops + 1,
            options.loopCount,
            options.infiniteLoops,
            completedEvents,
            recording.eventCount(),
            "Playback loop started.");

        const Clock::time_point playbackStart =
            Clock::now();

        std::chrono::microseconds pausedDuration{0};
        std::uint64_t addedWaitMicroseconds = 0;

        std::size_t completedEventsInLoop = 0;

        bool pauseKeyWasDown =
            isKeyPressed(settings.playPauseKey);

        bool cancelKeyWasDown =
            isKeyPressed(settings.playCancelKey);

        for (const InputEvent& event
             : recording.events())
        {
            const auto eventOffset =
                std::chrono::microseconds(
                    event.timestampMicroseconds
                    + addedWaitMicroseconds);

            while (true)
            {
                if (options.timeoutEnabled
                    && Clock::now() >= timeoutDeadline)
                {
                    timedOut = true;
                    break;
                }

                if (controller.cancellationRequested())
                {
                    break;
                }

                processExternalSessionControls(
                    &cancellationSession,
                    controller,
                    callbacks,
                    completedLoops + 1,
                    options,
                    completedEvents,
                    recording.eventCount());

                if (controller.cancellationRequested())
                {
                    break;
                }

                const bool cancelKeyIsDown =
                    isKeyPressed(
                        settings.playCancelKey);

                if (cancelKeyIsDown
                    && !cancelKeyWasDown)
                {
                    controller.requestCancel();
                    break;
                }

                cancelKeyWasDown =
                    cancelKeyIsDown;

                const bool pauseKeyIsDown =
                    isKeyPressed(
                        settings.playPauseKey);

                if (pauseKeyIsDown
                    && !pauseKeyWasDown)
                {
                    controller.togglePause();

                    if (controller.paused())
                    {
                        reportPlaybackProgress(
                            callbacks,
                            PlaybackState::Paused,
                            completedLoops + 1,
                            options.loopCount,
                            options.infiniteLoops,
                            completedEvents,
                            recording.eventCount(),
                            "Playback paused.");
                    }
                    else
                    {
                        reportPlaybackProgress(
                            callbacks,
                            PlaybackState::Playing,
                            completedLoops + 1,
                            options.loopCount,
                            options.infiniteLoops,
                            completedEvents,
                            recording.eventCount(),
                            "Playback resumed.");
                    }
                }

                pauseKeyWasDown =
                    pauseKeyIsDown;

                if (controller.paused())
                {
                    const Clock::time_point pauseTick =
                        Clock::now();

                    Sleep(1);

                    pausedDuration +=
                        std::chrono::duration_cast<
                            std::chrono::microseconds>(
                            Clock::now() - pauseTick);

                    continue;
                }

                const Clock::time_point deadline =
                    playbackStart
                    + eventOffset
                    + pausedDuration;

                if (Clock::now() >= deadline)
                {
                    break;
                }

                Sleep(1);
            }

            if (controller.cancellationRequested()
                || timedOut)
            {
                break;
            }

            if (event.type == EventType::Wait)
            {
                addedWaitMicroseconds +=
                    event.waitMicroseconds;

                ++completedEvents;
                ++completedEventsInLoop;

                if (completedEventsInLoop % 250 == 0
                    || completedEventsInLoop
                        == recording.eventCount())
                {
                    reportPlaybackProgress(
                        callbacks,
                        PlaybackState::Progress,
                        completedLoops + 1,
                        options.loopCount,
                        options.infiniteLoops,
                        completedEventsInLoop,
                        recording.eventCount(),
                        "Playback progress.");
                }

                continue;
            }

            if (!backend.execute(
                    event,
                    errorMessage))
            {
                backend.releaseAll();

                reportFailure(
                    callbacks,
                    completedLoops + 1,
                    options,
                    completedEvents,
                    recording.eventCount(),
                    errorMessage);

                return makePlaybackResult(
                    PlaybackResultCode::BackendFailed,
                    errorMessage,
                    completedLoops,
                    completedEvents);
            }

            ++completedEvents;
            ++completedEventsInLoop;

            if (completedEventsInLoop % 250 == 0
                || completedEventsInLoop
                    == recording.eventCount())
            {
                reportPlaybackProgress(
                    callbacks,
                    PlaybackState::Progress,
                    completedLoops + 1,
                    options.loopCount,
                    options.infiniteLoops,
                    completedEventsInLoop,
                    recording.eventCount(),
                    "Playback progress.");
            }
        }

        backend.releaseAll();

        if (controller.cancellationRequested()
            || timedOut)
        {
            break;
        }

        ++completedLoops;

        reportPlaybackProgress(
            callbacks,
            PlaybackState::LoopCompleted,
            completedLoops,
            options.loopCount,
            options.infiniteLoops,
            recording.eventCount(),
            recording.eventCount(),
            "Playback loop completed.");
    }

    backend.releaseAll();

    if (timedOut)
    {
        reportPlaybackProgress(
            callbacks,
            PlaybackState::TimedOut,
            completedLoops + 1,
            options.loopCount,
            options.infiniteLoops,
            completedEvents,
            recording.eventCount(),
            "Playback timed out.");

        return makePlaybackResult(
            PlaybackResultCode::TimedOut,
            "Playback timed out.",
            completedLoops,
            completedEvents);
    }

    if (controller.cancellationRequested())
    {
        reportPlaybackProgress(
            callbacks,
            PlaybackState::Cancelled,
            completedLoops + 1,
            options.loopCount,
            options.infiniteLoops,
            completedEvents,
            recording.eventCount(),
            "Playback cancelled.");

        return makePlaybackResult(
            PlaybackResultCode::Cancelled,
            "Playback cancelled.",
            completedLoops,
            completedEvents);
    }

    reportPlaybackProgress(
        callbacks,
        PlaybackState::Completed,
        completedLoops,
        options.loopCount,
        options.infiniteLoops,
        completedEvents,
        recording.eventCount(),
        "Playback completed.");

    return makePlaybackResult(
        PlaybackResultCode::Completed,
        "Playback completed.",
        completedLoops,
        completedEvents);
}