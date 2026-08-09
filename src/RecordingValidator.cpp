#include "RecordingValidator.h"

#include <cstdint>
#include <sstream>
#include <unordered_set>

namespace
{
bool pointInsideVirtualDesktop(
    int x,
    int y,
    const DisplayMetadata& display)
{
    const int right =
        display.virtualDesktopLeft
        + display.virtualDesktopWidth;

    const int bottom =
        display.virtualDesktopTop
        + display.virtualDesktopHeight;

    return
        x >= display.virtualDesktopLeft
        && x < right
        && y >= display.virtualDesktopTop
        && y < bottom;
}

std::string eventDescription(
    std::size_t eventIndex)
{
    return
        "Event "
        + std::to_string(eventIndex + 1);
}

void validateDisplayMetadata(
    const Recording& recording,
    ValidationResult& result)
{
    if (!recording.hasDisplayMetadata())
    {
        result.warnings.push_back(
            "The recording does not contain display metadata.");

        return;
    }

    const DisplayMetadata& display =
        recording.displayMetadata();

    if (display.virtualDesktopWidth <= 0
        || display.virtualDesktopHeight <= 0)
    {
        result.errors.push_back(
            "The virtual desktop has invalid dimensions.");
    }

    if (display.monitors.empty())
    {
        result.errors.push_back(
            "The display metadata does not contain any monitors.");

        return;
    }

    std::size_t primaryMonitorCount = 0;

    for (std::size_t index = 0;
         index < display.monitors.size();
         ++index)
    {
        const MonitorMetadata& monitor =
            display.monitors[index];

        if (monitor.primary)
        {
            ++primaryMonitorCount;
        }

        if (monitor.right <= monitor.left
            || monitor.bottom <= monitor.top)
        {
            result.errors.push_back(
                "Monitor "
                + std::to_string(index + 1)
                + " has invalid display bounds.");
        }

        if (monitor.workRight <= monitor.workLeft
            || monitor.workBottom <= monitor.workTop)
        {
            result.errors.push_back(
                "Monitor "
                + std::to_string(index + 1)
                + " has invalid work-area bounds.");
        }

        if (monitor.workLeft < monitor.left
            || monitor.workTop < monitor.top
            || monitor.workRight > monitor.right
            || monitor.workBottom > monitor.bottom)
        {
            result.warnings.push_back(
                "Monitor "
                + std::to_string(index + 1)
                + " has a work area outside its display bounds.");
        }
    }

    if (primaryMonitorCount == 0)
    {
        result.errors.push_back(
            "No monitor is marked as the primary monitor.");
    }
    else if (primaryMonitorCount > 1)
    {
        result.errors.push_back(
            "More than one monitor is marked as primary.");
    }

    if (recording.hasStartingCursorPosition()
        && !pointInsideVirtualDesktop(
            recording.startingCursorX(),
            recording.startingCursorY(),
            display))
    {
        result.errors.push_back(
            "The starting cursor position is outside "
            "the recorded virtual desktop.");
    }
}

void validateEventSequence(
    const Recording& recording,
    ValidationResult& result)
{
    std::unordered_set<unsigned int> heldKeys;
    std::unordered_set<int> heldMouseButtons;

    std::uint64_t previousTimestamp = 0;
    bool hasPreviousTimestamp = false;

    for (std::size_t index = 0;
         index < recording.events().size();
         ++index)
    {
        const InputEvent& event =
            recording.events()[index];

        const std::string prefix =
            eventDescription(index);

        if (hasPreviousTimestamp
            && event.timestampMicroseconds
                < previousTimestamp)
        {
            result.errors.push_back(
                prefix
                + " has a timestamp earlier than "
                "the previous event.");
        }

        previousTimestamp =
            event.timestampMicroseconds;

        hasPreviousTimestamp = true;

        switch (event.type)
        {
            case EventType::MouseMove:
                if (event.mouseDeltaX == 0
                    && event.mouseDeltaY == 0)
                {
                    result.warnings.push_back(
                        prefix
                        + " is a zero-distance mouse movement.");
                }

                break;

            case EventType::MouseTeleport:
                if (recording.hasDisplayMetadata()
                    && !pointInsideVirtualDesktop(
                        event.mouseX,
                        event.mouseY,
                        recording.displayMetadata()))
                {
                    result.errors.push_back(
                        prefix
                        + " teleports the cursor outside "
                        "the recorded virtual desktop.");
                }

                break;

            case EventType::MouseButtonDown:
                if (event.mouseButton < 1
                    || event.mouseButton > 3)
                {
                    result.errors.push_back(
                        prefix
                        + " contains an unsupported mouse button.");
                }
                else if (heldMouseButtons.contains(
                             event.mouseButton))
                {
                    result.warnings.push_back(
                        prefix
                        + " presses a mouse button that "
                        "is already held.");
                }
                else
                {
                    heldMouseButtons.insert(
                        event.mouseButton);
                }

                break;

            case EventType::MouseButtonUp:
                if (event.mouseButton < 1
                    || event.mouseButton > 3)
                {
                    result.errors.push_back(
                        prefix
                        + " contains an unsupported mouse button.");
                }
                else if (!heldMouseButtons.contains(
                              event.mouseButton))
                {
                    result.warnings.push_back(
                        prefix
                        + " releases a mouse button that "
                        "was not held.");
                }
                else
                {
                    heldMouseButtons.erase(
                        event.mouseButton);
                }

                break;

            case EventType::MouseWheel:
                if (event.mouseWheelDelta == 0)
                {
                    result.warnings.push_back(
                        prefix
                        + " contains a zero-distance wheel event.");
                }

                break;

            case EventType::KeyDown:
                if (event.keyCode == 0)
                {
                    result.errors.push_back(
                        prefix
                        + " contains an invalid keyboard scan code.");
                }
                else if (heldKeys.contains(event.keyCode))
                {
                    result.warnings.push_back(
                        prefix
                        + " presses a key that is already held.");
                }
                else
                {
                    heldKeys.insert(event.keyCode);
                }

                break;

            case EventType::KeyUp:
                if (event.keyCode == 0)
                {
                    result.errors.push_back(
                        prefix
                        + " contains an invalid keyboard scan code.");
                }
                else if (!heldKeys.contains(event.keyCode))
                {
                    result.warnings.push_back(
                        prefix
                        + " releases a key that was not held.");
                }
                else
                {
                    heldKeys.erase(event.keyCode);
                }

                break;

            case EventType::Wait:
                if (event.waitMicroseconds == 0)
                {
                    result.warnings.push_back(
                        prefix
                        + " contains a zero-duration wait.");
                }

                break;
        }
    }

    for (const unsigned int keyCode : heldKeys)
    {
        std::ostringstream message;

        message
            << "Keyboard scan code "
            << keyCode
            << " remains held at the end of the recording.";

        result.warnings.push_back(
            message.str());
    }

    for (const int mouseButton : heldMouseButtons)
    {
        result.warnings.push_back(
            "Mouse button "
            + std::to_string(mouseButton)
            + " remains held at the end of the recording.");
    }
}
}

ValidationResult validateRecording(
    const Recording& recording)
{
    ValidationResult result;

    if (recording.empty())
    {
        result.errors.push_back(
            "The recording contains no events.");

        return result;
    }

    validateDisplayMetadata(
        recording,
        result);

    validateEventSequence(
        recording,
        result);

    return result;
}