#include "RecordingFile.h"

#include <fstream>
#include <iomanip>
#include <string>

namespace
{
constexpr const char* FileSignature = "INPUTPLAY";
constexpr unsigned int CurrentFileVersion = 3;
constexpr std::size_t MaximumEventCount = 10'000'000;
constexpr std::size_t MaximumMonitorCount = 64;

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

        case 6:
            type = EventType::Wait;
            return true;
	    case 7:
			type = EventType::MouseTeleport;
			return true;

        default:
            return false;
    }
}

bool loadVersionOne(
    std::ifstream& inputFile,
    Recording& loadedRecording,
    std::size_t& eventCount,
    std::string& errorMessage)
{
    inputFile >> eventCount;

    if (!inputFile)
    {
        errorMessage =
            "The recording does not contain a valid event count.";

        return false;
    }

    return true;
}

bool loadVersionTwoStartMetadata(
    std::ifstream& inputFile,
    Recording& loadedRecording,
    std::size_t& eventCount,
    std::string& errorMessage)
{
    std::string startCursorLabel;
    int startX = 0;
    int startY = 0;
    int hasStartPosition = 0;

    inputFile
        >> startCursorLabel
        >> startX
        >> startY
        >> hasStartPosition;

    if (!inputFile || startCursorLabel != "START_CURSOR")
    {
        errorMessage =
            "The recording has invalid start-cursor metadata.";

        return false;
    }

    if (hasStartPosition != 0)
    {
        loadedRecording.setStartingCursorPosition(
            startX,
            startY);
    }

    std::string eventsLabel;

    inputFile >> eventsLabel >> eventCount;

    if (!inputFile || eventsLabel != "EVENTS")
    {
        errorMessage =
            "The recording has an invalid event header.";

        return false;
    }

    return true;
}

bool loadVersionThreeMetadata(
    std::ifstream& inputFile,
    Recording& loadedRecording,
    std::size_t& eventCount,
    std::string& errorMessage)
{
    std::string startCursorLabel;
    int startX = 0;
    int startY = 0;
    int hasStartPosition = 0;

    inputFile
        >> startCursorLabel
        >> startX
        >> startY
        >> hasStartPosition;

    if (!inputFile || startCursorLabel != "START_CURSOR")
    {
        errorMessage =
            "The recording has invalid start-cursor metadata.";

        return false;
    }

    if (hasStartPosition != 0)
    {
        loadedRecording.setStartingCursorPosition(
            startX,
            startY);
    }

    DisplayMetadata displayMetadata;
    std::string virtualDesktopLabel;

    inputFile
        >> virtualDesktopLabel
        >> displayMetadata.virtualDesktopLeft
        >> displayMetadata.virtualDesktopTop
        >> displayMetadata.virtualDesktopWidth
        >> displayMetadata.virtualDesktopHeight;

    if (!inputFile
        || virtualDesktopLabel != "VIRTUAL_DESKTOP")
    {
        errorMessage =
            "The recording has invalid virtual-desktop metadata.";

        return false;
    }

    std::string monitorsLabel;
    std::size_t monitorCount = 0;

    inputFile >> monitorsLabel >> monitorCount;

    if (!inputFile || monitorsLabel != "MONITORS")
    {
        errorMessage =
            "The recording has an invalid monitor header.";

        return false;
    }

    if (monitorCount > MaximumMonitorCount)
    {
        errorMessage =
            "The recording contains too many monitor records.";

        return false;
    }

    for (std::size_t index = 0;
         index < monitorCount;
         ++index)
    {
        std::string monitorLabel;
        MonitorMetadata monitor;
        int isPrimary = 0;

        inputFile
            >> monitorLabel
            >> std::quoted(monitor.deviceName)
            >> monitor.left
            >> monitor.top
            >> monitor.right
            >> monitor.bottom
            >> monitor.workLeft
            >> monitor.workTop
            >> monitor.workRight
            >> monitor.workBottom
            >> isPrimary;

        if (!inputFile || monitorLabel != "MONITOR")
        {
            errorMessage =
                "The recording contains invalid monitor metadata "
                "at index "
                + std::to_string(index)
                + ".";

            return false;
        }

        monitor.primary = isPrimary != 0;

        displayMetadata.monitors.push_back(monitor);
    }

    loadedRecording.setDisplayMetadata(displayMetadata);

    std::string eventsLabel;

    inputFile >> eventsLabel >> eventCount;

    if (!inputFile || eventsLabel != "EVENTS")
    {
        errorMessage =
            "The recording has an invalid event header.";

        return false;
    }

    return true;
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
        errorMessage =
            "Unable to open the recording file for writing.";

        return false;
    }

    outputFile
        << FileSignature
        << ' '
        << CurrentFileVersion
        << '\n';

    outputFile
        << "START_CURSOR "
        << recording.startingCursorX()
        << ' '
        << recording.startingCursorY()
        << ' '
        << (recording.hasStartingCursorPosition() ? 1 : 0)
        << '\n';

    const DisplayMetadata& displayMetadata =
        recording.displayMetadata();

    outputFile
        << "VIRTUAL_DESKTOP "
        << displayMetadata.virtualDesktopLeft
        << ' '
        << displayMetadata.virtualDesktopTop
        << ' '
        << displayMetadata.virtualDesktopWidth
        << ' '
        << displayMetadata.virtualDesktopHeight
        << '\n';

    outputFile
        << "MONITORS "
        << displayMetadata.monitors.size()
        << '\n';

    for (const MonitorMetadata& monitor
         : displayMetadata.monitors)
    {
        outputFile
            << "MONITOR "
            << std::quoted(monitor.deviceName)
            << ' '
            << monitor.left
            << ' '
            << monitor.top
            << ' '
            << monitor.right
            << ' '
            << monitor.bottom
            << ' '
            << monitor.workLeft
            << ' '
            << monitor.workTop
            << ' '
            << monitor.workRight
            << ' '
            << monitor.workBottom
            << ' '
            << (monitor.primary ? 1 : 0)
            << '\n';
    }

    outputFile
        << "EVENTS "
        << recording.eventCount()
        << '\n';

    for (const InputEvent& event : recording.events())
    {
        outputFile
            << event.timestampMicroseconds
            << ' '
            << eventTypeToInteger(event.type)
            << ' '
            << event.mouseX
            << ' '
            << event.mouseY
            << ' '
            << event.mouseDeltaX
            << ' '
            << event.mouseDeltaY
            << ' '
            << event.mouseButton
            << ' '
            << event.mouseWheelDelta
            << ' '
            << event.keyCode
            << ' '
            << event.waitMicroseconds
            << '\n';
    }

    if (!outputFile)
    {
        errorMessage =
            "An error occurred while writing the recording file.";

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
        errorMessage =
            "Unable to open the recording file.";

        return false;
    }

    std::string signature;
    unsigned int version = 0;

    inputFile >> signature >> version;

    if (!inputFile)
    {
        errorMessage =
            "The recording header is incomplete.";

        return false;
    }

    if (signature != FileSignature)
    {
        errorMessage =
            "The file is not an InputPlay recording.";

        return false;
    }

    if (version < 1 || version > CurrentFileVersion)
    {
        errorMessage =
            "The recording version is not supported.";

        return false;
    }

    Recording loadedRecording;
    std::size_t eventCount = 0;
    bool metadataLoaded = false;

    if (version == 1)
    {
        metadataLoaded = loadVersionOne(
            inputFile,
            loadedRecording,
            eventCount,
            errorMessage);
    }
    else if (version == 2)
    {
        metadataLoaded = loadVersionTwoStartMetadata(
            inputFile,
            loadedRecording,
            eventCount,
            errorMessage);
    }
    else
    {
        metadataLoaded = loadVersionThreeMetadata(
            inputFile,
            loadedRecording,
            eventCount,
            errorMessage);
    }

    if (!metadataLoaded)
    {
        return false;
    }

    if (eventCount > MaximumEventCount)
    {
        errorMessage =
            "The recording contains too many events.";

        return false;
    }

    for (std::size_t index = 0;
         index < eventCount;
         ++index)
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

        if (version >= 2)
        {
            inputFile >> event.waitMicroseconds;
        }

        if (!inputFile)
        {
            errorMessage =
                "The recording contains an incomplete event "
                "at index "
                + std::to_string(index)
                + ".";

            return false;
        }

        if (!integerToEventType(
                eventTypeValue,
                event.type))
        {
            errorMessage =
                "The recording contains an unknown event type "
                "at index "
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