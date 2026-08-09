#include "Recording.h"
#include "RecordingFile.h"
#include "DryRunBackend.h"
#include "SendInputBackend.h"
#include "MouseRecorder.h"
#include "Settings.h"

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
    std::cout << "InputPlay\n\n";
    std::cout << "Commands:\n";
    std::cout << "  record <file>\n";
    std::cout
			  << " play <file> [--send-input] [--align-start] "
			  << "[--loops <number|inf>]\n";
    std::cout << "  info <file>\n";
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

int runRecordCommand(const std::string& filePath)
{
    Recording recording;
    MouseRecorder recorder;
    std::string errorMessage;

    if (!recorder.record(recording, errorMessage))
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
        std::cerr << "Unable to load recording: "
                  << errorMessage
                  << '\n';

        return 1;
    }

    std::cout << "InputPlay recording information\n";
    std::cout << "File: " << filePath << '\n';
    std::cout << "Events: "
              << recording.eventCount()
              << '\n';

    std::size_t eventNumber = 1;

    for (const InputEvent& event : recording.events())
    {
        std::cout
            << eventNumber
            << ": "
            << eventTypeName(event.type)
            << " at "
            << event.timestampMicroseconds
            << " microseconds\n";

        ++eventNumber;
    }

    return 0;
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
int runPlayCommand(
    const std::string& filePath,
    IInputBackend& backend,
    bool alignStart,
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

        return runRecordCommand(argv[2]);
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

		if (useSendInput)
		{
			std::cout
				<< "WARNING: Live input playback enabled\n";

			SendInputBackend backend;

			return runPlayCommand(
				argv[2],
				backend,
				alignStart,
				settings,
				loopCount,
				infiniteLoops);
		}

		DryRunBackend backend;

		return runPlayCommand(
			argv[2],
			backend,
			alignStart,
			settings,
			loopCount,
			infiniteLoops);
	}

    std::cerr << "Unknown command: "
              << command
              << '\n';

    return 1;
}