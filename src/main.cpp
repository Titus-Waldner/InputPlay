#include "Recording.h"
#include "RecordingFile.h"
#include "DryRunBackend.h"
#include "SendInputBackend.h"
#include "MouseRecorder.h"
#include "Settings.h"
#include "RecordingValidator.h"

#include <iomanip>
#include <chrono>
#include <iostream>
#include <string>
#include <thread>
#include <stdexcept>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace
{
void printHelp()
{
    std::cerr
			  << "Usage: InputPlay play <file> "
			  << "[--send-input] [--align-start] "
			  << "[--loops <number|inf>] "
			  << "[--strict-display|--ignore-display]\n";
    std::cout << "Commands:\n";
    std::cout << "  record <file>\n";
    std::cout
			  << "  play <file> [--send-input] [--align-start] "
			  << "[--loops <number|inf>] "
			  << "[--strict-display|--ignore-display]\n";
    std::cout << "  info <file>\n";
	std::cout << "  validate <file>\n";
	std::cout << "  test-model\n";
}

const char* eventTypeName(EventType type)
{
    switch (type)
    {
        case EventType::MouseMove:
            return "MouseMove";

        case EventType::MouseButtonDown:
            return "MouseButtonDown";

        case EventType::MouseButtonUp:
            return "MouseButtonUp";

        case EventType::MouseWheel:
            return "MouseWheel";

        case EventType::KeyDown:
            return "KeyDown";

        case EventType::KeyUp:
            return "KeyUp";
			
		case EventType::Wait:
			return "Wait";
			
		case EventType::MouseTeleport:
			return "MouseTeleport";

        default:
            return "Unknown";
    }
}

Recording createSampleRecording()
{
    Recording recording;

    InputEvent firstMove;
    firstMove.timestampMicroseconds = 0;
    firstMove.type = EventType::MouseMove;
    firstMove.mouseX = 500;
    firstMove.mouseY = 300;
    firstMove.mouseDeltaX = 5;
    firstMove.mouseDeltaY = 2;
    recording.addEvent(firstMove);

    InputEvent secondMove;
    secondMove.timestampMicroseconds = 10'000;
    secondMove.type = EventType::MouseMove;
    secondMove.mouseX = 503;
    secondMove.mouseY = 299;
    secondMove.mouseDeltaX = 3;
    secondMove.mouseDeltaY = -1;
    recording.addEvent(secondMove);

    InputEvent leftButtonDown;
    leftButtonDown.timestampMicroseconds = 20'000;
    leftButtonDown.type = EventType::MouseButtonDown;
    leftButtonDown.mouseX = 503;
    leftButtonDown.mouseY = 299;
    leftButtonDown.mouseButton = 1;
    recording.addEvent(leftButtonDown);

	InputEvent leftButtonUp;
	leftButtonUp.timestampMicroseconds = 70'000;
	leftButtonUp.type = EventType::MouseButtonUp;
	leftButtonUp.mouseX = 503;
	leftButtonUp.mouseY = 299;
	leftButtonUp.mouseButton = 1;
	recording.addEvent(leftButtonUp);

    return recording;
}

int runRecordCommand(
    const std::string& filePath,
    const Settings& settings)
{
    Recording recording;
    MouseRecorder recorder;
    std::string errorMessage;

    if (!recorder.record(
        recording,
        settings,
        errorMessage))
    {
        std::cerr << "Recording failed: "
                  << errorMessage
                  << '\n';

        return 1;
    }

    if (!RecordingFile::save(
            recording,
            filePath,
            errorMessage))
    {
        std::cerr << "Unable to save recording: "
                  << errorMessage
                  << '\n';

        return 1;
    }

    std::cout << "Recording saved\n";
    std::cout << "File: " << filePath << '\n';
    std::cout << "Events: "
              << recording.eventCount()
              << '\n';

    return 0;
}
int runInfoCommand(const std::string& filePath)
{
    Recording recording;
    std::string errorMessage;

    if (!RecordingFile::load(
            filePath,
            recording,
            errorMessage))
    {
        std::cerr
            << "Unable to load recording: "
            << errorMessage
            << '\n';

        return 3;
    }

    std::size_t mouseMoveCount = 0;
    std::size_t mouseTeleportCount = 0;
    std::size_t mouseButtonCount = 0;
    std::size_t mouseWheelCount = 0;
    std::size_t keyboardCount = 0;
    std::size_t waitCount = 0;

    std::uint64_t durationMicroseconds = 0;
    std::uint64_t explicitWaitMicroseconds = 0;

    for (const InputEvent& event : recording.events())
    {
        if (event.timestampMicroseconds
            > durationMicroseconds)
        {
            durationMicroseconds =
                event.timestampMicroseconds;
        }

        switch (event.type)
        {
            case EventType::MouseMove:
                ++mouseMoveCount;
                break;

            case EventType::MouseTeleport:
                ++mouseTeleportCount;
                break;

            case EventType::MouseButtonDown:
            case EventType::MouseButtonUp:
                ++mouseButtonCount;
                break;

            case EventType::MouseWheel:
                ++mouseWheelCount;
                break;

            case EventType::KeyDown:
            case EventType::KeyUp:
                ++keyboardCount;
                break;

            case EventType::Wait:
                ++waitCount;

                explicitWaitMicroseconds +=
                    event.waitMicroseconds;

                break;
        }
    }

    const double durationSeconds =
        static_cast<double>(
            durationMicroseconds
            + explicitWaitMicroseconds)
        / 1'000'000.0;

    std::cout
        << "InputPlay recording information\n\n";

    std::cout
        << "File:                  "
        << filePath
        << '\n';

    std::cout
        << "Events:                "
        << recording.eventCount()
        << '\n';

    std::cout
        << "Duration:              "
        << std::fixed
        << std::setprecision(3)
        << durationSeconds
        << " seconds\n";

    if (recording.hasStartingCursorPosition())
    {
        std::cout
            << "Starting cursor:       "
            << recording.startingCursorX()
            << ", "
            << recording.startingCursorY()
            << '\n';
    }
    else
    {
        std::cout
            << "Starting cursor:       Unknown\n";
    }

    if (recording.hasDisplayMetadata())
    {
        const DisplayMetadata& display =
            recording.displayMetadata();

        std::cout
            << "Monitor count:         "
            << display.monitors.size()
            << '\n';

        std::cout
            << "Virtual desktop:       "
            << display.virtualDesktopLeft
            << ","
            << display.virtualDesktopTop
            << " "
            << display.virtualDesktopWidth
            << "x"
            << display.virtualDesktopHeight
            << '\n';

        for (std::size_t index = 0;
             index < display.monitors.size();
             ++index)
        {
            const MonitorMetadata& monitor =
                display.monitors[index];

            const int monitorWidth =
                monitor.right - monitor.left;

            const int monitorHeight =
                monitor.bottom - monitor.top;

            std::cout
                << "Monitor "
                << index + 1
                << ":             "
                << monitor.deviceName
                << " at "
                << monitor.left
                << ","
                << monitor.top
                << " "
                << monitorWidth
                << "x"
                << monitorHeight;

            if (monitor.primary)
            {
                std::cout << " (primary)";
            }

            std::cout << '\n';
        }
    }
    else
    {
        std::cout
            << "Display metadata:      Not available\n";
    }

    std::cout << '\n';

    std::cout
        << "Mouse movements:       "
        << mouseMoveCount
        << '\n';

    std::cout
        << "Mouse teleports:       "
        << mouseTeleportCount
        << '\n';

    std::cout
        << "Mouse button events:   "
        << mouseButtonCount
        << '\n';

    std::cout
        << "Mouse wheel events:    "
        << mouseWheelCount
        << '\n';

    std::cout
        << "Keyboard events:       "
        << keyboardCount
        << '\n';

    std::cout
        << "Wait events:           "
        << waitCount
        << '\n';

    if (!recording.hasDisplayMetadata())
    {
        std::cout
            << "\nDisplay compatibility: Unknown\n";

        return 0;
    }

    DisplayMetadata currentDisplay;

    if (!captureDisplayMetadata(
            currentDisplay,
            errorMessage))
    {
        std::cout
            << "\nDisplay compatibility: Unable to check\n";

        std::cout
            << "Reason: "
            << errorMessage
            << '\n';

        return 0;
    }

    std::string compatibilityMessage;

    const DisplayCompatibility compatibility =
        compareDisplayMetadata(
            recording.displayMetadata(),
            currentDisplay,
            compatibilityMessage);

    std::cout << "\nDisplay compatibility: ";

    switch (compatibility)
    {
        case DisplayCompatibility::Exact:
            std::cout << "Exact\n";
            break;

        case DisplayCompatibility::CompatibleWithWarnings:
            std::cout << "Compatible with warnings\n";
            break;

        case DisplayCompatibility::Incompatible:
            std::cout << "Incompatible\n";
            break;

        case DisplayCompatibility::Unknown:
            std::cout << "Unknown\n";
            break;
    }

    std::cout
        << "Compatibility details: "
        << compatibilityMessage
        << '\n';

    return 0;
}

int runValidateCommand(
    const std::string& filePath)
{
    Recording recording;
    std::string errorMessage;

    if (!RecordingFile::load(
            filePath,
            recording,
            errorMessage))
    {
        std::cerr
            << "Unable to load recording: "
            << errorMessage
            << '\n';

        return 3;
    }

    std::cout
        << "Validating recording\n\n";

    std::cout
        << "File:           "
        << filePath
        << '\n';

    std::cout
        << "Events checked: "
        << recording.eventCount()
        << '\n';

    const ValidationResult result =
        validateRecording(recording);

    std::cout
        << "Warnings:       "
        << result.warningCount()
        << '\n';

    std::cout
        << "Errors:         "
        << result.errorCount()
        << "\n\n";

    for (const std::string& warning
         : result.warnings)
    {
        std::cout
            << "WARNING: "
            << warning
            << '\n';
    }

    for (const std::string& error
         : result.errors)
    {
        std::cout
            << "ERROR: "
            << error
            << '\n';
    }

    if (!result.warnings.empty()
        || !result.errors.empty())
    {
        std::cout << '\n';
    }

    if (result.valid())
    {
        std::cout
            << "Result: Valid\n";

        return 0;
    }

    std::cout
        << "Result: Invalid\n";

    return 7;
}

int runModelTest()
{
    const Recording recording = createSampleRecording();

    std::cout << "Recording model test\n";
    std::cout << "Event count: "
              << recording.eventCount()
              << '\n';

    for (const InputEvent& event : recording.events())
    {
        std::cout << "Event timestamp: "
                  << event.timestampMicroseconds
                  << " microseconds\n";
    }

    if (recording.eventCount() != 4)
    {
        std::cerr << "Model test failed\n";
        return 1;
    }

    std::cout << "Model test passed\n";
    return 0;
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
    const Settings& settings)
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

int runPlayCommand(
    const std::string& filePath,
    IInputBackend& backend,
    bool alignStart,
    bool strictDisplay,
    bool ignoreDisplay,
    const Settings& settings,
    unsigned int loopCount,
    bool infiniteLoops)
{
    Recording recording;
    std::string errorMessage;

    if (!RecordingFile::load(
            filePath,
            recording,
            errorMessage))
    {
        std::cerr
            << "Unable to load recording: "
            << errorMessage
            << '\n';

        return 3;
    }

	if (recording.empty())
	{
		std::cout << "Recording contains no events\n";
		return 0;
	}
	if (!checkDisplayCompatibility(
        recording,
        strictDisplay,
        ignoreDisplay,
        errorMessage))
	{
		std::cerr
			<< "Display validation failed: "
			<< errorMessage
			<< '\n';

		return 6;
	}

	if (alignStart
		&& !recording.hasStartingCursorPosition())
	{
		std::cerr
			<< "Recording does not contain a "
			<< "starting cursor position\n";

		return 1;
	}

	if (!waitForPlaybackStart(settings))
	{
		backend.releaseAll();

		std::cout
			<< "Playback cancelled before start\n";

		return 4;
	}

	std::cout
		<< "Playback controls: "
        << keyNameFromVirtualKey(settings.playPauseKey)
        << " pauses/resumes, "
        << keyNameFromVirtualKey(settings.playCancelKey)
        << " cancels\n";

    if (infiniteLoops)
    {
        std::cout << "Loops: infinite\n";
    }
    else
    {
        std::cout << "Loops: " << loopCount << '\n';
    }

    using Clock = std::chrono::steady_clock;

    unsigned int completedLoops = 0;
    bool cancelled = false;

    while (infiniteLoops || completedLoops < loopCount)
	{
		if (alignStart)
		{
			if (!SetCursorPos(
					recording.startingCursorX(),
					recording.startingCursorY()))
			{
				backend.releaseAll();

				std::cerr
					<< "Windows was unable to align the cursor\n";

				return 1;
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

        for (const InputEvent& event : recording.events())
        {
            const auto eventOffset =
                std::chrono::microseconds(
                    event.timestampMicroseconds
                    + addedWaitMicroseconds);

            while (true)
            {
    
				const bool cancelKeyIsDown =
				isKeyPressed(settings.playCancelKey);

				if (cancelKeyIsDown && !cancelKeyWasDown)
				{
					cancelled = true;
					break;
				}

				cancelKeyWasDown = cancelKeyIsDown;

				const bool pauseKeyIsDown =
					isKeyPressed(settings.playPauseKey);

				if (pauseKeyIsDown && !pauseKeyWasDown)
				{
					paused = !paused;

					if (paused)
					{
						std::cout << "Playback paused\n";
					}
					else
					{
						std::cout << "Playback resumed\n";
					}
				}

				pauseKeyWasDown = pauseKeyIsDown;

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

            if (cancelled)
            {
                break;
            }

            if (event.type == EventType::Wait)
            {
                addedWaitMicroseconds +=
                    event.waitMicroseconds;

                continue;
            }

            if (!backend.execute(event, errorMessage))
            {
                backend.releaseAll();

                std::cerr
                    << "Playback failed: "
                    << errorMessage
                    << '\n';

                return 1;
            }
        }

        backend.releaseAll();

        if (cancelled)
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

    if (cancelled)
	{
		std::cout << "Playback cancelled\n";
		return 4;
	}

    std::cout << "Playback completed\n";
    return 0;
}
}

int main(int argc, char* argv[])
{
    Settings settings;
	std::string settingsPath;
	std::string settingsError;

	if (!loadOrCreateSettings(
			settings,
			settingsPath,
			settingsError))
	{
		std::cerr
			<< "Unable to load settings: "
			<< settingsError
			<< '\n';

		return 1;
	}
		
	
	
	
    if (argc < 2)
    {
        std::cout << "InputPlay\n";
        std::cout << "Use --help for commands\n";
        return 0;
    }

    const std::string command = argv[1];

    if (command == "--help" || command == "help")
    {
        printHelp();
        return 0;
    }

    if (command == "test-model")
    {
        return runModelTest();
    }

    if (command == "record")
    {
        if (argc < 3)
        {
            std::cerr << "Missing recording file path\n";
            std::cerr << "Usage: InputPlay record <file>\n";
            return 2;
        }

        return runRecordCommand(
			argv[2],
			settings);
    }

    if (command == "info")
    {
        if (argc < 3)
        {
            std::cerr << "Missing recording file path\n";
            std::cerr << "Usage: InputPlay info <file>\n";
            return 2;
        }

        return runInfoCommand(argv[2]);
    }
	
	if (command == "validate")
	{
		if (argc < 3)
		{
			std::cerr
				<< "Missing recording file path\n";

			std::cerr
				<< "Usage: InputPlay validate <file>\n";

			return 2;
		}

		return runValidateCommand(argv[2]);
	}

	if (command == "play")
	{
		if (argc < 3)
		{
			std::cerr << "Missing recording file path\n";
			std::cerr
				<< "Usage: InputPlay play <file> "
				<< "[--send-input] [--align-start] "
				<< "[--loops <number|inf>]\n";

			return 2;
		}

		bool useSendInput = false;
		bool alignStart = false;
		bool infiniteLoops = false;
		bool strictDisplay = false;
		bool ignoreDisplay = false;

		unsigned int loopCount =
			settings.defaultLoops;

		for (int argumentIndex = 3;
			 argumentIndex < argc;
			 ++argumentIndex)
		{
			const std::string option =
				argv[argumentIndex];

			if (option == "--send-input")
			{
				useSendInput = true;
			}
			else if (option == "--align-start")
			{
				alignStart = true;
			}
			else if (option == "--strict-display")
			{
				strictDisplay = true;
			}
			else if (option == "--ignore-display")
			{
				ignoreDisplay = true;
			}
			else if (option == "--loops")
			{
				if (argumentIndex + 1 >= argc)
				{
					std::cerr
						<< "--loops requires a number or inf\n";

					return 2;
				}

				const std::string loopValue =
					argv[++argumentIndex];

				if (loopValue == "inf")
				{
					infiniteLoops = true;
				}
				else
				{
					try
					{
						const unsigned long parsed =
							std::stoul(loopValue);

						if (parsed == 0)
						{
							throw std::invalid_argument(
								"zero loops");
						}

						loopCount =
							static_cast<unsigned int>(parsed);
					}
					catch (...)
					{
						std::cerr
							<< "Invalid loop count: "
							<< loopValue
							<< '\n';

						return 2;
					}
				}
			}
			else
			{
				std::cerr
					<< "Unknown playback option: "
					<< option
					<< '\n';

				return 2;
			}
		}

		if (strictDisplay && ignoreDisplay)
		{
			std::cerr
				<< "--strict-display and --ignore-display "
				<< "cannot be used together\n";

			return 2;
		}
		if (useSendInput)
		{
			std::cout
				<< "WARNING: Live input playback enabled\n";

			SendInputBackend backend;

			return runPlayCommand(
				argv[2],
				backend,
				alignStart,
				strictDisplay,
				ignoreDisplay,
				settings,
				loopCount,
				infiniteLoops);
		}

		DryRunBackend backend;

		return runPlayCommand(
			argv[2],
			backend,
			alignStart,
			strictDisplay,
			ignoreDisplay,
			settings,
			loopCount,
			infiniteLoops);
	}

    std::cerr << "Unknown command: "
              << command
              << '\n';

    return 1;
}