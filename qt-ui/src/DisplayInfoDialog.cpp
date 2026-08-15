#include "DisplayInfoDialog.h"
#include "DarkStyle.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QPainter>
#include <QScrollArea>
#include <QFrame>

// MonitorDiagramWidget implementation

MonitorDiagramWidget::MonitorDiagramWidget(QWidget* parent)
    : QWidget(parent)
{
    setMinimumSize(300, 150);
}

void MonitorDiagramWidget::setRecordedMetadata(const DisplayMetadata& metadata)
{
    recordedMetadata_ = metadata;
    hasRecorded_ = !metadata.monitors.empty();
    update();
}

void MonitorDiagramWidget::setCurrentMetadata(const DisplayMetadata& metadata)
{
    currentMetadata_ = metadata;
    hasCurrent_ = !metadata.monitors.empty();
    update();
}

void MonitorDiagramWidget::setShowBoth(bool show)
{
    showBoth_ = show;
    update();
}

void MonitorDiagramWidget::paintEvent(
    QPaintEvent* event)
{
    Q_UNUSED(event);

    QPainter painter(this);

    painter.setRenderHint(
        QPainter::Antialiasing);

    // All custom-painted colors come from the active theme.
    painter.fillRect(
        rect(),
        QColor(
            DarkStyle::bgPrimary()));

    painter.setPen(
        QColor(
            DarkStyle::border()));

    painter.drawRect(
        rect().adjusted(
            0,
            0,
            -1,
            -1));

    if (!hasRecorded_
        && !hasCurrent_)
    {
        painter.setPen(
            DarkStyle::toneColor(
                "secondary"));

        painter.drawText(
            rect(),
            Qt::AlignCenter,
            tr("No display data available"));

        return;
    }

    const int margin = 20;

    const QRect drawArea =
        rect().adjusted(
            margin,
            margin + 20,
            -margin,
            -margin);

    QFont labelFont =
        painter.font();

    labelFont.setPointSize(9);

    painter.setFont(
        labelFont);

    if (showBoth_
        && hasRecorded_
        && hasCurrent_)
    {
        const int halfWidth =
            drawArea.width() / 2 - 10;

        const QRect leftArea(
            drawArea.left(),
            drawArea.top(),
            halfWidth,
            drawArea.height());

        const QRect rightArea(
            drawArea.left() + halfWidth + 20,
            drawArea.top(),
            halfWidth,
            drawArea.height());

        painter.setPen(
            DarkStyle::toneColor(
                "accentLight"));

        painter.drawText(
            leftArea.left(),
            drawArea.top() - 5,
            tr("Recorded"));

        painter.setPen(
            DarkStyle::toneColor(
                "accent"));

        painter.drawText(
            rightArea.left(),
            drawArea.top() - 5,
            tr("Current"));

        drawMonitorLayout(
            painter,
            recordedMetadata_,
            "accentLight",
            leftArea,
            true);

        drawMonitorLayout(
            painter,
            currentMetadata_,
            "accent",
            rightArea,
            true);
    }
    else if (hasRecorded_)
    {
        painter.setPen(
            DarkStyle::toneColor(
                "accentLight"));

        painter.drawText(
            drawArea.left(),
            drawArea.top() - 5,
            tr(
                "Recorded Display "
                "Configuration"));

        drawMonitorLayout(
            painter,
            recordedMetadata_,
            "accentLight",
            drawArea,
            true);
    }
    else if (hasCurrent_)
    {
        painter.setPen(
            DarkStyle::toneColor(
                "accent"));

        painter.drawText(
            drawArea.left(),
            drawArea.top() - 5,
            tr(
                "Current Display "
                "Configuration"));

        drawMonitorLayout(
            painter,
            currentMetadata_,
            "accent",
            drawArea,
            true);
    }
}

void MonitorDiagramWidget::drawMonitorLayout(
    QPainter& painter,
    const DisplayMetadata& metadata,
    const char* tone,
    const QRect& bounds,
    bool showLabels)
{
    if (metadata.monitors.empty())
    {
        return;
    }

    const QColor color =
        DarkStyle::toneColor(
            tone);

    const int minX =
        metadata.virtualDesktopLeft;

    const int minY =
        metadata.virtualDesktopTop;

    const int maxX =
        metadata.virtualDesktopLeft
        + metadata.virtualDesktopWidth;

    const int maxY =
        metadata.virtualDesktopTop
        + metadata.virtualDesktopHeight;

    const int totalWidth =
        maxX - minX;

    const int totalHeight =
        maxY - minY;

    if (totalWidth <= 0
        || totalHeight <= 0)
    {
        return;
    }

    const double scaleX =
        static_cast<double>(
            bounds.width())
        / totalWidth;

    const double scaleY =
        static_cast<double>(
            bounds.height())
        / totalHeight;

    const double scale =
        qMin(
            scaleX,
            scaleY)
        * 0.9;

    const int scaledWidth =
        static_cast<int>(
            totalWidth * scale);

    const int scaledHeight =
        static_cast<int>(
            totalHeight * scale);

    const int offsetX =
        bounds.left()
        + (bounds.width() - scaledWidth) / 2;

    const int offsetY =
        bounds.top()
        + (bounds.height() - scaledHeight) / 2;

    for (size_t index = 0;
         index < metadata.monitors.size();
         ++index)
    {
        const MonitorMetadata& monitor =
            metadata.monitors[index];

        const int x =
            offsetX
            + static_cast<int>(
                (monitor.left - minX)
                * scale);

        const int y =
            offsetY
            + static_cast<int>(
                (monitor.top - minY)
                * scale);

        const int width =
            static_cast<int>(
                (monitor.right - monitor.left)
                * scale);

        const int height =
            static_cast<int>(
                (monitor.bottom - monitor.top)
                * scale);

        const QRect monitorRect(
            x,
            y,
            width,
            height);

        QColor fillColor =
            color;

        fillColor.setAlpha(
            monitor.primary
            ? 60
            : 40);

        painter.fillRect(
            monitorRect,
            fillColor);

        const QPen monitorPen(
            color,
            monitor.primary
            ? 2
            : 1);

        painter.setPen(
            monitorPen);

        painter.setBrush(
            Qt::NoBrush);

        painter.drawRect(
            monitorRect);

        if (showLabels
            && monitorRect.width() > 30
            && monitorRect.height() > 20)
        {
            painter.setPen(
                color);

            QFont font =
                painter.font();

            font.setPointSize(8);

            font.setBold(
                monitor.primary);

            painter.setFont(
                font);

            QString label =
                QString::number(
                    index + 1);

            if (monitor.primary)
            {
                label += "*";
            }

            painter.drawText(
                monitorRect,
                Qt::AlignCenter,
                label);
        }
    }
}
// DisplayInfoDialog implementation

DisplayInfoDialog::DisplayInfoDialog(Recording* recording, QWidget* parent)
    : QDialog(parent)
    , recording_(recording)
{
    setWindowTitle(tr("Display Compatibility"));
    setMinimumSize(700, 500);
    resize(800, 600);
    
    // Capture current display
    std::string errorMsg;
    hasCurrentMetadata_ = captureDisplayMetadata(currentMetadata_, errorMsg);
    
    setupUi();
    updateDisplay();
}

void DisplayInfoDialog::setupUi()
{
    QVBoxLayout* mainLayout =
        new QVBoxLayout(this);

    mainLayout->setSpacing(12);

    // Status header.
    QGroupBox* statusGroup =
        new QGroupBox(
            tr("Compatibility Status"));

    QHBoxLayout* statusLayout =
        new QHBoxLayout(
            statusGroup);

    statusIconLabel_ =
        new QLabel();

    statusIconLabel_->setFixedSize(
        32,
        32);

    statusLayout->addWidget(
        statusIconLabel_);

    QVBoxLayout* statusTextLayout =
        new QVBoxLayout();

    statusLabel_ =
        new QLabel(
            tr("Checking..."));

    statusLabel_->setProperty(
        "statusHeading",
        true);

    DarkStyle::setTone(
        statusLabel_,
        "accent");

    statusTextLayout->addWidget(
        statusLabel_);

    compatMessageLabel_ =
        new QLabel();

    compatMessageLabel_->setWordWrap(
        true);

    DarkStyle::setTone(
        compatMessageLabel_,
        "secondary");

    statusTextLayout->addWidget(
        compatMessageLabel_);

    statusLayout->addLayout(
        statusTextLayout,
        1);

    mainLayout->addWidget(
        statusGroup);

    // Visual diagram.
    QGroupBox* diagramGroup =
        new QGroupBox(
            tr("Monitor Layout Comparison"));

    QVBoxLayout* diagramLayout =
        new QVBoxLayout(
            diagramGroup);

    diagramWidget_ =
        new MonitorDiagramWidget();

    diagramLayout->addWidget(
        diagramWidget_);

    mainLayout->addWidget(
        diagramGroup);

    // Details in a horizontal split.
    QHBoxLayout* detailsLayout =
        new QHBoxLayout();

    // Recorded display information.
    QGroupBox* recordedGroup =
        new QGroupBox(
            tr("Recorded Display"));

    DarkStyle::setTone(
        recordedGroup,
        "accentLight");

    QVBoxLayout* recordedLayout =
        new QVBoxLayout(
            recordedGroup);

    recordedVirtualDesktopLabel_ =
        new QLabel(
            tr("Virtual Desktop: -"));

    recordedLayout->addWidget(
        recordedVirtualDesktopLabel_);

    recordedMonitorCountLabel_ =
        new QLabel(
            tr("Monitors: -"));

    recordedLayout->addWidget(
        recordedMonitorCountLabel_);

    QScrollArea* recordedScroll =
        new QScrollArea();

    recordedScroll->setWidgetResizable(
        true);

    recordedScroll->setFrameShape(
        QFrame::NoFrame);

    QWidget* recordedMonitorsWidget =
        new QWidget();

    recordedMonitorsGrid_ =
        new QGridLayout(
            recordedMonitorsWidget);

    recordedMonitorsGrid_->setSpacing(
        8);

    recordedScroll->setWidget(
        recordedMonitorsWidget);

    recordedLayout->addWidget(
        recordedScroll,
        1);

    detailsLayout->addWidget(
        recordedGroup);

    // Current display information.
    QGroupBox* currentGroup =
        new QGroupBox(
            tr("Current Display"));

    DarkStyle::setTone(
        currentGroup,
        "accent");

    QVBoxLayout* currentLayout =
        new QVBoxLayout(
            currentGroup);

    currentVirtualDesktopLabel_ =
        new QLabel(
            tr("Virtual Desktop: -"));

    currentLayout->addWidget(
        currentVirtualDesktopLabel_);

    currentMonitorCountLabel_ =
        new QLabel(
            tr("Monitors: -"));

    currentLayout->addWidget(
        currentMonitorCountLabel_);

    QScrollArea* currentScroll =
        new QScrollArea();

    currentScroll->setWidgetResizable(
        true);

    currentScroll->setFrameShape(
        QFrame::NoFrame);

    QWidget* currentMonitorsWidget =
        new QWidget();

    currentMonitorsGrid_ =
        new QGridLayout(
            currentMonitorsWidget);

    currentMonitorsGrid_->setSpacing(
        8);

    currentScroll->setWidget(
        currentMonitorsWidget);

    currentLayout->addWidget(
        currentScroll,
        1);

    detailsLayout->addWidget(
        currentGroup);

    mainLayout->addLayout(
        detailsLayout,
        1);

    // Close button.
    QHBoxLayout* buttonLayout =
        new QHBoxLayout();

    buttonLayout->addStretch();

    QPushButton* closeButton =
        new QPushButton(
            tr("Close"));

    closeButton->setMinimumWidth(
        100);

    connect(
        closeButton,
        &QPushButton::clicked,
        this,
        &QDialog::accept);

    buttonLayout->addWidget(
        closeButton);

    mainLayout->addLayout(
        buttonLayout);
}

void DisplayInfoDialog::updateDisplay()
{
    // Remove existing recorded-monitor detail rows.
    while (QLayoutItem* item =
               recordedMonitorsGrid_->takeAt(0))
    {
        delete item->widget();
        delete item;
    }

    // Remove existing current-monitor detail rows.
    while (QLayoutItem* item =
               currentMonitorsGrid_->takeAt(0))
    {
        delete item->widget();
        delete item;
    }

    // Handle recordings without display metadata.
    if (!recording_
        || !recording_->hasDisplayMetadata())
    {
        statusLabel_->setText(
            tr("No Display Data"));

        DarkStyle::setTone(
            statusLabel_,
            "secondary");

        compatMessageLabel_->setText(
            tr(
                "This recording does not contain "
                "display metadata. It was likely "
                "recorded with an older version or "
                "display capture was disabled."));

        DarkStyle::setTone(
            compatMessageLabel_,
            "secondary");

        recordedVirtualDesktopLabel_->setText(
            tr("Virtual Desktop: Not recorded"));

        recordedMonitorCountLabel_->setText(
            tr("Monitors: Unknown"));

        diagramWidget_->setRecordedMetadata(
            DisplayMetadata{});

        diagramWidget_->setCurrentMetadata(
            currentMetadata_);

        if (hasCurrentMetadata_)
        {
            currentVirtualDesktopLabel_->setText(
                tr("Virtual Desktop: %1 at %2")
                    .arg(
                        formatResolution(
                            currentMetadata_
                                .virtualDesktopWidth,
                            currentMetadata_
                                .virtualDesktopHeight))
                    .arg(
                        formatPosition(
                            currentMetadata_
                                .virtualDesktopLeft,
                            currentMetadata_
                                .virtualDesktopTop)));

            currentMonitorCountLabel_->setText(
                tr("Monitors: %1")
                    .arg(
                        currentMetadata_
                            .monitors
                            .size()));

            for (size_t index = 0;
                 index
                 < currentMetadata_.monitors.size();
                 ++index)
            {
                addMonitorInfo(
                    currentMonitorsGrid_,
                    index,
                    currentMetadata_.monitors[index],
                    "accent");
            }
        }
        else
        {
            currentVirtualDesktopLabel_->setText(
                tr(
                    "Virtual Desktop: "
                    "Could not detect"));

            currentMonitorCountLabel_->setText(
                tr("Monitors: Unknown"));
        }

        return;
    }

    const DisplayMetadata& recorded =
        recording_->displayMetadata();

    diagramWidget_->setRecordedMetadata(
        recorded);

    diagramWidget_->setCurrentMetadata(
        currentMetadata_);

    // Recorded display information.
    recordedVirtualDesktopLabel_->setText(
        tr("Virtual Desktop: %1 at %2")
            .arg(
                formatResolution(
                    recorded.virtualDesktopWidth,
                    recorded.virtualDesktopHeight))
            .arg(
                formatPosition(
                    recorded.virtualDesktopLeft,
                    recorded.virtualDesktopTop)));

    recordedMonitorCountLabel_->setText(
        tr("Monitors: %1")
            .arg(
                recorded.monitors.size()));

    for (size_t index = 0;
         index < recorded.monitors.size();
         ++index)
    {
        addMonitorInfo(
            recordedMonitorsGrid_,
            index,
            recorded.monitors[index],
            "accentLight");
    }

    // Current display information.
    if (hasCurrentMetadata_)
    {
        currentVirtualDesktopLabel_->setText(
            tr("Virtual Desktop: %1 at %2")
                .arg(
                    formatResolution(
                        currentMetadata_
                            .virtualDesktopWidth,
                        currentMetadata_
                            .virtualDesktopHeight))
                .arg(
                    formatPosition(
                        currentMetadata_
                            .virtualDesktopLeft,
                        currentMetadata_
                            .virtualDesktopTop)));

        currentMonitorCountLabel_->setText(
            tr("Monitors: %1")
                .arg(
                    currentMetadata_
                        .monitors
                        .size()));

        for (size_t index = 0;
             index < currentMetadata_.monitors.size();
             ++index)
        {
            addMonitorInfo(
                currentMonitorsGrid_,
                index,
                currentMetadata_.monitors[index],
                "accent");
        }

        std::string compatibilityMessage;

        const DisplayCompatibility compatibility =
            compareDisplayMetadata(
                recorded,
                currentMetadata_,
                compatibilityMessage);

        switch (compatibility)
        {
            case DisplayCompatibility::Exact:
                statusLabel_->setText(
                    tr("✓ Fully Compatible"));

                DarkStyle::setTone(
                    statusLabel_,
                    "accent");

                compatMessageLabel_->setText(
                    tr(
                        "Your current display "
                        "configuration exactly matches "
                        "the recorded configuration. "
                        "Playback should work perfectly."));

                break;

            case DisplayCompatibility::CompatibleWithWarnings:
                statusLabel_->setText(
                    tr(
                        "⚠ Compatible with "
                        "Warnings"));

                DarkStyle::setTone(
                    statusLabel_,
                    "accentLight");

                compatMessageLabel_->setText(
                    tr(
                        "Minor differences detected: "
                        "%1\nPlayback should still work "
                        "but may have slight positioning "
                        "differences.")
                        .arg(
                            QString::fromStdString(
                                compatibilityMessage)));

                break;

            case DisplayCompatibility::Incompatible:
                statusLabel_->setText(
                    tr("✗ Incompatible"));

                DarkStyle::setTone(
                    statusLabel_,
                    "accentDark");

                compatMessageLabel_->setText(
                    tr(
                        "Display configuration mismatch: "
                        "%1\nPlayback may not work "
                        "correctly. Mouse coordinates may "
                        "be off-screen or misaligned.")
                        .arg(
                            QString::fromStdString(
                                compatibilityMessage)));

                break;

            default:
                statusLabel_->setText(
                    tr("? Unknown"));

                DarkStyle::setTone(
                    statusLabel_,
                    "secondary");

                compatMessageLabel_->setText(
                    tr(
                        "Could not determine "
                        "compatibility."));

                break;
        }
    }
    else
    {
        currentVirtualDesktopLabel_->setText(
            tr(
                "Virtual Desktop: "
                "Could not detect"));

        currentMonitorCountLabel_->setText(
            tr("Monitors: Unknown"));

        statusLabel_->setText(
            tr("⚠ Cannot Verify"));

        DarkStyle::setTone(
            statusLabel_,
            "accentLight");

        compatMessageLabel_->setText(
            tr(
                "Could not capture current display "
                "information to verify compatibility."));
    }

    DarkStyle::setTone(
        compatMessageLabel_,
        "secondary");
}

void DisplayInfoDialog::addMonitorInfo(
    QGridLayout* grid,
    size_t index,
    const MonitorMetadata& monitor,
    const char* tone)
{
    const int row =
        static_cast<int>(
            index);

    QLabel* numberLabel =
        new QLabel(
            QString("#%1%2")
                .arg(index + 1)
                .arg(
                    monitor.primary
                    ? " (Primary)"
                    : ""));

    numberLabel->setProperty(
        "bold",
        true);

    DarkStyle::setTone(
        numberLabel,
        tone);

    grid->addWidget(
        numberLabel,
        row,
        0);

    const int width =
        monitor.right - monitor.left;

    const int height =
        monitor.bottom - monitor.top;

    QLabel* resolutionLabel =
        new QLabel(
            formatResolution(
                width,
                height));

    grid->addWidget(
        resolutionLabel,
        row,
        1);

    QLabel* positionLabel =
        new QLabel(
            formatPosition(
                monitor.left,
                monitor.top));

    DarkStyle::setTone(
        positionLabel,
        "secondary");

    grid->addWidget(
        positionLabel,
        row,
        2);

    QLabel* nameLabel =
        new QLabel(
            QString::fromStdString(
                monitor.deviceName));

    DarkStyle::setTone(
        nameLabel,
        "secondary");

    grid->addWidget(
        nameLabel,
        row,
        3);
}

QString DisplayInfoDialog::formatResolution(int width, int height) const
{
    return QString("%1 × %2").arg(width).arg(height);
}

QString DisplayInfoDialog::formatPosition(int x, int y) const
{
    return QString("(%1, %2)").arg(x).arg(y);
}
