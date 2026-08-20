#include "PlaybackWidget.h"
#include "DarkStyle.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QSizePolicy>
#include <QPushButton>
#include <QSlider>
#include <QLabel>
#include <QProgressBar>
#include <QComboBox>
#include <QSpinBox>
#include <QCheckBox>
#include <QGroupBox>
#include <QFrame>
#include <QSettings>

PlaybackWidget::PlaybackWidget(QWidget* parent)
    : QWidget(parent)
    , controller_(std::make_unique<PlaybackController>())
{
    setupUi();
    updateButtonStates();
}

PlaybackWidget::~PlaybackWidget() = default;

void PlaybackWidget::setupUi()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(8, 8, 8, 8);
    mainLayout->setSpacing(8);
    
    // Control buttons row
    // Playback controls use two rows so the controls cannot overlap.
	QVBoxLayout* controlsLayout =
		new QVBoxLayout();

	controlsLayout->setContentsMargins(
		0,
		0,
		0,
		0);

	controlsLayout->setSpacing(8);

	QHBoxLayout* transportLayout =
		new QHBoxLayout();

	transportLayout->setContentsMargins(
		0,
		0,
		0,
		0);

	transportLayout->setSpacing(6);

	QHBoxLayout* optionsLayout =
		new QHBoxLayout();

	optionsLayout->setContentsMargins(
		0,
		0,
		0,
		0);

	optionsLayout->setSpacing(8);
    playButton_ = new QPushButton(tr("▶ Play"));
    playButton_->setProperty("primary", true);
    playButton_->setMinimumWidth(100);
    playButton_->setToolTip(tr("Start playback (Space)"));
    connect(playButton_, &QPushButton::clicked, this, &PlaybackWidget::play);
    transportLayout->addWidget(
		playButton_);
    
    pauseButton_ = new QPushButton(tr("⏸ Pause"));
    pauseButton_->setMinimumWidth(90);
    pauseButton_->setToolTip(tr("Pause playback"));
    connect(pauseButton_, &QPushButton::clicked, this, &PlaybackWidget::pause);
    transportLayout->addWidget(
		pauseButton_);
    
    stopButton_ = new QPushButton(tr("⏹ Stop"));
    stopButton_->setMinimumWidth(80);
    stopButton_->setToolTip(tr("Stop playback"));
    connect(stopButton_, &QPushButton::clicked, this, &PlaybackWidget::stop);
    transportLayout->addWidget(
		stopButton_);
    
    transportLayout->addSpacing(12);
	
	transportLayout->addStretch();

   
    
	QLabel* inputMethodLabel =
		new QLabel(
			tr("Input Method:"));

	transportLayout->addWidget(
		inputMethodLabel);

	inputMethodCombo_ =
		new QComboBox(
			this);

	inputMethodCombo_->addItem(
		tr("Windows - Exact Position"),
		static_cast<int>(
			InputBackendType::
				SendInputAbsolute));

	inputMethodCombo_->addItem(
		tr("Windows - Corrected Relative"),
		static_cast<int>(
			InputBackendType::
				SendInputCorrectedRelative));

	inputMethodCombo_->addItem(
		tr("Virtual HID - Native Relative"),
		static_cast<int>(
			InputBackendType::
				VhfNativeRelative));

	inputMethodCombo_->addItem(
		tr("Virtual HID - Corrected Relative (Experimental)"),
		static_cast<int>(
			InputBackendType::
				VhfCorrectedRelative));

	inputMethodCombo_->addItem(
		tr("Virtual HID - Exact Position"),
		static_cast<int>(
			InputBackendType::
				VhfAbsolute));

	inputMethodCombo_->setCurrentIndex(
		0);

	inputMethodCombo_->setMinimumWidth(
		260);

	inputMethodCombo_->setMaximumWidth(
		340);

	inputMethodCombo_->setToolTip(
		tr(
			"Selects how recorded mouse and keyboard events "
			"are submitted during playback."));

	connect(
		inputMethodCombo_,
		QOverload<int>::of(
			&QComboBox::currentIndexChanged),
		this,
		&PlaybackWidget::onInputMethodChanged);

	transportLayout->addWidget(
		inputMethodCombo_);

	transportLayout->addSpacing(
		12);
	
	QLabel* loopTitleLabel =
		new QLabel(
			tr("Loops:"));

	optionsLayout->addWidget(
		loopTitleLabel);

	loopCombo_ =
		new QComboBox(
			this);

	loopCombo_->addItem(
		tr("1"),
		1);

	loopCombo_->addItem(
		tr("2"),
		2);

	loopCombo_->addItem(
		tr("3"),
		3);

	loopCombo_->addItem(
		tr("5"),
		5);

	loopCombo_->addItem(
		tr("10"),
		10);

	loopCombo_->addItem(
		tr("Custom..."),
		-1);

	loopCombo_->addItem(
		tr("∞ Infinite"),
		0);

	loopCombo_->setMinimumWidth(
		120);

	loopCombo_->setMaximumWidth(
		160);

	optionsLayout->addWidget(
		loopCombo_);

	loopSpinBox_ =
		new QSpinBox(
			this);

	loopSpinBox_->setMinimum(
		1);

	loopSpinBox_->setMaximum(
		9999);

	loopSpinBox_->setValue(
		1);

	loopSpinBox_->setVisible(
		false);

	loopSpinBox_->setMinimumWidth(
		90);

	loopSpinBox_->setMaximumWidth(
		120);

	optionsLayout->addWidget(
		loopSpinBox_);

	connect(
		loopCombo_,
		QOverload<int>::of(
			&QComboBox::currentIndexChanged),
		this,
		&PlaybackWidget::onLoopSelectionChanged);

	optionsLayout->addSpacing(
		12);
	
    // Mode checkboxes
    dryRunCheck_ = new QCheckBox(tr("Dry Run"));
    dryRunCheck_->setChecked(false);
    dryRunCheck_->setToolTip(tr("Simulate playback without sending actual input (safe mode)"));
    optionsLayout->addWidget(
		dryRunCheck_);
    
    alignStartCheck_ = new QCheckBox(tr("Align Start"));
    alignStartCheck_->setToolTip(tr("Move cursor to starting position before playback"));
    optionsLayout->addWidget(
		alignStartCheck_);
		
	alignStartCheck_->setChecked(true);
		
		blockPhysicalMouseCheck_ =
		new QCheckBox(
			tr("Block physical mouse input"),
			this);

	blockPhysicalMouseCheck_->setChecked(
		true);

	blockPhysicalMouseCheck_->setToolTip(
		tr(
			"Prevents physical mouse movement, clicks, and scrolling "
			"from interfering with real playback. "
			"F9 and Emergency Stop remain available."));

	blockPhysicalMouseCheck_->setEnabled(
		!dryRunCheck_->isChecked());

	connect(
		dryRunCheck_,
		&QCheckBox::toggled,
		this,
		&PlaybackWidget::onDryRunChanged);

	optionsLayout->addWidget(
		blockPhysicalMouseCheck_);
	
    optionsLayout->addStretch();

	controlsLayout->addLayout(
		transportLayout);

	controlsLayout->addLayout(
		optionsLayout);

	mainLayout->addLayout(
		controlsLayout);
    
    // Separator
    QFrame* separator = new QFrame();
    separator->setFrameShape(QFrame::HLine);
    mainLayout->addWidget(separator);
    
    // Progress row
    QHBoxLayout* progressLayout = new QHBoxLayout();
    progressLayout->setSpacing(12);
    
    progressBar_ = new QProgressBar();
    progressBar_->setMinimum(0);
    progressBar_->setMaximum(100);
    progressBar_->setValue(0);
    progressBar_->setTextVisible(false);
    progressBar_->setMinimumWidth(200);
    progressLayout->addWidget(progressBar_, 1);
    
    progressLabel_ = new QLabel(tr("Ready"));
    progressLabel_->setMinimumWidth(100);
    progressLayout->addWidget(progressLabel_);
    
    loopLabel_ = new QLabel(tr(""));
    loopLabel_->setMinimumWidth(80);
    progressLayout->addWidget(loopLabel_);
    
    timeLabel_ = new QLabel(tr("0:00 / 0:00"));
    timeLabel_->setMinimumWidth(100);
    timeLabel_->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    progressLayout->addWidget(timeLabel_);
    
    mainLayout->addLayout(progressLayout);
    
    setMinimumHeight(
		145);

	setMaximumHeight(
		165);
		
	restoreInputMethod();
}

void PlaybackWidget::restoreInputMethod()
{
    QSettings settings(
        "InputPlay",
        "Studio");

    const int savedBackendValue =
        settings.value(
            "playback/inputBackend",
            static_cast<int>(
                InputBackendType::
                    SendInputAbsolute))
            .toInt();

    int selectedIndex =
        inputMethodCombo_->findData(
            savedBackendValue);

    if (selectedIndex < 0)
    {
        selectedIndex =
            inputMethodCombo_->findData(
                static_cast<int>(
                    InputBackendType::
                        SendInputAbsolute));
    }

    inputMethodCombo_->setCurrentIndex(
        selectedIndex);

    onInputMethodChanged(
        selectedIndex);
}

void PlaybackWidget::onLoopSelectionChanged(
    int index)
{
    if (!loopCombo_
        || !loopSpinBox_)
    {
        return;
    }

    const int selectedValue =
        loopCombo_->itemData(
            index).toInt();

    loopSpinBox_->setVisible(
        selectedValue == -1);
}



void PlaybackWidget::setRecording(Recording* recording)
{
    recording_ = recording;
    
    // Reset state
    playing_ = false;
    paused_ = false;
    progressBar_->setValue(0);
    
    if (recording_ && !recording_->empty()) {
        progressLabel_->setText(tr("Ready - %1 events").arg(recording_->eventCount()));
    } else {
        progressLabel_->setText(tr("No macro loaded"));
    }
    
    timeLabel_->setText(tr("0:00 / 0:00"));
    loopLabel_->clear();
    
    updateButtonStates();
}

void PlaybackWidget::play()
{
    if (!recording_ || recording_->empty()) {
        return;
    }
    
    if (paused_) {
        resume();
        return;
    }
    
    playing_ = true;
    paused_ = false;
    controller_->reset();
    
    updateButtonStates();
    progressLabel_->setText(tr("Playing..."));
    
    emit playRequested();
}

void PlaybackWidget::pause()
{
    if (!playing_ || paused_) {
        return;
    }
    
    paused_ = true;
    controller_->requestPause();
    
    updateButtonStates();
    progressLabel_->setText(tr("Paused"));
    
    emit pauseRequested();
}

void PlaybackWidget::resume()
{
    if (!playing_ || !paused_) {
        return;
    }
    
    paused_ = false;
    controller_->requestResume();
    
    updateButtonStates();
    progressLabel_->setText(tr("Playing..."));
    
    emit resumeRequested();
}

void PlaybackWidget::stop()
{
    if (!playing_)
    {
        return;
    }

    /*
     * Keep playing_ true until PlaybackThread has actually exited.
     * MainWindow calls setPlaybackStopped() from QThread::finished.
     */
    paused_ =
        false;

    controller_->requestCancel();

    playButton_->setEnabled(
        false);

    pauseButton_->setEnabled(
        false);

    stopButton_->setEnabled(
        false);

    progressLabel_->setText(
        tr("Stopping..."));

    emit stopRequested();
}

void PlaybackWidget::togglePlayPause()
{
    if (!playing_) {
        play();
    } else if (paused_) {
        resume();
    } else {
        pause();
    }
}

void PlaybackWidget::onPlaybackProgress(const PlaybackProgress& progress)
{
    updateProgressDisplay(progress);
    emit progressUpdated(progress);
}

void PlaybackWidget::onPlaybackCompleted()
{
    playing_ = false;
    paused_ = false;
    
    updateButtonStates();
    progressLabel_->setText(tr("Completed"));
    progressBar_->setValue(100);
}

void PlaybackWidget::onPlaybackError(const QString& message)
{
    playing_ = false;
    paused_ = false;
    
    updateButtonStates();
    progressLabel_->setText(tr("Error: %1").arg(message));
    progressBar_->setValue(0);
}

void PlaybackWidget::updateButtonStates()
{
    const bool hasRecording =
        recording_
        && !recording_->empty();

    playButton_->setEnabled(
        hasRecording
        && (!playing_ || paused_));

    playButton_->setText(
        paused_
            ? tr("▶ Resume")
            : tr("▶ Play"));

    pauseButton_->setEnabled(
        playing_
        && !paused_);

    stopButton_->setEnabled(
        playing_);

    loopCombo_->setEnabled(
        !playing_);

    loopSpinBox_->setEnabled(
        !playing_);

    inputMethodCombo_->setEnabled(
        !playing_);

    dryRunCheck_->setEnabled(
        !playing_);

    alignStartCheck_->setEnabled(
        !playing_);

    if (playing_)
    {
        blockPhysicalMouseCheck_->setEnabled(
            false);

        return;
    }

    onInputMethodChanged(
        inputMethodCombo_->currentIndex());
}

void PlaybackWidget::updateProgressDisplay(const PlaybackProgress& progress)
{
    // Update progress bar
    if (progress.totalEvents > 0) {
        int percent = static_cast<int>(
            (progress.completedEvents * 100) / progress.totalEvents
        );
        progressBar_->setValue(percent);
    }
    
    // Update progress label
    switch (progress.state) {
        case PlaybackState::Preparing:
            progressLabel_->setText(tr("Preparing..."));
            break;
        case PlaybackState::Armed:
            progressLabel_->setText(tr("Press hotkey to start"));
            break;
        case PlaybackState::Playing:
            progressLabel_->setText(tr("Playing: %1/%2")
                .arg(progress.completedEvents)
                .arg(progress.totalEvents));
            break;
        case PlaybackState::Paused:
            progressLabel_->setText(tr("Paused: %1/%2")
                .arg(progress.completedEvents)
                .arg(progress.totalEvents));
            break;
        case PlaybackState::LoopCompleted:
            progressLabel_->setText(tr("Loop %1 completed")
                .arg(progress.currentLoop));
            break;
        case PlaybackState::Completed:
            progressLabel_->setText(tr("Completed"));
            break;
        case PlaybackState::Cancelled:
            progressLabel_->setText(tr("Cancelled"));
            break;
        case PlaybackState::TimedOut:
            progressLabel_->setText(tr("Timed out"));
            break;
        case PlaybackState::Failed:
            progressLabel_->setText(tr("Failed"));
            break;
        default:
            break;
    }
    
    // Update loop label
    if (progress.infiniteLoops) {
        loopLabel_->setText(tr("Loop %1 (∞)").arg(progress.currentLoop));
    } else if (progress.totalLoops > 1) {
        loopLabel_->setText(tr("Loop %1/%2")
            .arg(progress.currentLoop)
            .arg(progress.totalLoops));
    } else {
        loopLabel_->clear();
    }
}

double PlaybackWidget::speed() const
{
    return 1.0;
}

bool PlaybackWidget::blockPhysicalMouseEnabled() const
{
    if (!blockPhysicalMouseCheck_
        || !blockPhysicalMouseCheck_->isChecked())
    {
        return false;
    }

    const InputBackendType backendType =
        inputBackendType();

    return backendType
            != InputBackendType::
                VhfCorrectedRelative
        && backendType
            != InputBackendType::
                VhfAbsolute
        && backendType
            != InputBackendType::
                VhfNativeRelative;
}
InputBackendType PlaybackWidget::inputBackendType() const
{
    if (!inputMethodCombo_)
    {
        return InputBackendType::
            SendInputAbsolute;
    }

    const int backendValue =
        inputMethodCombo_->currentData().toInt();

    switch (static_cast<InputBackendType>(
                backendValue))
    {
		

        case InputBackendType::
            SendInputAbsolute:
            return InputBackendType::
                SendInputAbsolute;

        case InputBackendType::
            SendInputCorrectedRelative:
            return InputBackendType::
                SendInputCorrectedRelative;

        case InputBackendType::
            VhfCorrectedRelative:
            return InputBackendType::
                VhfCorrectedRelative;

        case InputBackendType::
            VhfAbsolute:
            return InputBackendType::
                VhfAbsolute;
		case InputBackendType::
			VhfNativeRelative:
			return InputBackendType::
				VhfNativeRelative;
        default:
            return InputBackendType::
                SendInputAbsolute;
    }
}

void PlaybackWidget::onInputMethodChanged(
    int index)
{
    if (!inputMethodCombo_
        || !blockPhysicalMouseCheck_
        || index < 0)
    {
        return;
    }

    const InputBackendType backendType =
        static_cast<InputBackendType>(
            inputMethodCombo_->itemData(
                index).toInt());

    QSettings settings(
        "InputPlay",
        "Studio");

    settings.setValue(
        "playback/inputBackend",
        static_cast<int>(
            backendType));

	const bool virtualHidSelected =
		backendType
			== InputBackendType::
				VhfCorrectedRelative
		|| backendType
			== InputBackendType::
				VhfAbsolute
		|| backendType
			== InputBackendType::
				VhfNativeRelative;

    if (virtualHidSelected)
    {
        blockPhysicalMouseCheck_->setChecked(
            false);

        blockPhysicalMouseCheck_->setEnabled(
            false);

        blockPhysicalMouseCheck_->setToolTip(
            tr(
                "Physical mouse blocking is unavailable with "
                "Virtual HID playback because the current blocker "
                "cannot distinguish the virtual mouse from the "
                "physical mouse."));
    }
    else
    {
        blockPhysicalMouseCheck_->setEnabled(
            !playing_
            && !dryRunCheck_->isChecked());

        blockPhysicalMouseCheck_->setToolTip(
            tr(
                "Prevents physical mouse movement, clicks, and "
                "scrolling from interfering with real playback. "
                "F9 and Emergency Stop remain available."));
    }

    switch (backendType)
    {
        case InputBackendType::
            SendInputAbsolute:
            inputMethodCombo_->setToolTip(
                tr(
                    "Replays the recorded absolute cursor path "
                    "through Windows SendInput. Provides the "
                    "highest desktop-position accuracy."));
            break;

        case InputBackendType::
            SendInputCorrectedRelative:
            inputMethodCombo_->setToolTip(
                tr(
                    "Replays relative movement through Windows "
                    "SendInput and continuously corrects "
                    "positional drift."));
            break;

		case InputBackendType::
			VhfCorrectedRelative:
			inputMethodCombo_->setToolTip(
				tr(
					"Experimental: replays relative movement through the "
					"installed Virtual HID device and applies positional "
					"feedback correction. This mode may oscillate on some "
					"systems."));
			break;
			
		case InputBackendType::
			VhfNativeRelative:
			inputMethodCombo_->setToolTip(
				tr(
					"Replays the original recorded relative mouse "
					"movement through the installed Virtual HID device "
					"without positional feedback correction. This is the "
					"recommended Virtual HID relative mode."));
			break;

        case InputBackendType::
            VhfAbsolute:
            inputMethodCombo_->setToolTip(
				tr(
				"Replays the recorded absolute cursor path through "
				"the installed Virtual HID device. Coordinates apply "
				"to the primary monitor."));
            break;
			
		
    }
}

void PlaybackWidget::onDryRunChanged(
    bool checked)
{
    Q_UNUSED(
        checked);

    if (!inputMethodCombo_)
    {
        return;
    }

    onInputMethodChanged(
        inputMethodCombo_->currentIndex());
}

bool PlaybackWidget::isDryRun() const
{
    return dryRunCheck_->isChecked();
}

bool PlaybackWidget::alignStartEnabled() const
{
    return alignStartCheck_->isChecked();
}

bool PlaybackWidget::isLooping() const
{
    /*
     * A stored value of zero represents infinite playback.
     * Values greater than zero represent a finite loop count.
     */
    return loopCombo_->currentData().toInt()
        == 0;
}

int PlaybackWidget::loopCount() const
{
    const int selectedValue =
        loopCombo_->currentData().toInt();

    if (selectedValue == -1)
    {
        return loopSpinBox_->value();
    }

    if (selectedValue == 0)
    {
        return 0;
    }

    return selectedValue;
}

void PlaybackWidget::applyDefaults(
    int loopCount,
    bool dryRun)
{
    if (playing_)
    {
        return;
    }
	
	if (loopCount == 0)
	{
		const int infiniteIndex =
			loopCombo_->findData(
				0);

		loopCombo_->setCurrentIndex(
			infiniteIndex);

		loopSpinBox_->setVisible(
			false);

		dryRunCheck_->setChecked(
			dryRun);

		return;
	}

	if (loopCount < 1)
	{
		loopCount =
			1;
	}

    /*
     * Use a predefined entry when one exists. Otherwise select the
     * Custom entry and put the value in the custom spin box.
     */
    int matchingIndex =
        loopCombo_->findData(
            loopCount);

    if (matchingIndex >= 0)
    {
        loopCombo_->setCurrentIndex(
            matchingIndex);

        loopSpinBox_->setVisible(
            false);
    }
    else
    {
        const int customIndex =
            loopCombo_->findData(
                -1);

        loopCombo_->setCurrentIndex(
            customIndex);

        loopSpinBox_->setValue(
            loopCount);

        loopSpinBox_->setVisible(
            true);
    }

    dryRunCheck_->setChecked(
        dryRun);
}

void PlaybackWidget::setPlaybackActive(bool active)
{
    playing_ = active;
    paused_ = false;
    updateButtonStates();
    
    if (!active) {
        progressBar_->setValue(0);
    }
}

void PlaybackWidget::setPlaybackStopped()
{
    playing_ =
        false;

    paused_ =
        false;

    updateButtonStates();

    progressBar_->setValue(
        0);

    progressLabel_->setText(
        tr("Stopped"));

    loopLabel_->clear();

    timeLabel_->setText(
        tr("0:00 / 0:00"));
}

void PlaybackWidget::updateProgress(int eventIndex, int totalEvents)
{
    if (totalEvents > 0) {
        int percent = (eventIndex * 100) / totalEvents;
        progressBar_->setValue(percent);
        progressLabel_->setText(tr("Playing: %1/%2").arg(eventIndex).arg(totalEvents));
    }
}
