#include "PlaybackThread.h"
#include "PlaybackEngine.h"
#include "DryRunBackend.h"
#include "SendInputBackend.h"
#include "RecordingFile.h"

#include <QTemporaryFile>
#include <QElapsedTimer>
#include <QMutexLocker>
#include <functional>

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

bool PlaybackThread::waitForDelay(
    std::uint64_t delayMicroseconds)
{
    /*
     * Sleep in short intervals so Stop can interrupt playback even
     * when the recording contains a long delay between two events.
     */
    std::uint64_t remainingMilliseconds =
        delayMicroseconds / 1000;

    while (remainingMilliseconds > 0)
    {
        {
            QMutexLocker locker(
                &mutex_);

            if (shouldStop_
                || controller_->cancellationRequested())
            {
                return false;
            }
        }

        const unsigned long sleepMilliseconds =
            static_cast<unsigned long>(
                qMin<std::uint64_t>(
                    remainingMilliseconds,
                    10));

        msleep(
            sleepMilliseconds);

        remainingMilliseconds -=
            sleepMilliseconds;
    }

    /*
     * Preserve sub-millisecond delays where possible.
     */
    const std::uint64_t remainingMicroseconds =
        delayMicroseconds % 1000;

    if (remainingMicroseconds > 0)
    {
        usleep(
            static_cast<unsigned long>(
                remainingMicroseconds));
    }

    {
        QMutexLocker locker(
            &mutex_);

        if (shouldStop_
            || controller_->cancellationRequested())
        {
            return false;
        }
    }

    return true;
}

void PlaybackThread::run()
{
    emit playbackStarted();

    try
    {
        Recording playbackRecording =
            recording_;

        double currentSpeed;

        {
            QMutexLocker locker(
                &mutex_);

            currentSpeed =
                speed_;
        }

        if (currentSpeed <= 0.0)
        {
            currentSpeed =
                1.0;
        }

        /*
         * PlaybackEngine owns the authoritative scheduler.
         *
         * Scale a temporary copy of the recording so the GUI speed
         * control remains available without maintaining a second
         * playback implementation.
         */
        if (currentSpeed != 1.0)
        {
            Recording scaledRecording;

            if (playbackRecording
                    .hasStartingCursorPosition())
            {
                scaledRecording
                    .setStartingCursorPosition(
                        playbackRecording
                            .startingCursorX(),
                        playbackRecording
                            .startingCursorY());
            }

            if (playbackRecording
                    .hasDisplayMetadata())
            {
                scaledRecording
                    .setDisplayMetadata(
                        playbackRecording
                            .displayMetadata());
            }

            for (const InputEvent& originalEvent
                 : playbackRecording.events())
            {
                InputEvent scaledEvent =
                    originalEvent;

                scaledEvent.timestampMicroseconds =
                    static_cast<std::uint64_t>(
                        static_cast<double>(
                            originalEvent
                                .timestampMicroseconds)
                        / currentSpeed);

                scaledEvent.waitMicroseconds =
                    static_cast<std::uint64_t>(
                        static_cast<double>(
                            originalEvent
                                .waitMicroseconds)
                        / currentSpeed);

                scaledRecording.addEvent(
                    scaledEvent);
            }

            playbackRecording =
                scaledRecording;
        }

        QTemporaryFile temporaryFile;

        temporaryFile.setAutoRemove(
            true);

        if (!temporaryFile.open())
        {
            PlaybackResult result;

            result.code =
                PlaybackResultCode::InternalError;

            result.message =
                "Unable to create the temporary playback file.";

            const QString errorText =
                QString::fromStdString(
                    result.message);

            emit playbackError(
                errorText);

            emit error(
                errorText);

            emit playbackCompleted(
                result);

            emit playbackStopped();

            return;
        }

        const QString temporaryPath =
            temporaryFile.fileName();

        temporaryFile.close();

        std::string saveError;

        if (!RecordingFile::save(
                playbackRecording,
                temporaryPath.toStdString(),
                saveError))
        {
            PlaybackResult result;

            result.code =
                PlaybackResultCode::InternalError;

            result.message =
                saveError;

            const QString errorText =
                QString::fromStdString(
                    saveError);

            emit playbackError(
                errorText);

            emit error(
                errorText);

            emit playbackCompleted(
                result);

            emit playbackStopped();

            return;
        }

        if (dryRun_)
        {
            backend_ =
                std::make_unique<DryRunBackend>();
        }
        else
        {
            backend_ =
                std::make_unique<SendInputBackend>();
        }

        PlaybackOptions engineOptions =
            options_;

        /*
         * The GUI already owns the start control, so PlaybackEngine
         * must begin immediately instead of waiting for the CLI key.
         */
        engineOptions.startImmediately =
            true;

        engineOptions.alignStart =
            alignStart_;

        PlaybackCallbacks callbacks;

        callbacks.onProgress =
			std::bind(
				&PlaybackThread::handleEngineProgress,
				this,
				std::placeholders::_1);


        PlaybackResult result =
            runPlayback(
                temporaryPath.toStdString(),
                *backend_,
                settings_,
                engineOptions,
                *controller_,
                callbacks);

        backend_->releaseAll();

        if (result.code
            == PlaybackResultCode::BackendFailed
            || result.code
                == PlaybackResultCode::InternalError
            || result.code
                == PlaybackResultCode::
                    RecordingLoadFailed
            || result.code
                == PlaybackResultCode::
                    DisplayIncompatible)
        {
            const QString errorText =
                QString::fromStdString(
                    result.message);

            emit playbackError(
                errorText);

            emit error(
                errorText);
        }

        emit playbackCompleted(
            result);

        emit playbackStopped();
    }
    catch (const std::exception& exception)
    {
        PlaybackResult result;

        result.code =
            PlaybackResultCode::InternalError;

        result.message =
            exception.what();

        if (backend_)
        {
            backend_->releaseAll();
        }

        const QString errorText =
            QString::fromStdString(
                exception.what());

        emit playbackError(
            errorText);

        emit error(
            errorText);

        emit playbackCompleted(
            result);

        emit playbackStopped();
    }

    backend_.reset();
}

void PlaybackThread::handleEngineProgress(
    const PlaybackProgress& progress)
{
    emit progressChanged(
        progress);

    if (progress.totalEvents > 0
        && progress.completedEvents > 0)
    {
        emit eventExecuted(
            static_cast<int>(
                progress.completedEvents - 1));
    }
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

void PlaybackThread::setAlignStart(
    bool alignStart)
{
    QMutexLocker locker(
        &mutex_);

    alignStart_ =
        alignStart;
}

void PlaybackThread::setLooping(
    bool looping)
{
    QMutexLocker locker(
        &mutex_);

    looping_ =
        looping;

    options_.infiniteLoops =
        looping;

    if (looping)
    {
        options_.loopCount =
            0;
    }
}

void PlaybackThread::setLoopCount(
    int loopCount)
{
    QMutexLocker locker(
        &mutex_);

    if (loopCount < 1)
    {
        loopCount =
            1;
    }

    looping_ =
        false;

    options_.infiniteLoops =
        false;

    options_.loopCount =
        static_cast<unsigned int>(
            loopCount);
}

void PlaybackThread::startConfiguredPlayback()
{
    /*
     * A cancelled PlaybackController remains cancelled until reset.
     * Reset both cancellation sources before every new GUI playback.
     */
    {
        QMutexLocker locker(
            &mutex_);

        shouldStop_ =
            false;

        controller_->reset();
    }

    start();
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
    /*
     * Request cancellation here. The worker thread emits
     * playbackStopped only after it has actually exited playback.
     */
    requestCancel();
}
