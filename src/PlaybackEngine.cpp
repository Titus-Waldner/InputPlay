#include "PlaybackEngine.h"

#include "CancellationSession.h"
#include "DisplayMetadata.h"
#include "ExitCodes.h"
#include "Recording.h"
#include "RecordingFile.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <chrono>
#include <cstdint>
#include <iostream>
#include <string>

namespace
{
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
    const CancellationSession* cancellationSession)
{
    std::cout
        << "Playback armed\n"
        << "Press "
        << keyNameFromVirtualKey(settings.playStartKey)
        << " to start or "
        << keyNameFromVirtualKey(settings.playCancelKey)
        << " to cancel\n";

    bool startWasDown =
        isKeyPressed(settings.playStartKey);

    bool cancelWasDown =
        isKeyPressed(settings.playCancelKey);

    while (true)
    {
        if (cancellationSession != nullptr
            && cancellationSession->isCancellationRequested())
        {
            return false;
        }

        const bool startIsDown =
            isKeyPressed(settings.playStartKey);

        const bool cancelIsDown =
            isKeyPressed(settings.playCancelKey);

        if (cancelIsDown && !cancelWasDown)
        {
            waitForKeyRelease(settings.playCancelKey);
            return false;
        }

        if (startIsDown && !startWasDown)
        {
            waitForKeyRelease(settings.playStartKey);
            return true;
        }

        startWasDown = startIsDown;
        cancelWasDown = cancelIsDown;

        Sleep(1);
    }
}

bool checkDisplayCompatibility(
    const Recording& recording,
    bool strictDisplay,
    bool ignoreDisplay,
    std::string& errorMessage)
{
    if (ignoreDisplay)
    {
        return true;
    }

    if (!recording.hasDisplayMetadata())
    {
        std::cout
            << "Display compatibility: Unknown\n"
            << "The recording does not contain display metadata.\n";

        if (strictDisplay)
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
            std::cout
                << "Display compatibility: Exact\n";

            return true;

        case DisplayCompatibility::CompatibleWithWarnings:
            std::cout
                << "Display compatibility warning: "
                << compatibilityMessage
                << '\n';

            if (strictDisplay)
            {
                errorMessage =
                    "Strict display validation rejected "
                    "the current display configuration.";

                return false;
            }

            return true;

        case DisplayCompatibility::Incompatible:
            std::cout
                << "Display compatibility warning: "
                << compatibilityMessage
                << '\n';

            if (strictDisplay)
            {
                errorMessage =
                    "The current display configuration is "
                    "incompatible with the recording.";

                return false;
            }

            return true;

        case DisplayCompatibility::Unknown:
            std::cout
                << "Display compatibility: Unknown\n"
                << compatibilityMessage
                << '\n';

            if (strictDisplay)
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
}

int runPlayback(
    const std::string& filePath,
    IInputBackend& backend,
    const Settings& settings,
    const PlaybackOptions& options)
{
    Recording recording;
    std::string errorMessage;

    CancellationSession cancellationSession;

    if (!options.sessionName.empty())
    {
        if (!cancellationSession.create(
                options.sessionName,
                errorMessage))
        {
            std::cerr
                << "Unable to create playback session: "
                << errorMessage
                << '\n';

            return ExitCode::GeneralFailure;
        }

        std::cout
            << "Playback session: "
            << options.sessionName
            << '\n';
    }

    if (!RecordingFile::load(
            filePath,
            recording,
            errorMessage))
    {
        std::cerr
            << "Unable to load recording: "
            << errorMessage
            << '\n';

        return ExitCode::RecordingLoadFailure;
    }

    if (recording.empty())
    {
        std::cout
            << "Recording contains no events\n";

        return ExitCode::Success;
    }

    if (!checkDisplayCompatibility(
            recording,
            options.strictDisplay,
            options.ignoreDisplay,
            errorMessage))
    {
        std::cerr
            << "Display validation failed: "
            << errorMessage
            << '\n';

        return ExitCode::DisplayIncompatible;
    }

    if (options.alignStart
        && !recording.hasStartingCursorPosition())
    {
        std::cerr
            << "Recording does not contain a "
            << "starting cursor position\n";

        return ExitCode::GeneralFailure;
    }

    const CancellationSession* sessionPointer =
        options.sessionName.empty()
        ? nullptr
        : &cancellationSession;

    if (!options.startImmediately)
    {
        if (!waitForPlaybackStart(
                settings,
                sessionPointer))
        {
            backend.releaseAll();

            std::cout
                << "Playback cancelled before start\n";

            return ExitCode::Cancelled;
        }
    }
    else
    {
        std::cout
            << "Playback starting immediately\n";
    }

    std::cout
        << "Playback controls: "
        << keyNameFromVirtualKey(settings.playPauseKey)
        << " pauses/resumes, "
        << keyNameFromVirtualKey(settings.playCancelKey)
        << " cancels\n";

    if (options.infiniteLoops)
    {
        std::cout << "Loops: infinite\n";
    }
    else
    {
        std::cout
            << "Loops: "
            << options.loopCount
            << '\n';
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

        std::cout
            << "Timeout: "
            << options.timeoutSeconds
            << " seconds\n";
    }

    unsigned int completedLoops = 0;
    bool cancelled = false;
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

        if (sessionPointer != nullptr
            && sessionPointer->isCancellationRequested())
        {
            cancelled = true;
            break;
        }

        if (options.alignStart)
        {
            if (!SetCursorPos(
                    recording.startingCursorX(),
                    recording.startingCursorY()))
            {
                backend.releaseAll();

                std::cerr
                    << "Windows was unable to align the cursor\n";

                return ExitCode::GeneralFailure;
            }

            std::cout
                << "Cursor aligned to "
                << recording.startingCursorX()
                << ", "
                << recording.startingCursorY()
                << '\n';
        }

        std::cout
            << "Starting loop "
            << completedLoops + 1
            << '\n';

        const Clock::time_point playbackStart =
            Clock::now();

        std::chrono::microseconds pausedDuration{0};
        std::uint64_t addedWaitMicroseconds = 0;

        bool paused = false;

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

                if (sessionPointer != nullptr
                    && sessionPointer
                        ->isCancellationRequested())
                {
                    cancelled = true;
                    break;
                }

                const bool cancelKeyIsDown =
                    isKeyPressed(
                        settings.playCancelKey);

                if (cancelKeyIsDown
                    && !cancelKeyWasDown)
                {
                    cancelled = true;
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
                    paused = !paused;

                    if (paused)
                    {
                        std::cout
                            << "Playback paused\n";
                    }
                    else
                    {
                        std::cout
                            << "Playback resumed\n";
                    }
                }

                pauseKeyWasDown =
                    pauseKeyIsDown;

                if (paused)
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

            if (cancelled || timedOut)
            {
                break;
            }

            if (event.type == EventType::Wait)
            {
                addedWaitMicroseconds +=
                    event.waitMicroseconds;

                continue;
            }

            if (!backend.execute(
                    event,
                    errorMessage))
            {
                backend.releaseAll();

                std::cerr
                    << "Playback failed: "
                    << errorMessage
                    << '\n';

                return ExitCode::GeneralFailure;
            }
        }

        backend.releaseAll();

        if (cancelled || timedOut)
        {
            break;
        }

        ++completedLoops;

        std::cout
            << "Completed loop "
            << completedLoops
            << '\n';
    }

    backend.releaseAll();

    if (timedOut)
    {
        std::cout
            << "Playback timed out\n";

        return ExitCode::Timeout;
    }

    if (cancelled)
    {
        std::cout
            << "Playback cancelled\n";

        return ExitCode::Cancelled;
    }

    std::cout
        << "Playback completed\n";

    return ExitCode::Success;
}