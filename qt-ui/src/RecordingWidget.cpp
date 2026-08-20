#include "RecordingWidget.h"
#include "RecordingThread.h"
#include "DarkStyle.h"
#include "Recording.h"
#include "InputEvent.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

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
		
	continueButton_ =
		new QPushButton(
			tr("● Continue Recording"));

	continueButton_->setProperty(
		"primary",
		false);

	continueButton_->setMinimumHeight(
		40);

	continueButton_->setMinimumWidth(
		170);

	continueButton_->setToolTip(
		tr(
			"Move the cursor to the final recorded position "
			"and append new input events to the current recording."));

	connect(
		continueButton_,
		&QPushButton::clicked,
		this,
		&RecordingWidget::continueRecording);

	buttonLayout->addWidget(
		continueButton_);


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

    const bool busy =
        recording
        || countdownActive_;

    const bool canContinue =
        existingRecording_
        && !existingRecording_->empty();

    recordButton_->setEnabled(
        !busy);

    continueButton_->setEnabled(
        !busy
        && canContinue);

    pauseButton_->setEnabled(
        recording);

    stopButton_->setEnabled(
        busy);

    cancelButton_->setEnabled(
        busy);

    captureMouseCheck_->setEnabled(
        !busy);

    captureKeyboardCheck_->setEnabled(
        !busy);

    countdownCheck_->setEnabled(
        !busy);

    countdownSpin_->setEnabled(
        !busy
        && countdownCheck_->isChecked());

    if (!recording)
    {
        pauseButton_->setText(
            tr("⏸ Pause"));

        DarkStyle::setTone(
            recordingIndicator_,
            "secondary");
    }
    else if (paused)
    {
        pauseButton_->setText(
            tr("▶ Resume"));

        DarkStyle::setTone(
            recordingIndicator_,
            "accentLight");
    }
    else
    {
        pauseButton_->setText(
            tr("⏸ Pause"));

        DarkStyle::setTone(
            recordingIndicator_,
            "accent");
    }

    recordButton_->update();
    continueButton_->update();
    pauseButton_->update();
    stopButton_->update();
    cancelButton_->update();
}

void RecordingWidget::setExistingRecording(
    Recording* recording)
{
    existingRecording_ =
        recording;

    updateButtonStates();
}

bool RecordingWidget::isContinuingRecording() const
{
    return continueMode_;
}

void RecordingWidget::startRecording()
{
    if (countdownActive_
        || recordingThread_->isRunning())
    {
        return;
    }

    continueMode_ =
        false;

    RecordingOptions options;

    options.captureMouse =
        captureMouseCheck_->isChecked();

    options.captureKeyboard =
        captureKeyboardCheck_->isChecked();

    options.waitForStartKey =
        false;

    recordingThread_->setOptions(
        options);

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
        beginRecordingCapture();
    }
}

void RecordingWidget::continueRecording()
{
    if (countdownActive_
        || recordingThread_->isRunning())
    {
        return;
    }

    if (!existingRecording_
        || existingRecording_->empty())
    {
        statusLabel_->setText(
            tr("No existing recording to continue"));

        emit statusChanged(
            tr("No existing recording to continue"));

        return;
    }

    continueMode_ =
        true;

    RecordingOptions options;

    options.captureMouse =
        captureMouseCheck_->isChecked();

    options.captureKeyboard =
        captureKeyboardCheck_->isChecked();

    options.waitForStartKey =
        false;

    recordingThread_->setOptions(
        options);

    eventCountLabel_->setText(
        tr("Existing events: %1")
            .arg(
                existingRecording_->eventCount()));

    const std::vector<InputEvent>& events =
        existingRecording_->events();

    if (!events.empty())
    {
        durationLabel_->setText(
            tr("Duration: %1")
                .arg(
                    formatDuration(
                        events.back()
                            .timestampMicroseconds)));
    }

    if (countdownCheck_->isChecked())
    {
        startCountdown();
    }
    else
    {
        beginRecordingCapture();
    }
}


bool RecordingWidget::moveCursorToLastRecordedPosition()
{
    if (!existingRecording_
        || existingRecording_->empty())
    {
        return false;
    }

    const std::vector<InputEvent>& events =
        existingRecording_->events();

    /*
     * Prefer the final event that explicitly changes or establishes
     * the cursor position.
     *
     * Do not prefer trailing wheel events because their stored
     * coordinates may not be the best indication of the final
     * recorded cursor destination.
     */
    for (auto eventIterator =
             events.rbegin();
         eventIterator != events.rend();
         ++eventIterator)
    {
        const InputEvent& event =
            *eventIterator;

        const bool explicitPositionEvent =
            event.type
                == EventType::MouseMove
            || event.type
                == EventType::MouseTeleport;

        if (!explicitPositionEvent)
        {
            continue;
        }

#ifdef _WIN32
        return SetCursorPos(
                   event.mouseX,
                   event.mouseY)
            != FALSE;
#else
        return false;
#endif
    }

    /*
     * A click-only recording may not contain a movement event.
     * Use the final button event as a fallback in that case.
     */
    for (auto eventIterator =
             events.rbegin();
         eventIterator != events.rend();
         ++eventIterator)
    {
        const InputEvent& event =
            *eventIterator;

        const bool buttonPositionEvent =
            event.type
                == EventType::MouseButtonDown
            || event.type
                == EventType::MouseButtonUp;

        if (!buttonPositionEvent)
        {
            continue;
        }

#ifdef _WIN32
        return SetCursorPos(
                   event.mouseX,
                   event.mouseY)
            != FALSE;
#else
        return false;
#endif
    }

    /*
     * A keyboard-only recording has no final mouse position.
     * Fall back to its saved starting cursor position if available.
     */
    if (existingRecording_
            ->hasStartingCursorPosition())
    {
#ifdef _WIN32
        return SetCursorPos(
                   existingRecording_
                       ->startingCursorX(),
                   existingRecording_
                       ->startingCursorY())
            != FALSE;
#else
        return false;
#endif
    }

    return false;
}

void RecordingWidget::beginRecordingCapture()
{
    if (continueMode_)
    {
        if (!moveCursorToLastRecordedPosition())
        {
            continueMode_ =
                false;

            statusLabel_->setText(
                tr(
                    "Unable to move the cursor to the "
                    "final recorded position"));

            emit statusChanged(
                tr(
                    "Continue Recording could not restore "
                    "the final cursor position"));

            updateButtonStates();

            return;
        }
    }

    recordingThread_->startRecording();

    updateButtonStates();
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

	beginRecordingCapture();
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
