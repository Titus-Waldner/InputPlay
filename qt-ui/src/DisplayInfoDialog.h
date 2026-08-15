#pragma once

#include "Recording.h"
#include "DisplayMetadata.h"

#include <QDialog>
#include <QLabel>
#include <QWidget>

class QGridLayout;
class QPainter;
class QRect;

// Widget that visualizes monitor layouts graphically.
class MonitorDiagramWidget
    : public QWidget
{
    Q_OBJECT

public:
    explicit MonitorDiagramWidget(
        QWidget* parent = nullptr);

    void setRecordedMetadata(
        const DisplayMetadata& metadata);

    void setCurrentMetadata(
        const DisplayMetadata& metadata);

    void setShowBoth(
        bool show);

protected:
    void paintEvent(
        QPaintEvent* event) override;

    QSize sizeHint() const override
    {
        return QSize(
            400,
            200);
    }

    QSize minimumSizeHint() const override
    {
        return QSize(
            300,
            150);
    }

private:
    void drawMonitorLayout(
        QPainter& painter,
        const DisplayMetadata& metadata,
        const char* tone,
        const QRect& bounds,
        bool showLabels);

    DisplayMetadata recordedMetadata_;
    DisplayMetadata currentMetadata_;

    bool hasRecorded_ = false;
    bool hasCurrent_ = false;
    bool showBoth_ = true;
};

class DisplayInfoDialog
    : public QDialog
{
    Q_OBJECT

public:
    explicit DisplayInfoDialog(
        Recording* recording,
        QWidget* parent = nullptr);

private:
    void setupUi();
    void updateDisplay();

    void addMonitorInfo(
        QGridLayout* grid,
        size_t index,
        const MonitorMetadata& monitor,
        const char* tone);

    QString formatResolution(
        int width,
        int height) const;

    QString formatPosition(
        int x,
        int y) const;

    Recording* recording_ = nullptr;

    DisplayMetadata currentMetadata_;
    bool hasCurrentMetadata_ = false;

    // UI elements.
    QLabel* statusLabel_ = nullptr;
    QLabel* statusIconLabel_ = nullptr;
    QLabel* compatMessageLabel_ = nullptr;

    // Recorded display information.
    QLabel* recordedVirtualDesktopLabel_ = nullptr;
    QLabel* recordedMonitorCountLabel_ = nullptr;
    QGridLayout* recordedMonitorsGrid_ = nullptr;

    // Current display information.
    QLabel* currentVirtualDesktopLabel_ = nullptr;
    QLabel* currentMonitorCountLabel_ = nullptr;
    QGridLayout* currentMonitorsGrid_ = nullptr;

    // Visual diagram.
    MonitorDiagramWidget* diagramWidget_ = nullptr;
};