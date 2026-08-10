#include "PlaybackController.h"
#include "PlaybackEngine.h"
#include "PlaybackOptions.h"
#include "PlaybackProgress.h"
#include "PlaybackResult.h"
#include "RecordingFile.h"
#include "Settings.h"
#include "Recording.h"
#include "RecordingSummary.h"
#include "RecordingValidator.h"
#include "TestInputBackend.h"

#include <iostream>
#include <string>
#include <filesystem>
#include <vector>
#include <functional>

namespace
{
int failures = 0;

class PlaybackProgressCollector
{
public:
    void operator()(
        const PlaybackProgress& progress)
    {
        updates.push_back(progress);
    }

    [[nodiscard]]
    bool containsState(
        PlaybackState state) const
    {
        for (const PlaybackProgress& progress
             : updates)
        {
            if (progress.state == state)
            {
                return true;
            }
        }

        return false;
    }

    std::vector<PlaybackProgress> updates;
};

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

std::filesystem::path createPlaybackTestFile()
{
    const std::filesystem::path filePath =
        std::filesystem::current_path()
        / "inputplay-core-playback-test.irec";

    const Recording recording =
        createTestRecording();

    std::string errorMessage;

    const bool saved =
        RecordingFile::save(
            recording,
            filePath.string(),
            errorMessage);

    expect(
        saved,
        "Playback test recording saves successfully");

    return filePath;
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

void testSuccessfulPlayback()
{
    const std::filesystem::path filePath =
        createPlaybackTestFile();

    TestInputBackend backend;
    PlaybackController controller;

    PlaybackOptions options;
    options.startImmediately = true;
    options.ignoreDisplay = true;
    options.loopCount = 1;

    Settings settings;

    PlaybackProgressCollector collector;

    PlaybackCallbacks callbacks;
    callbacks.onProgress =
    std::ref(collector);

    const PlaybackResult result =
        runPlayback(
            filePath.string(),
            backend,
            settings,
            options,
            controller,
            callbacks);

    expect(
        result.code
            == PlaybackResultCode::Completed,
        "Playback engine completes successfully");

    expect(
        result.completedLoops == 1,
        "Playback engine reports one completed loop");

    expect(
        result.completedEvents == 3,
        "Playback engine reports three completed events");

    expect(
        backend.executedEvents().size() == 3,
        "Playback engine executes every event");

    expect(
        backend.executedEvents()[0].type
            == EventType::MouseMove,
        "Playback preserves first event type");

    expect(
        backend.executedEvents()[1].type
            == EventType::KeyDown,
        "Playback preserves second event type");

    expect(
        backend.executedEvents()[2].type
            == EventType::KeyUp,
        "Playback preserves third event type");

    expect(
        collector.containsState(
            PlaybackState::Preparing),
        "Playback reports preparing state");

    expect(
        collector.containsState(
            PlaybackState::Playing),
        "Playback reports playing state");

    expect(
        collector.containsState(
            PlaybackState::LoopCompleted),
        "Playback reports loop completion");

    expect(
        collector.containsState(
            PlaybackState::Completed),
        "Playback reports completed state");

    expect(
        backend.releaseCount() >= 1,
        "Playback releases backend inputs");

    std::filesystem::remove(filePath);
}

void testMultiplePlaybackLoops()
{
    const std::filesystem::path filePath =
        createPlaybackTestFile();

    TestInputBackend backend;
    PlaybackController controller;

    PlaybackOptions options;
    options.startImmediately = true;
    options.ignoreDisplay = true;
    options.loopCount = 3;

    Settings settings;

    PlaybackCallbacks callbacks;

    const PlaybackResult result =
        runPlayback(
            filePath.string(),
            backend,
            settings,
            options,
            controller,
            callbacks);

    expect(
        result.code
            == PlaybackResultCode::Completed,
        "Multi-loop playback completes");

    expect(
        result.completedLoops == 3,
        "Multi-loop playback reports three loops");

    expect(
        result.completedEvents == 9,
        "Multi-loop playback reports nine processed events");

    expect(
        backend.executedEvents().size() == 9,
        "Multi-loop playback executes events three times");

    std::filesystem::remove(filePath);
}

void testPlaybackBackendFailure()
{
    const std::filesystem::path filePath =
        createPlaybackTestFile();

    TestInputBackend backend;

    backend.setFailureAfter(
        1,
        "Simulated playback backend failure.");

    PlaybackController controller;

    PlaybackOptions options;
    options.startImmediately = true;
    options.ignoreDisplay = true;

    Settings settings;

    PlaybackProgressCollector collector;

    PlaybackCallbacks callbacks;
    callbacks.onProgress =
    std::ref(collector);

    const PlaybackResult result =
        runPlayback(
            filePath.string(),
            backend,
            settings,
            options,
            controller,
            callbacks);

    expect(
        result.code
            == PlaybackResultCode::BackendFailed,
        "Playback reports backend failure");

    expect(
        result.completedEvents == 1,
        "Playback counts events completed before failure");

    expect(
        result.message
            == "Simulated playback backend failure.",
        "Playback preserves backend failure message");

    expect(
        collector.containsState(
            PlaybackState::Failed),
        "Playback reports failed state");

    expect(
        backend.releaseCount() >= 1,
        "Failed playback releases backend inputs");

    std::filesystem::remove(filePath);
}


void testPlaybackCancellation()
{
    const std::filesystem::path filePath =
        createPlaybackTestFile();

    TestInputBackend backend;
    PlaybackController controller;

    controller.requestCancel();

    PlaybackOptions options;
    options.startImmediately = true;
    options.ignoreDisplay = true;
    options.loopCount = 1;

    Settings settings;

    PlaybackProgressCollector collector;

    PlaybackCallbacks callbacks;
    callbacks.onProgress =
    std::ref(collector);

    const PlaybackResult result =
        runPlayback(
            filePath.string(),
            backend,
            settings,
            options,
            controller,
            callbacks);

    expect(
        result.code
            == PlaybackResultCode::Cancelled,
        "Playback honors in-process cancellation");

    expect(
        backend.executedEvents().empty(),
        "Cancelled playback executes no events");

    expect(
        collector.containsState(
            PlaybackState::Cancelled),
        "Cancelled playback reports cancelled state");

    std::filesystem::remove(filePath);
}

}

int main()
{
    testRecordingSummary();
    testRecordingValidation();
    testBackendExecution();
    testBackendFailure();

    testSuccessfulPlayback();
    testMultiplePlaybackLoops();
    testPlaybackBackendFailure();
    testPlaybackCancellation();

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