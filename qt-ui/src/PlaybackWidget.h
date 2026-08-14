#pragma once

#include "Recording.h"
#include "PlaybackController.h"
#include "PlaybackOptions.h"
#include "PlaybackProgress.h"

#include <QWidget>
#include <memory>

class QPushButton;
class QSlider;
class QLabel;
class QProgressBar;
class QComboBox;
class QSpinBox;
class QCheckBox;

class PlaybackWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PlaybackWidget(QWidget* parent = nullptr);
    ~PlaybackWidget() override;
    
    void setRecording(Recording* recording);
    
    bool isPlaying() const { return playing_; }
    bool isPaused() const { return paused_; }
    double speed() const;
    bool isDryRun() const;
    bool isLooping() const;

signals:
    void playRequested();
    void pauseRequested();
    void resumeRequested();
    void stopRequested();
    void speedChanged(double speed);
    void progressUpdated(const PlaybackProgress& progress);

public slots:
    void play();
    void pause();
    void resume();
    void stop();
    void togglePlayPause();
    
    void setPlaybackActive(bool active);
    void updateProgress(int eventIndex, int totalEvents);
    
    void onPlaybackProgress(const PlaybackProgress& progress);
    void onPlaybackCompleted();
    void onPlaybackError(const QString& message);

private:
    void setupUi();
    void updateButtonStates();
    void updateProgressDisplay(const PlaybackProgress& progress);
    
    Recording* recording_ = nullptr;
    std::unique_ptr<PlaybackController> controller_;
    
    bool playing_ = false;
    bool paused_ = false;
    
    // UI Elements
    QPushButton* playButton_ = nullptr;
    QPushButton* pauseButton_ = nullptr;
    QPushButton* stopButton_ = nullptr;
    
    QSlider* speedSlider_ = nullptr;
    QLabel* speedLabel_ = nullptr;
    
    QComboBox* loopCombo_ = nullptr;
    QSpinBox* loopSpinBox_ = nullptr;
    
    QCheckBox* dryRunCheck_ = nullptr;
    QCheckBox* alignStartCheck_ = nullptr;
    
    QProgressBar* progressBar_ = nullptr;
    QLabel* progressLabel_ = nullptr;
    QLabel* loopLabel_ = nullptr;
    QLabel* timeLabel_ = nullptr;
};
