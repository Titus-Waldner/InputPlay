#pragma once

#include "Recording.h"

#include <QWidget>
#include <QLabel>
#include <QMap>

class QVBoxLayout;
class QPushButton;

// Bar chart widget for event distribution
class EventBarChart : public QWidget
{
    Q_OBJECT

public:
    explicit EventBarChart(QWidget* parent = nullptr);
    
    void setData(const QMap<QString, int>& data, const QMap<QString, QColor>& colors);
    void clear();

protected:
    void paintEvent(QPaintEvent* event) override;
    QSize sizeHint() const override { return QSize(300, 150); }
    QSize minimumSizeHint() const override { return QSize(200, 100); }

private:
    QMap<QString, int> data_;
    QMap<QString, QColor> colors_;
    int maxValue_ = 0;
};

class StatisticsPanel : public QWidget
{
    Q_OBJECT

public:
    explicit StatisticsPanel(QWidget* parent = nullptr);
    
    void setRecording(Recording* recording);
    void refresh();

signals:
    void filterByType(EventType type);

private:
    void setupUi();
    void updateDisplay();
    void updateCharts();

    void onFilterMouseMoveClicked();
    void onFilterMouseClickClicked();
    void onFilterKeyboardClicked();

    Recording* recording_ = nullptr;

    
    // Overview
    QLabel* totalEventsLabel_ = nullptr;
    QLabel* totalDurationLabel_ = nullptr;
    QLabel* eventsPerSecondLabel_ = nullptr;
    QLabel* avgDelayLabel_ = nullptr;
    
    // Event type breakdown
    QLabel* mouseMoveLabel_ = nullptr;
    QLabel* mouseClickLabel_ = nullptr;
    QLabel* mouseWheelLabel_ = nullptr;
    QLabel* keyboardLabel_ = nullptr;
    QLabel* waitLabel_ = nullptr;
    QLabel* teleportLabel_ = nullptr;
    
    // Percentages
    QLabel* mouseMovePercentLabel_ = nullptr;
    QLabel* mouseClickPercentLabel_ = nullptr;
    QLabel* mouseWheelPercentLabel_ = nullptr;
    QLabel* keyboardPercentLabel_ = nullptr;
    QLabel* waitPercentLabel_ = nullptr;
    QLabel* teleportPercentLabel_ = nullptr;
    
    // Charts
    EventBarChart* eventTypeChart_ = nullptr;
    EventBarChart* timingChart_ = nullptr;
    
    // Filter buttons
    QPushButton* filterMouseMoveBtn_ = nullptr;
    QPushButton* filterMouseClickBtn_ = nullptr;
    QPushButton* filterKeyboardBtn_ = nullptr;
};
