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
#include <QStyle>

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
    return countdownActive_
        || recordingThread_->isRunning();
}

bool RecordingWidget::isCountingDown() const
{
    return countdownActive_;
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
    

	// Recording controls.
	QHBoxLayout* buttonLayout =
		new QHBoxLayout();

	buttonLayout->setSpacing(
		8);

	buttonLayout->setAlignment(
		Qt::AlignLeft);

	recordButton_ =
		new QPushButton(
			tr("● Record"));

	recordButton_->setProperty(
		"primary",
		true);

	recordButton_->setMinimumHeight(
		40);

	recordButton_->setMinimumWidth(
		110);

	recordButton_->setToolTip(
		tr("Start recording"));

	connect(
		recordButton_,
		&QPushButton::clicked,
		this,
		&RecordingWidget::startRecording);

	buttonLayout->addWidget(
		recordButton_);

	pauseButton_ =
		new QPushButton(
			tr("⏸ Pause"));

	pauseButton_->setProperty(
		"primary",
		false);

	pauseButton_->setMinimumHeight(
		40);

	pauseButton_->setMinimumWidth(
		110);

	pauseButton_->setToolTip(
		tr("Pause or resume recording"));

	connect(
		pauseButton_,
		&QPushButton::clicked,
		this,
		&RecordingWidget::togglePause);

	buttonLayout->addWidget(
		pauseButton_);

	stopButton_ =
		new QPushButton(
			tr("■ Stop"));

	stopButton_->setProperty(
		"primary",
		false);

	stopButton_->setMinimumHeight(
		40);

	stopButton_->setMinimumWidth(
		110);

	stopButton_->setToolTip(
		tr("Stop and keep the recording"));

	connect(
		stopButton_,
		&QPushButton::clicked,
		this,
		&RecordingWidget::stopRecording);

	buttonLayout->addWidget(
		stopButton_);

	cancelButton_ =
		new QPushButton(
			tr("× Cancel"));

	cancelButton_->setProperty(
		"primary",
		false);

	cancelButton_->setMinimumHeight(
		40);

	cancelButton_->setMinimumWidth(
		110);

	cancelButton_->setToolTip(
		tr("Cancel and discard the recording"));

	connect(
		cancelButton_,
		&QPushButton::clicked,
		this,
		&RecordingWidget::cancelRecording);

	buttonLayout->addWidget(
		cancelButton_);

	buttonLayout->addStretch();

	mainLayout->addLayout(
		buttonLayout);
		
    
    // Hotkey hints
    QLabel* hotkeyHint =
    new QLabel(
        tr(
            "Tip: F9 starts/stops and F10 pauses/resumes "
			"the active workspace"
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

    const bool countdown =
        countdownActive_;

    const bool active =
        recording
        || countdown;

    /*
     * Ready:
     *   Record is highlighted and enabled.
     *
     * Countdown:
     *   Cancel is highlighted and enabled.
     *
     * Recording:
     *   Pause, Stop, and Cancel are highlighted and enabled.
     *
     * Paused:
     *   Resume, Stop, and Cancel are highlighted and enabled.
     */
    recordButton_->setProperty(
        "primary",
        !active);

    pauseButton_->setProperty(
        "primary",
        recording);

    stopButton_->setProperty(
        "primary",
        recording);

    cancelButton_->setProperty(
        "primary",
        active);

    // Apply changed dynamic properties to the stylesheet.
    recordButton_->style()->unpolish(
        recordButton_);

    recordButton_->style()->polish(
        recordButton_);

    pauseButton_->style()->unpolish(
        pauseButton_);

    pauseButton_->style()->polish(
        pauseButton_);

    stopButton_->style()->unpolish(
        stopButton_);

    stopButton_->style()->polish(
        stopButton_);

    cancelButton_->style()->unpolish(
        cancelButton_);

    cancelButton_->style()->polish(
        cancelButton_);

    /*
     * Countdown is cancellable, but it cannot be paused or stopped
     * as a completed recording because capture has not started yet.
     */
    recordButton_->setEnabled(
        !active);

    pauseButton_->setEnabled(
        recording);

    stopButton_->setEnabled(
        recording);

    cancelButton_->setEnabled(
        active);

    recordButton_->setText(
        tr("● Record"));

    if (paused)
    {
        pauseButton_->setText(
            tr("▶ Resume"));

        pauseButton_->setToolTip(
            tr("Resume recording"));
    }
    else
    {
        pauseButton_->setText(
            tr("⏸ Pause"));

        pauseButton_->setToolTip(
            tr("Pause recording"));
    }

    stopButton_->setText(
        tr("■ Stop"));

    cancelButton_->setText(
        countdown
        ? tr("× Cancel Countdown")
        : tr("× Cancel"));

    if (countdown)
    {
        cancelButton_->setToolTip(
            tr("Cancel the recording countdown"));
    }
    else
    {
        cancelButton_->setToolTip(
            tr("Cancel and discard the recording"));
    }

    captureMouseCheck_->setEnabled(
        !active);

    captureKeyboardCheck_->setEnabled(
        !active);

    countdownCheck_->setEnabled(
        !active);

    countdownSpin_->setEnabled(
        !active
        && countdownCheck_->isChecked());

    if (countdown)
    {
        DarkStyle::setTone(
            recordingIndicator_,
            "accentLight");
    }
    else if (!recording)
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

    recordButton_->update();
    pauseButton_->update();
    stopButton_->update();
    cancelButton_->update();
}

void RecordingWidget::startRecording()
{
    if (countdownActive_
        || recordingThread_->isRunning())
    {
        return;
    }

    // Apply the selected recording options.
    RecordingOptions options;

    options.captureMouse =
        captureMouseCheck_->isChecked();

    options.captureKeyboard =
        captureKeyboardCheck_->isChecked();

    // The GUI controls when capture begins.
    options.waitForStartKey = false;

    recordingThread_->setOptions(
        options);

    // Reset the recording display.
    eventCountLabel_->setText(
        tr("Events: 0"));

    durationLabel_->setText(
        tr("Duration: 0:00.000"));

    if (countdownCheck_->isChecked())
    {
        startCountdown();
    }
    else
    {
        recordingThread_->startRecording();
        updateButtonStates();
    }
}

void RecordingWidget::startCountdown()
{
    countdownValue_ =
        countdownSpin_->value();

    countdownActive_ =
        true;

    statusLabel_->setText(
        tr("Starting in %1...")
            .arg(
                countdownValue_));

    emit statusChanged(
        tr("Recording starts in %1 seconds")
            .arg(
                countdownValue_));

    updateButtonStates();

    countdownTimer_->start(
        1000);
}

void RecordingWidget::cancelCountdown()
{
    if (!countdownActive_)
    {
        return;
    }

    countdownTimer_->stop();

    countdownActive_ =
        false;

    countdownValue_ =
        0;

    statusLabel_->setText(
        tr("Recording cancelled"));

    updateButtonStates();

    emit recordingCancelled();

    emit statusChanged(
        tr("Recording cancelled"));
}

void RecordingWidget::onCountdownTick()
{
    if (!countdownActive_)
    {
        countdownTimer_->stop();
        return;
    }

    --countdownValue_;

    if (countdownValue_ > 0)
    {
        statusLabel_->setText(
            tr("Starting in %1...")
                .arg(
                    countdownValue_));

        emit statusChanged(
            tr("Recording starts in %1 seconds")
                .arg(
                    countdownValue_));

        return;
    }

    countdownTimer_->stop();

    countdownActive_ =
        false;

    countdownValue_ =
        0;

    statusLabel_->setText(
        tr("Recording..."));

    recordingThread_->startRecording();

    updateButtonStates();
}

void RecordingWidget::stopRecording()
{
    /*
     * A countdown has not created a recording yet, so stopping during
     * countdown is equivalent to cancelling the countdown.
     */
    if (countdownActive_)
    {
        cancelCountdown();
        return;
    }

    if (!recordingThread_->isRunning())
    {
        return;
    }

    recordingThread_->stopRecording();
}

void RecordingWidget::cancelRecording()
{
    if (countdownActive_)
    {
        cancelCountdown();
        return;
    }

    if (!recordingThread_->isRunning())
    {
        return;
    }

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

void RecordingWidget::onRecordingStopped(
    const RecordingResult& result)
{
    countdownTimer_->stop();

    countdownActive_ =
        false;

    countdownValue_ =
        0;

    updateButtonStates();

    // Keep the remainder of the existing function here.    
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
