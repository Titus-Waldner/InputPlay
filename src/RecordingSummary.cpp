#include "RecordingSummary.h"

RecordingSummary summarizeRecording(
    const Recording& recording)
{
    RecordingSummary summary;

    summary.totalEvents =
        recording.eventCount();

    for (const InputEvent& event
         : recording.events())
    {
        if (event.timestampMicroseconds
            > summary.eventDurationMicroseconds)
        {
            summary.eventDurationMicroseconds =
                event.timestampMicroseconds;
        }

        switch (event.type)
        {
            case EventType::MouseMove:
                ++summary.mouseMovements;
                break;

            case EventType::MouseTeleport:
                ++summary.mouseTeleports;
                break;

            case EventType::MouseButtonDown:
            case EventType::MouseButtonUp:
                ++summary.mouseButtonEvents;
                break;

            case EventType::MouseWheel:
                ++summary.mouseWheelEvents;
                break;

            case EventType::KeyDown:
            case EventType::KeyUp:
                ++summary.keyboardEvents;
                break;

            case EventType::Wait:
                ++summary.waitEvents;

                summary.explicitWaitMicroseconds +=
                    event.waitMicroseconds;

                break;
        }
    }

    summary.totalDurationMicroseconds =
        summary.eventDurationMicroseconds
        + summary.explicitWaitMicroseconds;

    return summary;
}