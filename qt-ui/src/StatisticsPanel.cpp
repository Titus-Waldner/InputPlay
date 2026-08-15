#include "StatisticsPanel.h"
#include "RecordingSummary.h"
#include "DarkStyle.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QPainter>
#include <QPaintEvent>
#include <QScrollArea>
#include <QFrame>

#include <cmath>

// EventBarChart implementation

EventBarChart::EventBarChart(
    QWidget* parent)
    : QWidget(parent)
{
    setMinimumHeight(100);
}

void EventBarChart::setData(
    const QMap<QString, int>& data,
    const QMap<QString, QColor>& colors)
{
    data_ = data;
    colors_ = colors;
    maxValue_ = 0;

    for (auto iterator = data_.cbegin();
         iterator != data_.cend();
         ++iterator)
    {
        maxValue_ =
            qMax(
                maxValue_,
                iterator.value());
    }

    update();
}

void EventBarChart::clear()
{
    data_.clear();
    colors_.clear();
    maxValue_ = 0;

    update();
}

void EventBarChart::paintEvent(
    QPaintEvent* event)
{
    Q_UNUSED(event);

    QPainter painter(this);

    painter.setRenderHint(
        QPainter::Antialiasing);

    painter.fillRect(
        rect(),
        QColor(
            DarkStyle::bgPrimary()));

    if (data_.isEmpty()
        || maxValue_ == 0)
    {
        painter.setPen(
            DarkStyle::toneColor(
                "secondary"));

        painter.drawText(
            rect(),
            Qt::AlignCenter,
            tr("No data"));

        return;
    }

    const int margin = 10;
    const int labelHeight = 20;
    const int barSpacing = 8;

    const int numberOfBars =
        data_.size();

    const int availableWidth =
        width() - 2 * margin;

    int barWidth =
        (availableWidth
         - (numberOfBars - 1) * barSpacing)
        / numberOfBars;

    barWidth =
        qMin(
            barWidth,
            60);

    const int totalBarsWidth =
        numberOfBars * barWidth
        + (numberOfBars - 1) * barSpacing;

    const int startX =
        (width() - totalBarsWidth) / 2;

    const int chartHeight =
        height()
        - 2 * margin
        - labelHeight;

    int x = startX;

    QFont labelFont =
        painter.font();

    labelFont.setPointSize(8);

    painter.setFont(
        labelFont);

    for (auto iterator = data_.cbegin();
         iterator != data_.cend();
         ++iterator)
    {
        const QString label =
            iterator.key();

        const int value =
            iterator.value();

        const QColor color =
            colors_.value(
                label,
                DarkStyle::toneColor(
                    "accent"));

        int barHeight =
            static_cast<int>(
                (static_cast<double>(value)
                 / maxValue_)
                * chartHeight);

        barHeight =
            qMax(
                barHeight,
                2);

        const int y =
            margin
            + (chartHeight - barHeight);

        const QRect barRectangle(
            x,
            y,
            barWidth,
            barHeight);

        painter.fillRect(
            barRectangle,
            color);

        painter.setPen(
            DarkStyle::toneColor(
                "primary"));

        const QString valueText =
            value > 9999
            ? QString("%1k")
                  .arg(value / 1000)
            : QString::number(value);

        painter.drawText(
            x,
            y - 3,
            barWidth,
            15,
            Qt::AlignCenter,
            valueText);

        painter.setPen(
            DarkStyle::toneColor(
                "secondary"));

        painter.drawText(
            x - 5,
            height() - labelHeight,
            barWidth + 10,
            labelHeight,
            Qt::AlignCenter
                | Qt::TextWordWrap,
            label);

        x +=
            barWidth + barSpacing;
    }
}

// StatisticsPanel implementation

StatisticsPanel::StatisticsPanel(
    QWidget* parent)
    : QWidget(parent)
{
    setupUi();
}

void StatisticsPanel::setupUi()
{
    QVBoxLayout* mainLayout =
        new QVBoxLayout(this);

    mainLayout->setContentsMargins(
        8,
        8,
        8,
        8);

    mainLayout->setSpacing(8);

    QLabel* title =
        new QLabel(
            tr("Statistics"));

    title->setProperty(
        "heading",
        true);

    mainLayout->addWidget(
        title);

    QScrollArea* scrollArea =
        new QScrollArea();

    scrollArea->setWidgetResizable(
        true);

    scrollArea->setFrameShape(
        QFrame::NoFrame);

    QWidget* scrollContent =
        new QWidget();

    QVBoxLayout* scrollLayout =
        new QVBoxLayout(
            scrollContent);

    scrollLayout->setContentsMargins(
        0,
        0,
        0,
        0);

    scrollLayout->setSpacing(12);

    // Overview.
    QGroupBox* overviewGroup =
        new QGroupBox(
            tr("Overview"));

    QGridLayout* overviewGrid =
        new QGridLayout(
            overviewGroup);

    overviewGrid->setSpacing(6);

    overviewGrid->addWidget(
        new QLabel(
            tr("Total Events:")),
        0,
        0);

    totalEventsLabel_ =
        new QLabel(
            tr("-"));

    totalEventsLabel_->setProperty(
        "bold",
        true);

    DarkStyle::setTone(
        totalEventsLabel_,
        "accent");

    overviewGrid->addWidget(
        totalEventsLabel_,
        0,
        1);

    overviewGrid->addWidget(
        new QLabel(
            tr("Duration:")),
        1,
        0);

    totalDurationLabel_ =
        new QLabel(
            tr("-"));

    totalDurationLabel_->setProperty(
        "bold",
        true);

    DarkStyle::setTone(
        totalDurationLabel_,
        "accent");

    overviewGrid->addWidget(
        totalDurationLabel_,
        1,
        1);

    overviewGrid->addWidget(
        new QLabel(
            tr("Events/sec:")),
        2,
        0);

    eventsPerSecondLabel_ =
        new QLabel(
            tr("-"));

    overviewGrid->addWidget(
        eventsPerSecondLabel_,
        2,
        1);

    overviewGrid->addWidget(
        new QLabel(
            tr("Avg Delay:")),
        3,
        0);

    avgDelayLabel_ =
        new QLabel(
            tr("-"));

    overviewGrid->addWidget(
        avgDelayLabel_,
        3,
        1);

    scrollLayout->addWidget(
        overviewGroup);

    // Event distribution.
    QGroupBox* chartGroup =
        new QGroupBox(
            tr("Event Distribution"));

    QVBoxLayout* chartLayout =
        new QVBoxLayout(
            chartGroup);

    eventTypeChart_ =
        new EventBarChart();

    eventTypeChart_->setMinimumHeight(
        120);

    chartLayout->addWidget(
        eventTypeChart_);

    scrollLayout->addWidget(
        chartGroup);

    // Event breakdown.
    QGroupBox* breakdownGroup =
        new QGroupBox(
            tr("Event Breakdown"));

    QGridLayout* breakdownGrid =
        new QGridLayout(
            breakdownGroup);

    breakdownGrid->setSpacing(4);

    // Mouse movement.
    mouseMoveLabel_ =
        new QLabel(
            tr("Mouse Moves:"));

    DarkStyle::setEventTone(
        mouseMoveLabel_,
        "mouseMove");

    breakdownGrid->addWidget(
        mouseMoveLabel_,
        0,
        0);

    mouseMovePercentLabel_ =
        new QLabel(
            tr("-"));

    breakdownGrid->addWidget(
        mouseMovePercentLabel_,
        0,
        1,
        Qt::AlignRight);

    filterMouseMoveBtn_ =
        new QPushButton(
            tr("Filter"));

    filterMouseMoveBtn_->setMaximumWidth(
        60);

    filterMouseMoveBtn_->setProperty(
        "compact",
        true);

    connect(
        filterMouseMoveBtn_,
        &QPushButton::clicked,
        this,
        &StatisticsPanel::onFilterMouseMoveClicked);

    breakdownGrid->addWidget(
        filterMouseMoveBtn_,
        0,
        2);

    // Mouse clicks.
    mouseClickLabel_ =
        new QLabel(
            tr("Mouse Clicks:"));

    DarkStyle::setEventTone(
        mouseClickLabel_,
        "mouseClick");

    breakdownGrid->addWidget(
        mouseClickLabel_,
        1,
        0);

    mouseClickPercentLabel_ =
        new QLabel(
            tr("-"));

    breakdownGrid->addWidget(
        mouseClickPercentLabel_,
        1,
        1,
        Qt::AlignRight);

    filterMouseClickBtn_ =
        new QPushButton(
            tr("Filter"));

    filterMouseClickBtn_->setMaximumWidth(
        60);

    filterMouseClickBtn_->setProperty(
        "compact",
        true);

    connect(
        filterMouseClickBtn_,
        &QPushButton::clicked,
        this,
        &StatisticsPanel::onFilterMouseClickClicked);

    breakdownGrid->addWidget(
        filterMouseClickBtn_,
        1,
        2);

    // Mouse wheel.
    mouseWheelLabel_ =
        new QLabel(
            tr("Mouse Wheel:"));

    DarkStyle::setEventTone(
        mouseWheelLabel_,
        "mouseWheel");

    breakdownGrid->addWidget(
        mouseWheelLabel_,
        2,
        0);

    mouseWheelPercentLabel_ =
        new QLabel(
            tr("-"));

    breakdownGrid->addWidget(
        mouseWheelPercentLabel_,
        2,
        1,
        Qt::AlignRight);

    // Keyboard.
    keyboardLabel_ =
        new QLabel(
            tr("Keyboard:"));

    DarkStyle::setEventTone(
        keyboardLabel_,
        "keyboard");

    breakdownGrid->addWidget(
        keyboardLabel_,
        3,
        0);

    keyboardPercentLabel_ =
        new QLabel(
            tr("-"));

    breakdownGrid->addWidget(
        keyboardPercentLabel_,
        3,
        1,
        Qt::AlignRight);

    filterKeyboardBtn_ =
        new QPushButton(
            tr("Filter"));

    filterKeyboardBtn_->setMaximumWidth(
        60);

    filterKeyboardBtn_->setProperty(
        "compact",
        true);

    connect(
        filterKeyboardBtn_,
        &QPushButton::clicked,
        this,
        &StatisticsPanel::onFilterKeyboardClicked);

    breakdownGrid->addWidget(
        filterKeyboardBtn_,
        3,
        2);

    // Wait events.
    waitLabel_ =
        new QLabel(
            tr("Wait Events:"));

    DarkStyle::setEventTone(
        waitLabel_,
        "wait");

    breakdownGrid->addWidget(
        waitLabel_,
        4,
        0);

    waitPercentLabel_ =
        new QLabel(
            tr("-"));

    breakdownGrid->addWidget(
        waitPercentLabel_,
        4,
        1,
        Qt::AlignRight);

    // Teleports.
    teleportLabel_ =
        new QLabel(
            tr("Teleports:"));

    DarkStyle::setEventTone(
        teleportLabel_,
        "teleport");

    breakdownGrid->addWidget(
        teleportLabel_,
        5,
        0);

    teleportPercentLabel_ =
        new QLabel(
            tr("-"));

    breakdownGrid->addWidget(
        teleportPercentLabel_,
        5,
        1,
        Qt::AlignRight);

    scrollLayout->addWidget(
        breakdownGroup);

    scrollLayout->addStretch();

    scrollArea->setWidget(
        scrollContent);

    mainLayout->addWidget(
        scrollArea,
        1);
}

void StatisticsPanel::setRecording(
    Recording* recording)
{
    recording_ = recording;

    updateDisplay();
}

void StatisticsPanel::refresh()
{
    updateDisplay();
}

void StatisticsPanel::updateDisplay()
{
    if (!recording_
        || recording_->empty())
    {
        totalEventsLabel_->setText(
            tr("-"));

        totalDurationLabel_->setText(
            tr("-"));

        eventsPerSecondLabel_->setText(
            tr("-"));

        avgDelayLabel_->setText(
            tr("-"));

        mouseMovePercentLabel_->setText(
            tr("-"));

        mouseClickPercentLabel_->setText(
            tr("-"));

        mouseWheelPercentLabel_->setText(
            tr("-"));

        keyboardPercentLabel_->setText(
            tr("-"));

        waitPercentLabel_->setText(
            tr("-"));

        teleportPercentLabel_->setText(
            tr("-"));

        eventTypeChart_->clear();

        return;
    }

    const RecordingSummary summary =
        summarizeRecording(
            *recording_);

    totalEventsLabel_->setText(
        QString::number(
            summary.totalEvents));

    const double durationSeconds =
        summary.totalDurationMicroseconds
        / 1'000'000.0;

    if (durationSeconds < 60.0)
    {
        totalDurationLabel_->setText(
            tr("%1 sec")
                .arg(
                    durationSeconds,
                    0,
                    'f',
                    2));
    }
    else
    {
        const int minutes =
            static_cast<int>(
                durationSeconds / 60.0);

        const double seconds =
            durationSeconds
            - minutes * 60.0;

        totalDurationLabel_->setText(
            tr("%1m %2s")
                .arg(minutes)
                .arg(
                    seconds,
                    0,
                    'f',
                    1));
    }

    if (durationSeconds > 0.0)
    {
        const double eventsPerSecond =
            summary.totalEvents
            / durationSeconds;

        eventsPerSecondLabel_->setText(
            tr("%1")
                .arg(
                    eventsPerSecond,
                    0,
                    'f',
                    1));
    }
    else
    {
        eventsPerSecondLabel_->setText(
            tr("-"));
    }

    if (summary.totalEvents > 1)
    {
        const double averageDelay =
            summary.totalDurationMicroseconds
            / static_cast<double>(
                summary.totalEvents - 1);

        if (averageDelay < 1000.0)
        {
            avgDelayLabel_->setText(
                tr("%1 µs")
                    .arg(
                        averageDelay,
                        0,
                        'f',
                        0));
        }
        else if (averageDelay < 1'000'000.0)
        {
            avgDelayLabel_->setText(
                tr("%1 ms")
                    .arg(
                        averageDelay / 1000.0,
                        0,
                        'f',
                        1));
        }
        else
        {
            avgDelayLabel_->setText(
                tr("%1 s")
                    .arg(
                        averageDelay / 1'000'000.0,
                        0,
                        'f',
                        2));
        }
    }
    else
    {
        avgDelayLabel_->setText(
            tr("-"));
    }

    const double mouseMovePercent =
        summary.totalEvents > 0
        ? static_cast<double>(
              summary.mouseMovements)
              / summary.totalEvents
              * 100.0
        : 0.0;

    const double mouseClickPercent =
        summary.totalEvents > 0
        ? static_cast<double>(
              summary.mouseButtonEvents)
              / summary.totalEvents
              * 100.0
        : 0.0;

    const double mouseWheelPercent =
        summary.totalEvents > 0
        ? static_cast<double>(
              summary.mouseWheelEvents)
              / summary.totalEvents
              * 100.0
        : 0.0;

    const double keyboardPercent =
        summary.totalEvents > 0
        ? static_cast<double>(
              summary.keyboardEvents)
              / summary.totalEvents
              * 100.0
        : 0.0;

    const double waitPercent =
        summary.totalEvents > 0
        ? static_cast<double>(
              summary.waitEvents)
              / summary.totalEvents
              * 100.0
        : 0.0;

    const double teleportPercent =
        summary.totalEvents > 0
        ? static_cast<double>(
              summary.mouseTeleports)
              / summary.totalEvents
              * 100.0
        : 0.0;

    mouseMovePercentLabel_->setText(
        tr("%1 (%2%)")
            .arg(
                summary.mouseMovements)
            .arg(
                mouseMovePercent,
                0,
                'f',
                1));

    mouseClickPercentLabel_->setText(
        tr("%1 (%2%)")
            .arg(
                summary.mouseButtonEvents)
            .arg(
                mouseClickPercent,
                0,
                'f',
                1));

    mouseWheelPercentLabel_->setText(
        tr("%1 (%2%)")
            .arg(
                summary.mouseWheelEvents)
            .arg(
                mouseWheelPercent,
                0,
                'f',
                1));

    keyboardPercentLabel_->setText(
        tr("%1 (%2%)")
            .arg(
                summary.keyboardEvents)
            .arg(
                keyboardPercent,
                0,
                'f',
                1));

    waitPercentLabel_->setText(
        tr("%1 (%2%)")
            .arg(
                summary.waitEvents)
            .arg(
                waitPercent,
                0,
                'f',
                1));

    teleportPercentLabel_->setText(
        tr("%1 (%2%)")
            .arg(
                summary.mouseTeleports)
            .arg(
                teleportPercent,
                0,
                'f',
                1));

    updateCharts();
}

void StatisticsPanel::updateCharts()
{
    if (!recording_
        || recording_->empty())
    {
        eventTypeChart_->clear();
        return;
    }

    const RecordingSummary summary =
        summarizeRecording(
            *recording_);

    QMap<QString, int> data;
    QMap<QString, QColor> colors;

    if (summary.mouseMovements > 0)
    {
        data["Move"] =
            summary.mouseMovements;

        colors["Move"] =
            DarkStyle::eventColor(
                "mouseMove");
    }

    if (summary.mouseButtonEvents > 0)
    {
        data["Click"] =
            summary.mouseButtonEvents;

        colors["Click"] =
            DarkStyle::eventColor(
                "mouseClick");
    }

    if (summary.mouseWheelEvents > 0)
    {
        data["Wheel"] =
            summary.mouseWheelEvents;

        colors["Wheel"] =
            DarkStyle::eventColor(
                "mouseWheel");
    }

    if (summary.keyboardEvents > 0)
    {
        data["Key"] =
            summary.keyboardEvents;

        colors["Key"] =
            DarkStyle::eventColor(
                "keyboard");
    }

    if (summary.waitEvents > 0)
    {
        data["Wait"] =
            summary.waitEvents;

        colors["Wait"] =
            DarkStyle::eventColor(
                "wait");
    }

    if (summary.mouseTeleports > 0)
    {
        data["Teleport"] =
            summary.mouseTeleports;

        colors["Teleport"] =
            DarkStyle::eventColor(
                "teleport");
    }

    eventTypeChart_->setData(
        data,
        colors);
}

void StatisticsPanel::onFilterMouseMoveClicked()
{
    emit filterByType(
        EventType::MouseMove);
}

void StatisticsPanel::onFilterMouseClickClicked()
{
    emit filterByType(
        EventType::MouseButtonDown);
}

void StatisticsPanel::onFilterKeyboardClicked()
{
    emit filterByType(
        EventType::KeyDown);
}