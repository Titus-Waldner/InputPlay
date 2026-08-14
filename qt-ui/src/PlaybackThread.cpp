#include "PlaybackThread.h"
#include "PlaybackEngine.h"
#include "DryRunBackend.h"
#include "SendInputBackend.h"

#include <QMutexLocker>

PlaybackThread::PlaybackThread(QObject* parent)
    : QThread(parent)
    , controller_(std::make_unique<PlaybackController>())
{
}

PlaybackThread::~PlaybackThread()
{
    requestCancel();
    wait();
}

void PlaybackThread::startPlayback(
    const Recording& recording,
    const Settings& settings,
    const PlaybackOptions& options,
    bool dryRun)
{
    QMutexLocker locker(&mutex_);
    
    recording_ = recording;
    settings_ = settings;
    options_ = options;
    dryRun_ = dryRun;
    shouldStop_ = false;
    
    controller_->reset();
    
    start();
}

void PlaybackThread::requestPause()
{
    QMutexLocker locker(&mutex_);
    if (controller_) {
        controller_->requestPause();
    }
}

void PlaybackThread::requestResume()
{
    QMutexLocker locker(&mutex_);
    if (controller_) {
        controller_->requestResume();
    }
}

void PlaybackThread::requestCancel()
{
    QMutexLocker locker(&mutex_);
    shouldStop_ = true;
    if (controller_) {
        controller_->requestCancel();
    }
}

bool PlaybackThread::isPaused() const
{
    QMutexLocker locker(&mutex_);
    return controller_ ? controller_->paused() : false;
}

bool PlaybackThread::isRunning() const
{
    return QThread::isRunning();
}

void PlaybackThread::run()
{
    emit playbackStarted();
    
    // Create backend based on mode
    if (dryRun_) {
        backend_ = std::make_unique<DryRunBackend>();
    } else {
        backend_ = std::make_unique<SendInputBackend>();
    }
    
    // Setup callbacks
    PlaybackCallbacks callbacks;
    callbacks.onProgress = [this](const PlaybackProgress& progress) {
        emit progressChanged(progress);
    };
    
    // Create a temporary file path for the engine (it loads from file)
    // Since we already have the recording in memory, we'll need to work around this
    // by saving to a temp file or modifying the engine
    
    // For now, we'll create a modified approach that works with Recording directly
    // This requires modifications to PlaybackEngine, but for initial implementation
    // we can simulate the playback behavior
    
    try {
        // Run playback - this uses the existing CLI playback engine
        // The engine expects a file path, so we need to adapt
        
        PlaybackResult result;
        result.code = PlaybackResultCode::Completed;
        result.completedLoops = options_.loopCount;
        result.completedEvents = recording_.eventCount();
        
        // Simulate progress for each event
        const auto& events = recording_.events();
        std::size_t totalEvents = events.size();
        
        for (unsigned int loop = 1; loop <= options_.loopCount || options_.infiniteLoops; ++loop) {
            // Check for cancellation
            {
                QMutexLocker locker(&mutex_);
                if (shouldStop_ || controller_->cancellationRequested()) {
                    result.code = PlaybackResultCode::Cancelled;
                    result.completedLoops = loop - 1;
                    emit playbackCompleted(result);
                    return;
                }
            }
            
            for (std::size_t i = 0; i < totalEvents; ++i) {
                // Check for pause
                while (controller_->paused()) {
                    QMutexLocker locker(&mutex_);
                    if (shouldStop_ || controller_->cancellationRequested()) {
                        result.code = PlaybackResultCode::Cancelled;
                        result.completedLoops = loop - 1;
                        result.completedEvents = i;
                        emit playbackCompleted(result);
                        return;
                    }
                    msleep(10);
                }
                
                // Check for cancellation
                {
                    QMutexLocker locker(&mutex_);
                    if (shouldStop_ || controller_->cancellationRequested()) {
                        result.code = PlaybackResultCode::Cancelled;
                        result.completedLoops = loop - 1;
                        result.completedEvents = i;
                        emit playbackCompleted(result);
                        return;
                    }
                }
                
                const InputEvent& event = events[i];
                
                // Execute the event through backend
                std::string errorMessage;
                if (!backend_->execute(event, errorMessage)) {
                    result.code = PlaybackResultCode::BackendFailed;
                    result.message = errorMessage;
                    result.completedLoops = loop - 1;
                    result.completedEvents = i;
                    emit playbackError(QString::fromStdString(errorMessage));
                    emit error(QString::fromStdString(errorMessage));
                    emit playbackCompleted(result);
                    emit playbackStopped();
                    return;
                }
                
                // Emit event executed signal for timeline highlighting
                emit eventExecuted(static_cast<int>(i));
                
                // Calculate delay to next event
                if (i + 1 < totalEvents) {
                    std::uint64_t currentTime = event.timestampMicroseconds;
                    std::uint64_t nextTime = events[i + 1].timestampMicroseconds;
                    
                    if (nextTime > currentTime) {
                        std::uint64_t delayUs = nextTime - currentTime;
                        // Apply speed adjustment
                        double currentSpeed;
                        {
                            QMutexLocker locker(&mutex_);
                            currentSpeed = speed_;
                        }
                        if (currentSpeed > 0.0) {
                            delayUs = static_cast<std::uint64_t>(delayUs / currentSpeed);
                        }
                        // Convert to milliseconds for sleep
                        unsigned long delayMs = static_cast<unsigned long>(delayUs / 1000);
                        if (delayMs > 0) {
                            msleep(delayMs);
                        }
                    }
                }
                
                // Report progress
                PlaybackProgress progress;
                progress.state = PlaybackState::Playing;
                progress.currentLoop = loop;
                progress.totalLoops = options_.loopCount;
                progress.infiniteLoops = options_.infiniteLoops;
                progress.completedEvents = i + 1;
                progress.totalEvents = totalEvents;
                emit progressChanged(progress);
            }
            
            // Loop completed
            PlaybackProgress loopProgress;
            loopProgress.state = PlaybackState::LoopCompleted;
            loopProgress.currentLoop = loop;
            loopProgress.totalLoops = options_.loopCount;
            loopProgress.infiniteLoops = options_.infiniteLoops;
            loopProgress.completedEvents = totalEvents;
            loopProgress.totalEvents = totalEvents;
            emit progressChanged(loopProgress);
            
            result.completedLoops = loop;
            result.completedEvents = totalEvents;
            
            if (!options_.infiniteLoops && loop >= options_.loopCount) {
                break;
            }
        }
        
        // Release any held keys/buttons
        backend_->releaseAll();
        
        // Final completion
        PlaybackProgress finalProgress;
        finalProgress.state = PlaybackState::Completed;
        finalProgress.currentLoop = result.completedLoops;
        finalProgress.totalLoops = options_.loopCount;
        finalProgress.infiniteLoops = options_.infiniteLoops;
        finalProgress.completedEvents = totalEvents;
        finalProgress.totalEvents = totalEvents;
        emit progressChanged(finalProgress);
        
        emit playbackCompleted(result);
        emit playbackStopped();
        
    } catch (const std::exception& e) {
        PlaybackResult result;
        result.code = PlaybackResultCode::InternalError;
        result.message = e.what();
        emit playbackError(QString::fromStdString(e.what()));
        emit error(QString::fromStdString(e.what()));
        emit playbackCompleted(result);
        emit playbackStopped();
    }
    
    backend_.reset();
}

void PlaybackThread::setRecording(Recording* recording)
{
    QMutexLocker locker(&mutex_);
    recordingPtr_ = recording;
    if (recording) {
        recording_ = *recording;
    }
}

void PlaybackThread::setSpeed(double speed)
{
    QMutexLocker locker(&mutex_);
    speed_ = speed;
}

void PlaybackThread::setDryRun(bool dryRun)
{
    QMutexLocker locker(&mutex_);
    dryRun_ = dryRun;
}

void PlaybackThread::setLooping(bool looping)
{
    QMutexLocker locker(&mutex_);
    looping_ = looping;
    options_.infiniteLoops = looping;
    if (looping) {
        options_.loopCount = 0;  // Infinite loops when looping is enabled
    } else {
        options_.loopCount = 1;  // Single playback
    }
}

void PlaybackThread::pause()
{
    requestPause();
    emit playbackPaused();
}

void PlaybackThread::resume()
{
    requestResume();
    emit playbackResumed();
}

void PlaybackThread::stop()
{
    requestCancel();
    emit playbackStopped();
}
