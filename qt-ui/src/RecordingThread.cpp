#include "RecordingThread.h"
#include "InputRecorder.h"
#include "DisplayMetadata.h"

RecordingThread::RecordingThread(QObject* parent)
    : QThread(parent)
    , recording_(std::make_unique<Recording>())
{
}

RecordingThread::~RecordingThread()
{
    if (isRunning()) {
        controller_.requestCancel();
        wait(5000);
        if (isRunning()) {
            terminate();
            wait();
        }
    }
}

void RecordingThread::setOptions(const RecordingOptions& options)
{
    options_ = options;
}

std::unique_ptr<Recording> RecordingThread::takeRecording()
{
    return std::move(recording_);
}

bool RecordingThread::isRecording() const
{
    return running_ && !controller_.paused();
}

bool RecordingThread::isPaused() const
{
    return running_ && controller_.paused();
}

void RecordingThread::startRecording()
{
    if (isRunning()) {
        return;
    }
    
    // Reset for new recording
    recording_ = std::make_unique<Recording>();
    controller_.reset();
    controller_.requestStart();
    
    start();
}

void RecordingThread::pauseRecording()
{
    if (running_) {
        controller_.requestPause();
    }
}

void RecordingThread::resumeRecording()
{
    if (running_) {
        controller_.requestResume();
    }
}

void RecordingThread::togglePause()
{
    if (running_) {
        controller_.togglePause();
    }
}

void RecordingThread::stopRecording()
{
    if (running_) {
        controller_.requestStop();
    }
}

void RecordingThread::cancelRecording()
{
    if (running_) {
        controller_.requestCancel();
    }
}

void RecordingThread::run()
{
    running_ = true;
    
    // Capture display metadata before recording
    DisplayMetadata displayMetadata;
    std::string displayError;
    if (captureDisplayMetadata(displayMetadata, displayError)) {
        recording_->setDisplayMetadata(displayMetadata);
    }
    
    InputRecorder recorder;
    
    RecordingCallbacks callbacks;
    callbacks.onProgress = [this](const RecordingProgress& progress) {
        emit recordingProgress(progress);
        
        if (progress.state == RecordingState::Recording) {
            static bool wasRecording = false;
            if (!wasRecording) {
                emit recordingStarted();
                wasRecording = true;
            }
        } else if (progress.state == RecordingState::Paused) {
            emit recordingPaused();
        }
    };
    
    RecordingResult result = recorder.record(
        *recording_,
        options_,
        controller_,
        callbacks
    );
    
    running_ = false;
    
    emit recordingStopped(result);
}
