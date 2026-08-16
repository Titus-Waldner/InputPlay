#include "PlaybackThread.h"
#include "PlaybackEngine.h"
#include "DryRunBackend.h"
#include "SendInputBackend.h"

#include <QElapsedTimer>
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

    /*
     * Align the cursor before playback begins.
     */
    if (alignStart_
        && recording_.hasStartingCursorPosition())
    {
        InputEvent alignEvent;

        alignEvent.type =
            EventType::MouseTeleport;

        alignEvent.mouseX =
            recording_.startingCursorX();

        alignEvent.mouseY =
            recording_.startingCursorY();

        std::string errorMessage;

        if (!backend_->execute(
                alignEvent,
                errorMessage))
        {
            PlaybackResult result;

            result.code =
                PlaybackResultCode::BackendFailed;

            result.message =
                errorMessage;

            const QString errorText =
                QString::fromStdString(
                    errorMessage);

            emit playbackError(
                errorText);

            emit error(
                errorText);

            emit playbackCompleted(
                result);

            emit playbackStopped();

            backend_.reset();

            return;
        }
    }

    try
    {
        PlaybackResult result;

        result.code =
            PlaybackResultCode::Completed;

        const auto& events =
            recording_.events();

        const std::size_t totalEvents =
            events.size();

        for (unsigned int loop = 1;
             options_.infiniteLoops
                 || loop <= options_.loopCount;
             ++loop)
        {
            {
                QMutexLocker locker(
                    &mutex_);

                if (shouldStop_
                    || controller_->
                        cancellationRequested())
                {
                    result.code =
                        PlaybackResultCode::Cancelled;

                    result.completedLoops =
                        loop - 1;

                    backend_->releaseAll();

                    emit playbackCompleted(
                        result);

                    emit playbackStopped();

                    backend_.reset();

                    return;
                }
            }

            /*
             * Use one absolute clock for the entire loop.
             * This prevents timing errors from accumulating.
             */
            QElapsedTimer playbackClock;

            playbackClock.start();

            std::uint64_t addedWaitMicroseconds =
                0;

            std::uint64_t pausedMicroseconds =
                0;

            for (std::size_t index = 0;
                 index < totalEvents;
                 ++index)
            {
                const InputEvent& event =
                    events[index];

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
                 * Calculate this event's absolute playback time.
                 *
                 * Explicit Wait events extend the timestamps of all
                 * events that follow them.
                 */
                const std::uint64_t logicalTarget =
                    event.timestampMicroseconds
                    + addedWaitMicroseconds;

                const std::uint64_t scaledTarget =
                    static_cast<std::uint64_t>(
                        static_cast<double>(
                            logicalTarget)
                        / currentSpeed);

                /*
                 * Wait until the absolute target time.
                 */
                while (true)
                {
                    {
                        QMutexLocker locker(
                            &mutex_);

                        if (shouldStop_
                            || controller_->
                                cancellationRequested())
                        {
                            result.code =
                                PlaybackResultCode::Cancelled;

                            result.completedLoops =
                                loop - 1;

                            result.completedEvents =
                                index;

                            backend_->releaseAll();

                            PlaybackProgress progress;

                            progress.state =
                                PlaybackState::Cancelled;

                            progress.currentLoop =
                                loop;

                            progress.totalLoops =
                                options_.loopCount;

                            progress.infiniteLoops =
                                options_.infiniteLoops;

                            progress.completedEvents =
                                index;

                            progress.totalEvents =
                                totalEvents;

                            emit progressChanged(
                                progress);

                            emit playbackCompleted(
                                result);

                            emit playbackStopped();

                            backend_.reset();

                            return;
                        }
                    }

                    /*
                     * Paused time must not advance playback.
                     */
                    if (controller_->paused())
                    {
                        QElapsedTimer pauseClock;

                        pauseClock.start();

                        while (controller_->paused())
                        {
                            {
                                QMutexLocker locker(
                                    &mutex_);

                                if (shouldStop_
                                    || controller_->
                                        cancellationRequested())
                                {
                                    result.code =
                                        PlaybackResultCode::
                                            Cancelled;

                                    result.completedLoops =
                                        loop - 1;

                                    result.completedEvents =
                                        index;

                                    backend_->releaseAll();

                                    emit playbackCompleted(
                                        result);

                                    emit playbackStopped();

                                    backend_.reset();

                                    return;
                                }
                            }

                            msleep(
                                10);
                        }

                        const qint64 pauseNanoseconds =
                            pauseClock.nsecsElapsed();

                        if (pauseNanoseconds > 0)
                        {
                            pausedMicroseconds +=
                                static_cast<std::uint64_t>(
                                    pauseNanoseconds
                                    / 1000);
                        }

                        continue;
                    }

                    const qint64 elapsedNanoseconds =
                        playbackClock.nsecsElapsed();

                    const std::uint64_t elapsedMicroseconds =
                        elapsedNanoseconds > 0
                        ? static_cast<std::uint64_t>(
                            elapsedNanoseconds
                            / 1000)
                        : 0;

                    const std::uint64_t targetMicroseconds =
                        scaledTarget
                        + pausedMicroseconds;

                    if (elapsedMicroseconds
                        >= targetMicroseconds)
                    {
                        break;
                    }

                    const std::uint64_t remaining =
                        targetMicroseconds
                        - elapsedMicroseconds;

                    if (remaining > 2000)
                    {
                        const std::uint64_t sleepMs =
                            qMin<std::uint64_t>(
                                remaining / 1000,
                                10);

                        msleep(
                            static_cast<unsigned long>(
                                sleepMs));
                    }
                    else
                    {
                        usleep(
                            static_cast<unsigned long>(
                                qMin<std::uint64_t>(
                                    remaining,
                                    1000)));
                    }
                }

                /*
                 * Wait events are handled by the scheduler.
                 * They must not be passed to the input backend.
                 */
                if (event.type == EventType::Wait)
                {
                    addedWaitMicroseconds +=
                        event.waitMicroseconds;
                }
                else
                {
                    std::string errorMessage;

                    if (!backend_->execute(
                            event,
                            errorMessage))
                    {
                        result.code =
                            PlaybackResultCode::BackendFailed;

                        result.message =
                            errorMessage;

                        result.completedLoops =
                            loop - 1;

                        result.completedEvents =
                            index;

                        backend_->releaseAll();

                        const QString errorText =
                            QString::fromStdString(
                                errorMessage);

                        emit playbackError(
                            errorText);

                        emit error(
                            errorText);

                        emit playbackCompleted(
                            result);

                        emit playbackStopped();

                        backend_.reset();

                        return;
                    }
                }

                emit eventExecuted(
                    static_cast<int>(
                        index));

                PlaybackProgress progress;

                progress.state =
                    PlaybackState::Playing;

                progress.currentLoop =
                    loop;

                progress.totalLoops =
                    options_.loopCount;

                progress.infiniteLoops =
                    options_.infiniteLoops;

                progress.completedEvents =
                    index + 1;

                progress.totalEvents =
                    totalEvents;

                emit progressChanged(
                    progress);
            }

            result.completedLoops =
                loop;

            result.completedEvents =
                totalEvents;

            PlaybackProgress loopProgress;

            loopProgress.state =
                PlaybackState::LoopCompleted;

            loopProgress.currentLoop =
                loop;

            loopProgress.totalLoops =
                options_.loopCount;

            loopProgress.infiniteLoops =
                options_.infiniteLoops;

            loopProgress.completedEvents =
                totalEvents;

            loopProgress.totalEvents =
                totalEvents;

            emit progressChanged(
                loopProgress);

            if (!options_.infiniteLoops
                && loop >= options_.loopCount)
            {
                break;
            }
        }

        backend_->releaseAll();

        PlaybackProgress finalProgress;

        finalProgress.state =
            PlaybackState::Completed;

        finalProgress.currentLoop =
            result.completedLoops;

        finalProgress.totalLoops =
            options_.loopCount;

        finalProgress.infiniteLoops =
            options_.infiniteLoops;

        finalProgress.completedEvents =
            totalEvents;

        finalProgress.totalEvents =
            totalEvents;

        emit progressChanged(
            finalProgress);

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
