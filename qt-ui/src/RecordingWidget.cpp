#include "RecordingWidget.h"
#include "RecordingThread.h"
#include "DarkStyle.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QPushButton>
#include <QLabel>
#include <QCheckBox>
#include <QSpinBox>
#include <QGroupBox>
#include <QTimer>

RecordingWidget::RecordingWidget(QWidget* parent)
    : QWidget(parent)
    , recordingThread_(new RecordingThread(this))
    , countdownTimer_(new QTimer(this))
{
    setupUi();
    
    connect(recordingThread_, &RecordingThread::recordingStarted,
            this, &RecordingWidget::onRecordingStarted);
    connect(recordingThread_, &RecordingThread::recordingProgress,
            this, &RecordingWidget::onRecordingProgress);
    connect(recordingThread_, &RecordingThread::recordingStopped,
            this, &RecordingWidget::onRecordingStopped);
    
    connect(countdownTimer_, &QTimer::timeout, this, &RecordingWidget::onCountdownTick);
    
    updateButtonStates();
}

RecordingWidget::~RecordingWidget()
{
    if (recordingThread_->isRunning()) {
        recordingThread_->cancelRecording();
        recordingThread_->wait(3000);
    }
}

bool RecordingWidget::isRecording() const
{
    return recordingThread_->isRunning();
}

bool RecordingWidget::isPaused() const
{
    return recordingThread_->isPaused();
}

void RecordingWidget::setupUi()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(12, 12, 12, 12);
    mainLayout->setSpacing(12);
    
    // Recording indicator and status
    QHBoxLayout* statusLayout = new QHBoxLayout();
    
	recordingIndicator_ =
		new QLabel("●");

	recordingIndicator_->setProperty(
		"recordingIndicator",
		true);

	DarkStyle::setTone(
		recordingIndicator_,
		"secondary");

	statusLayout->addWidget(
		recordingIndicator_);

	statusLabel_ =
		new QLabel(
			tr("Ready to record"));

	statusLabel_->setProperty(
		"statusHeading",
		true);

	DarkStyle::setTone(
		statusLabel_,
		"accent");

	statusLayout->addWidget(
		statusLabel_,
		1);
    
    mainLayout->addLayout(statusLayout);
    
    // Statistics row
    QHBoxLayout* statsLayout = new QHBoxLayout();
    
    eventCountLabel_ = new QLabel(tr("Events: 0"));
    statsLayout->addWidget(eventCountLabel_);
    
    statsLayout->addStretch();
    
    durationLabel_ = new QLabel(tr("Duration: 0:00.000"));
    statsLayout->addWidget(durationLabel_);
    
    mainLayout->addLayout(statsLayout);
    
    // Options group
    QGroupBox* optionsGroup = new QGroupBox(tr("Recording Options"));
    QVBoxLayout* optionsLayout = new QVBoxLayout(optionsGroup);
    
    captureMouseCheck_ = new QCheckBox(tr("Capture mouse input"));
    captureMouseCheck_->setChecked(true);
    optionsLayout->addWidget(captureMouseCheck_);
    
    captureKeyboardCheck_ = new QCheckBox(tr("Capture keyboard input"));
    captureKeyboardCheck_->setChecked(true);
    optionsLayout->addWidget(captureKeyboardCheck_);
    
    QHBoxLayout* countdownLayout = new QHBoxLayout();
    countdownCheck_ = new QCheckBox(tr("Countdown before recording:"));
    countdownCheck_->setChecked(true);
    countdownLayout->addWidget(countdownCheck_);
    
    countdownSpin_ = new QSpinBox();
    countdownSpin_->setRange(1, 10);
    countdownSpin_->setValue(3);
    countdownSpin_->setSuffix(tr(" seconds"));
    countdownLayout->addWidget(countdownSpin_);
    countdownLayout->addStretch();
    
    connect(countdownCheck_, &QCheckBox::toggled, countdownSpin_, &QSpinBox::setEnabled);
    
    optionsLayout->addLayout(countdownLayout);
    
    mainLayout->addWidget(optionsGroup);
    
    // Control buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(8);
    buttonLayout->setAlignment(Qt::AlignLeft);
    
    recordButton_ =
    new QPushButton(
        tr("⏺"));

	recordButton_->setProperty(
		"primary",
		true);

	recordButton_->setMinimumHeight(
		40);

	recordButton_->setFixedWidth(
		50);

	recordButton_->setToolTip(
		tr("Start Recording"));

	connect(
		recordButton_,
		&QPushButton::clicked,
		this,
		&RecordingWidget::startRecording);

	buttonLayout->addWidget(
		recordButton_);
    
    pauseButton_ = new QPushButton(tr("⏸"));
    pauseButton_->setMinimumHeight(40);
    pauseButton_->setFixedWidth(50);
    pauseButton_->setToolTip(tr("Pause/Resume Recording"));
    connect(pauseButton_, &QPushButton::clicked, this, &RecordingWidget::togglePause);
    buttonLayout->addWidget(pauseButton_);
    
    stopButton_ =
		new QPushButton(
			tr("⏹"));

	stopButton_->setProperty(
		"primary",
		true);

	stopButton_->setMinimumHeight(
		40);

	stopButton_->setFixedWidth(
		50);

	stopButton_->setToolTip(
		tr("Stop Recording"));

	connect(
		stopButton_,
		&QPushButton::clicked,
		this,
		&RecordingWidget::stopRecording);

	buttonLayout->addWidget(
		stopButton_);
    
    cancelButton_ = new QPushButton(tr("✕"));
    cancelButton_->setMinimumHeight(40);
    cancelButton_->setFixedWidth(50);
    cancelButton_->setToolTip(tr("Cancel Recording"));
    connect(cancelButton_, &QPushButton::clicked, this, &RecordingWidget::cancelRecording);
    buttonLayout->addWidget(cancelButton_);
    
    mainLayout->addLayout(buttonLayout);
    
    // Hotkey hints
    QLabel* hotkeyHint =
    new QLabel(
        tr(
            "Tip: Use F9 to start/stop, "
            "F10 to pause "
            "(when enabled in Settings)"));

	hotkeyHint->setProperty(
		"subheading",
		true);

	hotkeyHint->setProperty(
		"italic",
		true);

	mainLayout->addWidget(
		hotkeyHint);
    
    mainLayout->addStretch();
}

void RecordingWidget::updateButtonStates()
{
    const bool recording =
        recordingThread_->isRunning();

    const bool paused =
        recordingThread_->isPaused();

    recordButton_->setEnabled(
        !recording);

    pauseButton_->setEnabled(
        recording);

    pauseButton_->setText(
        paused
        ? tr("▶")
        : tr("⏸"));

    stopButton_->setEnabled(
        recording);

    cancelButton_->setEnabled(
        recording);

    captureMouseCheck_->setEnabled(
        !recording);

    captureKeyboardCheck_->setEnabled(
        !recording);

    countdownCheck_->setEnabled(
        !recording);

    countdownSpin_->setEnabled(
        !recording
        && countdownCheck_->isChecked());

    if (!recording)
    {
        DarkStyle::setTone(
            recordingIndicator_,
            "secondary");
    }
    else if (paused)
    {
        DarkStyle::setTone(
            recordingIndicator_,
            "accentLight");
    }
    else
    {
        DarkStyle::setTone(
            recordingIndicator_,
            "accent");
    }
}

void RecordingWidget::startRecording()
{
    if (recordingThread_->isRunning()) {
        return;
    }
    
    // Set recording options
    RecordingOptions options;
    options.captureMouse = captureMouseCheck_->isChecked();
    options.captureKeyboard = captureKeyboardCheck_->isChecked();
    options.waitForStartKey = false; // GUI handles start trigger
    
    recordingThread_->setOptions(options);
    
    // Reset display
    eventCountLabel_->setText(tr("Events: 0"));
    durationLabel_->setText(tr("Duration: 0:00.000"));
    
    if (countdownCheck_->isChecked()) {
        startCountdown();
    } else {
        recordingThread_->startRecording();
        updateButtonStates();
    }
}

void RecordingWidget::startCountdown()
{
    countdownValue_ =
    countdownSpin_->value();

	statusLabel_->setText(
		tr("Starting in %1...")
			.arg(
				countdownValue_));

	DarkStyle::setTone(
		recordingIndicator_,
		"accentLight");
		
	recordButton_->setEnabled(false);
		
    captureMouseCheck_->setEnabled(false);
    captureKeyboardCheck_->setEnabled(false);
    countdownCheck_->setEnabled(false);
    countdownSpin_->setEnabled(false);
    
    countdownTimer_->start(1000);
}

void RecordingWidget::onCountdownTick()
{
    countdownValue_--;
    
    if (countdownValue_ > 0) {
        statusLabel_->setText(tr("Starting in %1...").arg(countdownValue_));
    } else {
        countdownTimer_->stop();
        statusLabel_->setText(tr("Recording..."));
        recordingThread_->startRecording();
        updateButtonStates();
    }
}

void RecordingWidget::stopRecording()
{
    countdownTimer_->stop();
    recordingThread_->stopRecording();
}

void RecordingWidget::cancelRecording()
{
    countdownTimer_->stop();
    recordingThread_->cancelRecording();
}

void RecordingWidget::togglePause()
{
    recordingThread_->togglePause();
    updateButtonStates();
    
    if (recordingThread_->isPaused()) {
        statusLabel_->setText(tr("Paused"));
        emit statusChanged(tr("Recording paused"));
    } else {
        statusLabel_->setText(tr("Recording..."));
        emit statusChanged(tr("Recording resumed"));
    }
}

void RecordingWidget::onRecordingStarted()
{
    statusLabel_->setText(tr("Recording..."));
    updateButtonStates();
    emit recordingStarted();
    emit statusChanged(tr("Recording started"));
}

void RecordingWidget::onRecordingProgress(const RecordingProgress& progress)
{
    eventCountLabel_->setText(tr("Events: %1").arg(progress.eventCount));
    durationLabel_->setText(tr("Duration: %1").arg(formatDuration(progress.elapsedMicroseconds)));
    
    switch (progress.state) {
        case RecordingState::Recording:
            statusLabel_->setText(tr("Recording..."));
            break;
        case RecordingState::Paused:
            statusLabel_->setText(tr("Paused"));
            break;
        case RecordingState::Armed:
            statusLabel_->setText(tr("Armed - waiting for input..."));
            break;
        default:
            break;
    }
    
    updateButtonStates();
}

void RecordingWidget::onRecordingStopped(const RecordingResult& result)
{
    updateButtonStates();
    
    if (result.code == RecordingResultCode::Completed) {
        statusLabel_->setText(tr("Recording completed - %1 events").arg(result.eventCount));
        emit recordingCompleted(result.eventCount > 0);
        emit statusChanged(tr("Recording completed: %1 events").arg(result.eventCount));
    } else if (result.code == RecordingResultCode::Cancelled) {
        statusLabel_->setText(tr("Recording cancelled"));
        emit recordingCancelled();
        emit statusChanged(tr("Recording cancelled"));
    } else {
        statusLabel_->setText(tr("Recording failed: %1").arg(QString::fromStdString(result.message)));
        emit statusChanged(tr("Recording failed"));
    }
}

QString RecordingWidget::formatDuration(uint64_t microseconds) const
{
    uint64_t totalSeconds = microseconds / 1'000'000;
    uint64_t minutes = totalSeconds / 60;
    uint64_t seconds = totalSeconds % 60;
    uint64_t millis = (microseconds / 1000) % 1000;
    
    return QString("%1:%2.%3")
        .arg(minutes)
        .arg(seconds, 2, 10, QChar('0'))
        .arg(millis, 3, 10, QChar('0'));
}
