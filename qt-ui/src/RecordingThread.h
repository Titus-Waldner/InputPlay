#pragma once

#include "Recording.h"
#include "RecordingController.h"
#include "RecordingOptions.h"
#include "RecordingProgress.h"
#include "RecordingResult.h"

#include <QThread>
#include <memory>

class RecordingThread : public QThread
{
    Q_OBJECT

public:
    explicit RecordingThread(QObject* parent = nullptr);
    ~RecordingThread() override;

    void setOptions(const RecordingOptions& options);
    
    Recording* recording() const { return recording_.get(); }
    std::unique_ptr<Recording> takeRecording();
    
    bool isRecording() const;
    bool isPaused() const;

signals:
    void recordingStarted();
    void recordingProgress(const RecordingProgress& progress);
    void recordingPaused();
    void recordingResumed();
    void recordingStopped(const RecordingResult& result);
    void recordingError(const QString& message);

public slots:
    void startRecording();
    void pauseRecording();
    void resumeRecording();
    void togglePause();
    void stopRecording();
    void cancelRecording();

protected:
    void run() override;

private:
    std::unique_ptr<Recording> recording_;
    RecordingController controller_;
    RecordingOptions options_;
    bool running_ = false;
};
