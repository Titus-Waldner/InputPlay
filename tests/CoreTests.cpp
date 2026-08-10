#include "Recording.h"
#include "RecordingSummary.h"
#include "RecordingValidator.h"
#include "TestInputBackend.h"

#include <iostream>
#include <string>

namespace
{
int failures = 0;

void expect(
    bool condition,
    const std::string& testName)
{
    if (condition)
    {
        std::cout
            << "PASS: "
            << testName
            << '\n';

        return;
    }

    std::cerr
        << "FAIL: "
        << testName
        << '\n';

    ++failures;
}

Recording createTestRecording()
{
    Recording recording;

    InputEvent movement;
    movement.timestampMicroseconds = 0;
    movement.type = EventType::MouseMove;
    movement.mouseDeltaX = 10;
    movement.mouseDeltaY = 5;

    recording.addEvent(movement);

    InputEvent keyDown;
    keyDown.timestampMicroseconds = 1000;
    keyDown.type = EventType::KeyDown;
    keyDown.keyCode = 30;

    recording.addEvent(keyDown);

    InputEvent keyUp;
    keyUp.timestampMicroseconds = 2000;
    keyUp.type = EventType::KeyUp;
    keyUp.keyCode = 30;

    recording.addEvent(keyUp);

    return recording;
}

void testRecordingSummary()
{
    const Recording recording =
        createTestRecording();

    const RecordingSummary summary =
        summarizeRecording(recording);

    expect(
        summary.totalEvents == 3,
        "Summary counts total events");

    expect(
        summary.mouseMovements == 1,
        "Summary counts mouse movements");

    expect(
        summary.keyboardEvents == 2,
        "Summary counts keyboard events");

    expect(
        summary.totalDurationMicroseconds == 2000,
        "Summary calculates duration");
}

void testRecordingValidation()
{
    const Recording recording =
        createTestRecording();

    const ValidationResult result =
        validateRecording(recording);

    expect(
        result.valid(),
        "Balanced recording is valid");
}

void testBackendExecution()
{
    const Recording recording =
        createTestRecording();

    TestInputBackend backend;
    std::string errorMessage;

    for (const InputEvent& event
         : recording.events())
    {
        const bool succeeded =
            backend.execute(
                event,
                errorMessage);

        expect(
            succeeded,
            "Test backend accepts event");
    }

    expect(
        backend.executedEvents().size() == 3,
        "Test backend stores executed events");

    backend.releaseAll();

    expect(
        backend.releaseCount() == 1,
        "Test backend tracks cleanup calls");
}

void testBackendFailure()
{
    const Recording recording =
        createTestRecording();

    TestInputBackend backend;

    backend.setFailureAfter(
        1,
        "Simulated backend failure.");

    std::string errorMessage;

    const bool firstResult =
        backend.execute(
            recording.events()[0],
            errorMessage);

    const bool secondResult =
        backend.execute(
            recording.events()[1],
            errorMessage);

    expect(
        firstResult,
        "Backend executes events before failure point");

    expect(
        !secondResult,
        "Backend fails at configured failure point");

    expect(
        errorMessage
            == "Simulated backend failure.",
        "Backend returns configured failure message");
}
}

int main()
{
    testRecordingSummary();
    testRecordingValidation();
    testBackendExecution();
    testBackendFailure();

    std::cout << '\n';

    if (failures == 0)
    {
        std::cout
            << "All core tests passed\n";

        return 0;
    }

    std::cerr
        << failures
        << " core test(s) failed\n";

    return 1;
}