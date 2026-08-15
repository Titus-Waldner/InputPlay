#pragma once

#include "Recording.h"

#include <QWidget>

class TimelineWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TimelineWidget(QWidget* parent = nullptr);
    
    void setRecording(Recording* recording);
    
    int selectedEventIndex() const { return selectedIndex_; }

signals:
    void eventSelected(int index);
    void eventDoubleClicked(int index);

public slots:
    void selectEvent(int index);
    void highlightEvent(int index);
    void setZoom(double zoom);
    void setPlayheadIndex(int index);
    void clearPlayhead();

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    void updateLayout();
    int eventAtPosition(const QPoint& pos) const;
    QRect eventRect(int index) const;
    QColor eventColor(EventType type) const;
    
    Recording* recording_ = nullptr;
    int selectedIndex_ = -1;
    int hoveredIndex_ = -1;
    int playheadIndex_ = -1;
    
    double zoom_ = 1.0;
    double scrollOffset_ = 0.0;
    
    static constexpr int TrackHeight = 30;
    static constexpr int EventMinWidth = 4;
    static constexpr int Margin = 20;
};
