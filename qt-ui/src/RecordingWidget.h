#pragma once

#include "RecordingOptions.h"
#include "RecordingProgress.h"
#include "RecordingResult.h"

#include <QWidget>

class QPushButton;
class QLabel;
class QCheckBox;
class QSpinBox;
class QTimer;
class RecordingThread;

class RecordingWidget : public QWidget
{
    Q_OBJECT

public:
    explicit RecordingWidget(QWidget* parent = nullptr);
    ~RecordingWidget() override;

    bool isRecording() const;
	bool isCountingDown() const;
    bool isPaused() const;
    
    RecordingThread* recordingThread() const { return recordingThread_; }

signals:
    void recordingStarted();
    void recordingCompleted(bool hasEvents);
    void recordingCancelled();
    void statusChanged(const QString& status);

public slots:
    void startRecording();
    void stopRecording();
    void cancelRecording();
    void togglePause();

private slots:
    void onRecordingStarted();
    void onRecordingProgress(const RecordingProgress& progress);
    void onRecordingStopped(const RecordingResult& result);
    void onCountdownTick();

private:
    void setupUi();
    void updateButtonStates();
    void startCountdown();
	void cancelCountdown();
    QString formatDuration(uint64_t microseconds) const;
    
    RecordingThread* recordingThread_ = nullptr;
    QTimer* countdownTimer_ = nullptr;
    int countdownValue_ = 0;
	bool countdownActive_ = false;
    
    // UI Elements
    QPushButton* recordButton_ = nullptr;
    QPushButton* pauseButton_ = nullptr;
    QPushButton* stopButton_ = nullptr;
    QPushButton* cancelButton_ = nullptr;
    
    QCheckBox* captureMouseCheck_ = nullptr;
    QCheckBox* captureKeyboardCheck_ = nullptr;
    QCheckBox* countdownCheck_ = nullptr;
    QSpinBox* countdownSpin_ = nullptr;
    
    QLabel* statusLabel_ = nullptr;
    QLabel* eventCountLabel_ = nullptr;
    QLabel* durationLabel_ = nullptr;
    QLabel* recordingIndicator_ = nullptr;
};
