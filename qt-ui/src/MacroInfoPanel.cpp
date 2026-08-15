#include "MacroInfoPanel.h"
#include "RecordingSummary.h"
#include "DisplayMetadata.h"
#include "DarkStyle.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QGroupBox>
#include <QFrame>
#include <QFileInfo>
#include <QScrollArea>

MacroInfoPanel::MacroInfoPanel(QWidget* parent)
    : QWidget(parent)
{
    setupUi();
}

void MacroInfoPanel::setupUi()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(8, 8, 8, 8);
    mainLayout->setSpacing(8);
    
    // Title
    QLabel* title = new QLabel(tr("Macro Info"));
    title->setProperty("heading", true);
    mainLayout->addWidget(title);
    
    // Scroll area for info
    QScrollArea* scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    
    QWidget* scrollContent = new QWidget();
    QVBoxLayout* scrollLayout = new QVBoxLayout(scrollContent);
    scrollLayout->setContentsMargins(0, 0, 0, 0);
    scrollLayout->setSpacing(8);
    
    // File info group
    QGroupBox* fileGroup = new QGroupBox(tr("File"));
    QVBoxLayout* fileLayout = new QVBoxLayout(fileGroup);
    fileLayout->setSpacing(4);
    
    fileNameLabel_ = new QLabel(tr("No file loaded"));
    fileNameLabel_->setWordWrap(true);
    fileLayout->addWidget(fileNameLabel_);
    
    filePathLabel_ = new QLabel();
    filePathLabel_->setProperty("subheading", true);
    filePathLabel_->setWordWrap(true);
    fileLayout->addWidget(filePathLabel_);
    
    fileSizeLabel_ = new QLabel();
    fileSizeLabel_->setProperty("subheading", true);
    fileLayout->addWidget(fileSizeLabel_);
    
    scrollLayout->addWidget(fileGroup);
    
    // Statistics group
    QGroupBox* statsGroup = new QGroupBox(tr("Statistics"));
    QVBoxLayout* statsLayout = new QVBoxLayout(statsGroup);
    statsLayout->setSpacing(4);
    
    eventCountLabel_ = new QLabel(tr("Events: -"));
    statsLayout->addWidget(eventCountLabel_);
    
    durationLabel_ = new QLabel(tr("Duration: -"));
    statsLayout->addWidget(durationLabel_);
    
    startPosLabel_ = new QLabel(tr("Start: -"));
    statsLayout->addWidget(startPosLabel_);
    
    scrollLayout->addWidget(statsGroup);
    
    // Event breakdown group
	
	

	QGroupBox* breakdownGroup =
		new QGroupBox(
			tr("Event Breakdown"));

	QVBoxLayout* breakdownLayout =
		new QVBoxLayout(
			breakdownGroup);

	breakdownLayout->setSpacing(2);

	mouseMoveCountLabel_ =
		new QLabel(
			tr("Mouse Moves: -"));

	DarkStyle::setEventTone(
		mouseMoveCountLabel_,
		"mouseMove");

	breakdownLayout->addWidget(
		mouseMoveCountLabel_);

	mouseClickCountLabel_ =
		new QLabel(
			tr("Mouse Clicks: -"));

	DarkStyle::setEventTone(
		mouseClickCountLabel_,
		"mouseClick");

	breakdownLayout->addWidget(
		mouseClickCountLabel_);

	mouseWheelCountLabel_ =
		new QLabel(
			tr("Mouse Wheel: -"));

	DarkStyle::setEventTone(
		mouseWheelCountLabel_,
		"mouseWheel");

	breakdownLayout->addWidget(
		mouseWheelCountLabel_);

	keyCountLabel_ =
		new QLabel(
			tr("Key Events: -"));

	DarkStyle::setEventTone(
		keyCountLabel_,
		"keyboard");

	breakdownLayout->addWidget(
		keyCountLabel_);

	waitCountLabel_ =
		new QLabel(
			tr("Wait Events: -"));

	DarkStyle::setEventTone(
		waitCountLabel_,
		"wait");

	breakdownLayout->addWidget(
		waitCountLabel_);

	teleportCountLabel_ =
		new QLabel(
			tr("Teleports: -"));

	DarkStyle::setEventTone(
		teleportCountLabel_,
		"teleport");

	breakdownLayout->addWidget(
		teleportCountLabel_);

	scrollLayout->addWidget(
		breakdownGroup);
	
    
    // Display info group
    QGroupBox* displayGroup = new QGroupBox(tr("Display Compatibility"));
    QVBoxLayout* displayLayout = new QVBoxLayout(displayGroup);
    displayLayout->setSpacing(4);
    
    displayStatusLabel_ = new QLabel(tr("Status: Unknown"));
    displayLayout->addWidget(displayStatusLabel_);
    
    monitorCountLabel_ = new QLabel(tr("Monitors: -"));
    displayLayout->addWidget(monitorCountLabel_);
    
    virtualDesktopLabel_ = new QLabel(tr("Virtual Desktop: -"));
    virtualDesktopLabel_->setWordWrap(true);
    displayLayout->addWidget(virtualDesktopLabel_);
    
    scrollLayout->addWidget(displayGroup);
    
    scrollLayout->addStretch();
    
    scrollArea->setWidget(scrollContent);
    mainLayout->addWidget(scrollArea, 1);
}

void MacroInfoPanel::setRecording(Recording* recording, const QString& filePath)
{
    recording_ = recording;
    filePath_ = filePath;
    updateDisplay();
}

void MacroInfoPanel::refresh()
{
    updateDisplay();
}

void MacroInfoPanel::updateDisplay()
{
    if (!recording_) {
        fileNameLabel_->setText(tr("No file loaded"));
        filePathLabel_->clear();
        fileSizeLabel_->clear();
        eventCountLabel_->setText(tr("Events: -"));
        durationLabel_->setText(tr("Duration: -"));
        startPosLabel_->setText(tr("Start: -"));
        mouseMoveCountLabel_->setText(tr("Mouse Moves: -"));
        mouseClickCountLabel_->setText(tr("Mouse Clicks: -"));
        mouseWheelCountLabel_->setText(tr("Mouse Wheel: -"));
        keyCountLabel_->setText(tr("Key Events: -"));
        waitCountLabel_->setText(tr("Wait Events: -"));
        teleportCountLabel_->setText(tr("Teleports: -"));
        displayStatusLabel_->setText(tr("Status: Unknown"));
        monitorCountLabel_->setText(tr("Monitors: -"));
        virtualDesktopLabel_->setText(tr("Virtual Desktop: -"));
        return;
    }
    
    // File info
    if (!filePath_.isEmpty()) {
        QFileInfo fileInfo(filePath_);
        fileNameLabel_->setText(fileInfo.fileName());
        filePathLabel_->setText(fileInfo.absolutePath());
        
        qint64 size = fileInfo.size();
        if (size < 1024) {
            fileSizeLabel_->setText(tr("Size: %1 bytes").arg(size));
        } else if (size < 1024 * 1024) {
            fileSizeLabel_->setText(tr("Size: %1 KB").arg(size / 1024.0, 0, 'f', 1));
        } else {
            fileSizeLabel_->setText(tr("Size: %1 MB").arg(size / (1024.0 * 1024.0), 0, 'f', 2));
        }
    } else {
        fileNameLabel_->setText(tr("Untitled"));
        filePathLabel_->setText(tr("Not saved"));
        fileSizeLabel_->clear();
    }
    
    // Calculate summary
    RecordingSummary summary = summarizeRecording(*recording_);
    
    // Statistics
    eventCountLabel_->setText(tr("Events: %1").arg(summary.totalEvents));
    
    double durationSeconds = summary.totalDurationMicroseconds / 1'000'000.0;
    if (durationSeconds < 60.0) {
        durationLabel_->setText(tr("Duration: %1s").arg(durationSeconds, 0, 'f', 2));
    } else {
        int minutes = static_cast<int>(durationSeconds / 60.0);
        double seconds = durationSeconds - (minutes * 60.0);
        durationLabel_->setText(tr("Duration: %1m %2s").arg(minutes).arg(seconds, 0, 'f', 1));
    }
    
    if (recording_->hasStartingCursorPosition()) {
        startPosLabel_->setText(tr("Start: (%1, %2)")
            .arg(recording_->startingCursorX())
            .arg(recording_->startingCursorY()));
    } else {
        startPosLabel_->setText(tr("Start: Not recorded"));
    }
    
    // Event breakdown
    mouseMoveCountLabel_->setText(tr("Mouse Moves: %1").arg(summary.mouseMovements));
    mouseClickCountLabel_->setText(tr("Mouse Clicks: %1").arg(summary.mouseButtonEvents));
    mouseWheelCountLabel_->setText(tr("Mouse Wheel: %1").arg(summary.mouseWheelEvents));
    keyCountLabel_->setText(tr("Key Events: %1").arg(summary.keyboardEvents));
    waitCountLabel_->setText(tr("Wait Events: %1").arg(summary.waitEvents));
    teleportCountLabel_->setText(tr("Teleports: %1").arg(summary.mouseTeleports));
    
    // Display compatibility
    updateDisplayCompatibility();
}

void MacroInfoPanel::updateDisplayCompatibility()
{
    if (!recording_
        || !recording_->hasDisplayMetadata())
    {
        displayStatusLabel_->setText(
            tr("Status: No display data"));

        DarkStyle::setTone(
            displayStatusLabel_,
            "secondary");

        monitorCountLabel_->setText(
            tr("Monitors: -"));

        virtualDesktopLabel_->setText(
            tr("Virtual Desktop: -"));

        return;
    }

    const DisplayMetadata& recorded =
        recording_->displayMetadata();

    // Capture the current display configuration.
    DisplayMetadata current;
    std::string errorMessage;

    if (captureDisplayMetadata(
            current,
            errorMessage))
    {
        std::string compatibilityMessage;

        const DisplayCompatibility compatibility =
            compareDisplayMetadata(
                recorded,
                current,
                compatibilityMessage);

        switch (compatibility)
        {
            case DisplayCompatibility::Exact:
                displayStatusLabel_->setText(
                    tr("Status: ✓ Compatible"));

                DarkStyle::setTone(
                    displayStatusLabel_,
                    "accent");

                break;

            case DisplayCompatibility::CompatibleWithWarnings:
                displayStatusLabel_->setText(
                    tr(
                        "Status: ⚠ Compatible "
                        "(warnings)"));

                DarkStyle::setTone(
                    displayStatusLabel_,
                    "accentLight");

                break;

            case DisplayCompatibility::Incompatible:
                displayStatusLabel_->setText(
                    tr("Status: ✗ Incompatible"));

                DarkStyle::setTone(
                    displayStatusLabel_,
                    "accentDark");

                break;

            default:
                displayStatusLabel_->setText(
                    tr("Status: ? Unknown"));

                DarkStyle::setTone(
                    displayStatusLabel_,
                    "secondary");

                break;
        }
    }
    else
    {
        displayStatusLabel_->setText(
            tr("Status: Could not check"));

        DarkStyle::setTone(
            displayStatusLabel_,
            "secondary");
    }

    monitorCountLabel_->setText(
        tr("Monitors: %1")
            .arg(
                recorded.monitors.size()));

    virtualDesktopLabel_->setText(
        tr(
            "Virtual Desktop: "
            "%1x%2 at (%3,%4)")
            .arg(
                recorded.virtualDesktopWidth)
            .arg(
                recorded.virtualDesktopHeight)
            .arg(
                recorded.virtualDesktopLeft)
            .arg(
                recorded.virtualDesktopTop));
}