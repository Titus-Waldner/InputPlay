#include "RecordingFile.h"

#include <fstream>
#include <limits>
#include <string>

namespace
{
constexpr const char* FileSignature = "INPUTPLAY";
constexpr unsigned int FileVersion = 1;
constexpr std::size_t MaximumEventCount = 10'000'000;

int eventTypeToInteger(EventType type)
{
    return static_cast<int>(type);
}

bool integerToEventType(int value, EventType& type)
{
    switch (value)
    {
        case 0:
            type = EventType::MouseMove;
            return true;

        case 1:
            type = EventType::MouseButtonDown;
            return true;

        case 2:
            type = EventType::MouseButtonUp;
            return true;

        case 3:
            type = EventType::MouseWheel;
            return true;

        case 4:
            type = EventType::KeyDown;
            return true;

        case 5:
            type = EventType::KeyUp;
            return true;

        default:
            return false;
    }
}
}

bool RecordingFile::save(
    const Recording& recording,
    const std::string& filePath,
    std::string& errorMessage)
{
    std::ofstream outputFile(filePath);

    if (!outputFile)
    {
        errorMessage = "Unable to open the recording file for writing.";
        return false;
    }

    outputFile << FileSignature << ' '
               << FileVersion << '\n';

    outputFile << recording.eventCount() << '\n';

    for (const InputEvent& event : recording.events())
    {
        outputFile
            << event.timestampMicroseconds << ' '
            << eventTypeToInteger(event.type) << ' '
            << event.mouseX << ' '
            << event.mouseY << ' '
            << event.mouseDeltaX << ' '
            << event.mouseDeltaY << ' '
            << event.mouseButton << ' '
            << event.mouseWheelDelta << ' '
            << event.keyCode << '\n';
    }

    if (!outputFile)
    {
        errorMessage = "An error occurred while writing the recording file.";
        return false;
    }

    errorMessage.clear();
    return true;
}

bool RecordingFile::load(
    const std::string& filePath,
    Recording& recording,
    std::string& errorMessage)
{
    std::ifstream inputFile(filePath);

    if (!inputFile)
    {
        errorMessage = "Unable to open the recording file.";
        return false;
    }

    std::string signature;
    unsigned int version = 0;

    inputFile >> signature >> version;

    if (!inputFile)
    {
        errorMessage = "The recording header is incomplete.";
        return false;
    }

    if (signature != FileSignature)
    {
        errorMessage = "The file is not an InputPlay recording.";
        return false;
    }

    if (version != FileVersion)
    {
        errorMessage = "The recording version is not supported.";
        return false;
    }

    std::size_t eventCount = 0;
    inputFile >> eventCount;

    if (!inputFile)
    {
        errorMessage = "The recording does not contain a valid event count.";
        return false;
    }

    if (eventCount > MaximumEventCount)
    {
        errorMessage = "The recording contains too many events.";
        return false;
    }

    Recording loadedRecording;

    for (std::size_t index = 0; index < eventCount; ++index)
    {
        InputEvent event;
        int eventTypeValue = 0;

        inputFile
            >> event.timestampMicroseconds
            >> eventTypeValue
            >> event.mouseX
            >> event.mouseY
            >> event.mouseDeltaX
            >> event.mouseDeltaY
            >> event.mouseButton
            >> event.mouseWheelDelta
            >> event.keyCode;

        if (!inputFile)
        {
            errorMessage =
                "The recording contains an incomplete event at index "
                + std::to_string(index)
                + ".";

            return false;
        }

        if (!integerToEventType(eventTypeValue, event.type))
        {
            errorMessage =
                "The recording contains an unknown event type at index "
                + std::to_string(index)
                + ".";

            return false;
        }

        loadedRecording.addEvent(event);
    }

    recording = loadedRecording;
    errorMessage.clear();
    return true;
}