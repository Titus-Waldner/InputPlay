#include "Recording.h"
#include "RecordingFile.h"
#include "DryRunBackend.h"
#include "SendInputBackend.h"
#include "MouseRecorder.h"

#include <chrono>
#include <iostream>
#include <string>
#include <thread>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace
{
void printHelp()
{
    std::cout << "InputPlay\n\n";
    std::cout << "Commands:\n";
    std::cout << "  record <file>\n";
    std::cout << " play <file> [--send-input] [--align-start]\n";
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

int runPlayCommand(
    const std::string& filePath,
    IInputBackend& backend,
    bool alignStart)
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

    if (recording.empty())
    {
        std::cout << "Recording contains no events\n";
        return 0;
    }

    if (alignStart)
    {
        if (!recording.hasStartingCursorPosition())
        {
            std::cerr
                << "Recording does not contain a starting cursor position\n";

            return 1;
        }

        if (!SetCursorPos(
                recording.startingCursorX(),
                recording.startingCursorY()))
        {
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

    std::cout << "Starting playback\n";
    std::cout << "Events: "
              << recording.eventCount()
              << '\n';

    using Clock = std::chrono::steady_clock;

    const Clock::time_point playbackStart = Clock::now();

    std::uint64_t addedWaitMicroseconds = 0;

    for (const InputEvent& event : recording.events())
    {
        const Clock::time_point eventDeadline =
            playbackStart
            + std::chrono::microseconds(
                event.timestampMicroseconds
                + addedWaitMicroseconds);

        std::this_thread::sleep_until(eventDeadline);

        if (event.type == EventType::Wait)
        {
            addedWaitMicroseconds +=
                event.waitMicroseconds;

            continue;
        }

        if (!backend.execute(event, errorMessage))
        {
            backend.releaseAll();

            std::cerr << "Playback failed: "
                      << errorMessage
                      << '\n';

            return 1;
        }
    }

    backend.releaseAll();

    std::cout << "Playback completed\n";
    return 0;
}
}

int main(int argc, char* argv[])
{
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
				<< "[--send-input] [--align-start]\n";

			return 2;
		}

		bool useSendInput = false;
		bool alignStart = false;

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
				alignStart);
		}

		DryRunBackend backend;

		return runPlayCommand(
			argv[2],
			backend,
			alignStart);
	}

    std::cerr << "Unknown command: "
              << command
              << '\n';

    return 1;
}