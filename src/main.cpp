#include "Recording.h"

#include <iostream>
#include <string>

void printHelp()
{
    std::cout << "InputPlay\n\n";
    std::cout << "Commands:\n";
    std::cout << "  record <file>\n";
    std::cout << "  play <file>\n";
    std::cout << "  info <file>\n";
    std::cout << "  test-model\n";
}

int runModelTest()
{
    Recording recording;

    InputEvent firstMove;
    firstMove.timestampMicroseconds = 0;
    firstMove.type = EventType::MouseMove;
    firstMove.mouseDeltaX = 5;
    firstMove.mouseDeltaY = 2;

    InputEvent secondMove;
    secondMove.timestampMicroseconds = 10'000;
    secondMove.type = EventType::MouseMove;
    secondMove.mouseDeltaX = 3;
    secondMove.mouseDeltaY = -1;

    InputEvent leftButtonDown;
    leftButtonDown.timestampMicroseconds = 20'000;
    leftButtonDown.type = EventType::MouseButtonDown;
    leftButtonDown.mouseButton = 1;

    recording.addEvent(firstMove);
    recording.addEvent(secondMove);
    recording.addEvent(leftButtonDown);

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
        std::cout << "Record command selected\n";
        return 0;
    }

    if (command == "play")
    {
        std::cout << "Play command selected\n";
        return 0;
    }

    if (command == "info")
    {
        std::cout << "Info command selected\n";
        return 0;
    }

    std::cerr << "Unknown command: "
              << command
              << '\n';

    return 1;
}