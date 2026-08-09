#include "Recording.h"
#include "RecordingFile.h"

#include <chrono>
#include <iostream>
#include <string>
#include <thread>

namespace
{
void printHelp()
{
    std::cout << "InputPlay\n\n";
    std::cout << "Commands:\n";
    std::cout << "  record <file>\n";
    std::cout << "  play <file>\n";
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

    return recording;
}

int runRecordCommand(const std::string& filePath)
{
    const Recording recording = createSampleRecording();

    std::string errorMessage;

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

    std::cout << "Sample recording saved\n";
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

    if (recording.eventCount() != 3)
    {
        std::cerr << "Model test failed\n";
        return 1;
    }

    std::cout << "Model test passed\n";
    return 0;
}

int runPlayCommand(const std::string& filePath)
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

    std::cout << "Starting dry-run playback\n";
    std::cout << "Events: "
              << recording.eventCount()
              << '\n';

    using Clock = std::chrono::steady_clock;

    const Clock::time_point playbackStart = Clock::now();

    for (const InputEvent& event : recording.events())
    {
        const Clock::time_point eventDeadline =
            playbackStart
            + std::chrono::microseconds(
                event.timestampMicroseconds);

        std::this_thread::sleep_until(eventDeadline);

        std::cout
            << "Playing "
            << eventTypeName(event.type)
            << " at "
            << event.timestampMicroseconds
            << " microseconds\n";
    }

    std::cout << "Dry-run playback completed\n";
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
			std::cerr << "Usage: InputPlay play <file>\n";
			return 2;
		}

		return runPlayCommand(argv[2]);
	}

    std::cerr << "Unknown command: "
              << command
              << '\n';

    return 1;
}