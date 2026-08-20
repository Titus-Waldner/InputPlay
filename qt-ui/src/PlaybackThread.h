#pragma once

#include "Recording.h"
#include "PlaybackOptions.h"
#include "PlaybackProgress.h"
#include "PlaybackResult.h"
#include "PlaybackController.h"
#include "Settings.h"
#include "InputBackendType.h"

#include <functional>
#include <QThread>
#include <QMutex>
#include <memory>


class IInputBackend;

class PlaybackThread : public QThread
{
    Q_OBJECT

public:
    explicit PlaybackThread(QObject* parent = nullptr);
    ~PlaybackThread() override;
    
    void startPlayback(
        const Recording& recording,
        const Settings& settings,
        const PlaybackOptions& options,
        bool dryRun);
    
    void requestPause();
    void requestResume();
    void requestCancel();
    
    bool isPaused() const;
    bool isRunning() const;
    
    // Convenience methods for Qt GUI integration
    void setRecording(Recording* recording);
    void setSpeed(double speed);
    void setDryRun(bool dryRun);
	void setInputBackendType(
		InputBackendType backendType);
	void setAlignStart(bool alignStart);
    void setLooping(
    bool looping);

	void setLoopCount(
		int loopCount);

	void startConfiguredPlayback();

public slots:
    void pause();
    void resume();
    void stop();

signals:
    void progressChanged(const PlaybackProgress& progress);
    void playbackCompleted(const PlaybackResult& result);
    void playbackError(const QString& message);
    
    // Additional signals for GUI integration
    void playbackStarted();
    void playbackStopped();
    void playbackPaused();
    void playbackResumed();
    void eventExecuted(int eventIndex);
    void error(const QString& message);

protected:
    void run() override;

private:
	void handleEngineProgress(const PlaybackProgress& progress);
	bool waitForDelay(
		std::uint64_t delayMicroseconds);
    Recording recording_;
    Recording* recordingPtr_ = nullptr;  // Non-owning pointer for GUI mode
    Settings settings_;
    PlaybackOptions options_;
    bool dryRun_ = true;
	InputBackendType backendType_ = InputBackendType::SendInputAbsolute;
    double speed_ = 1.0;
    bool looping_ = false;
	bool alignStart_ = true;
    
    std::unique_ptr<PlaybackController> controller_;
    std::unique_ptr<IInputBackend> backend_;
    
    mutable QMutex mutex_;
    bool shouldStop_ = false;
    bool shouldPause_ = false;
};
