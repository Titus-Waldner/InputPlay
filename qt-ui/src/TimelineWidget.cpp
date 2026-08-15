#include "TimelineWidget.h"
#include "DarkStyle.h"

#include <QPainter>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QToolTip>

TimelineWidget::TimelineWidget(QWidget* parent)
    : QWidget(parent)
{
    setMouseTracking(true);
    setMinimumHeight(150);
    setFocusPolicy(Qt::StrongFocus);
}

void TimelineWidget::setRecording(Recording* recording)
{
    recording_ = recording;
    selectedIndex_ = -1;
    hoveredIndex_ = -1;
    playheadIndex_ = -1;
    scrollOffset_ = 0.0;
    updateLayout();
    update();
}

void TimelineWidget::selectEvent(int index)
{
    if (selectedIndex_ != index) {
        selectedIndex_ = index;
        update();
        emit eventSelected(index);
    }
}

void TimelineWidget::highlightEvent(int index)
{
    // Same as selectEvent but doesn't emit signal (used for playback highlighting)
    if (selectedIndex_ != index) {
        selectedIndex_ = index;
        update();
    }
}

void TimelineWidget::setZoom(double zoom)
{
    zoom_ = qBound(0.1, zoom, 10.0);
    updateLayout();
    update();
}

void TimelineWidget::setPlayheadIndex(int index)
{
    if (playheadIndex_ != index) {
        playheadIndex_ = index;
        update();
    }
}

void TimelineWidget::clearPlayhead()
{
    if (playheadIndex_ != -1) {
        playheadIndex_ = -1;
        update();
    }
}

void TimelineWidget::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // Background
	painter.fillRect(
		rect(),
		QColor(
			DarkStyle::bgSecondary()));
    
    if (!recording_ || recording_->empty()) {
        // Draw placeholder text
        painter.setPen(
		DarkStyle::toneColor(
			"secondary"));
        painter.drawText(rect(), Qt::AlignCenter, tr("No events to display"));
        return;
    }
    
    const auto& events = recording_->events();
    std::uint64_t totalDuration = events.back().timestampMicroseconds;
    if (totalDuration == 0) totalDuration = 1;
    
    int contentWidth = width() - 2 * Margin;
    
    // Draw time ruler
	painter.setPen(
		QColor(
			DarkStyle::border()));
    painter.drawLine(Margin, 30, width() - Margin, 30);
    
    // Draw time marks
    painter.setPen(
		DarkStyle::toneColor(
			"secondary"));
    QFont smallFont = painter.font();
    smallFont.setPointSize(8);
    painter.setFont(smallFont);
    
    double secondsTotal = totalDuration / 1'000'000.0;
    double interval = 1.0; // 1 second intervals
    if (secondsTotal > 60) interval = 10.0;
    if (secondsTotal > 300) interval = 30.0;
    if (secondsTotal < 5) interval = 0.5;
    if (secondsTotal < 1) interval = 0.1;
    
    for (double t = 0; t <= secondsTotal; t += interval) {
        int x = Margin + static_cast<int>((t / secondsTotal) * contentWidth * zoom_ - scrollOffset_);
        if (x >= Margin && x <= width() - Margin) {
            painter.drawLine(x, 25, x, 35);
            
            QString label;
            if (interval < 1.0) {
                label = QString("%1ms").arg(static_cast<int>(t * 1000));
            } else {
                label = QString("%1s").arg(t, 0, 'f', interval < 1 ? 1 : 0);
            }
            painter.drawText(x - 20, 10, 40, 15, Qt::AlignCenter, label);
        }
    }
    
    // Draw events
    int trackY = 50;
    
    for (std::size_t i = 0; i < events.size(); ++i) {
        const InputEvent& ev = events[i];
        
        // Calculate position and width
        double startRatio = static_cast<double>(ev.timestampMicroseconds) / totalDuration;
        int x = Margin + static_cast<int>(startRatio * contentWidth * zoom_ - scrollOffset_);
        
        // Calculate width based on duration to next event
        int eventWidth = EventMinWidth;
        if (i + 1 < events.size()) {
            std::uint64_t duration = events[i + 1].timestampMicroseconds - ev.timestampMicroseconds;
            double durationRatio = static_cast<double>(duration) / totalDuration;
            eventWidth = qMax(EventMinWidth, static_cast<int>(durationRatio * contentWidth * zoom_));
        }
        
        // Skip if out of view
        if (x + eventWidth < Margin || x > width() - Margin) {
            continue;
        }
        
        // Get color for event type
        QColor color = eventColor(ev.type);
        
        // Highlight selected or hovered events.
		if (static_cast<int>(i) == selectedIndex_)
		{
			painter.setPen(
				QPen(
					DarkStyle::toneColor(
						"accent"),
					2));

			color =
				DarkStyle::toneColor(
					"accentLight");
		}
		else if (static_cast<int>(i) == hoveredIndex_)
		{
			painter.setPen(
				QPen(
					DarkStyle::toneColor(
						"accentLight"),
					1));

			color =
				color.lighter(120);
		}
		else
		{
			painter.setPen(
				Qt::NoPen);
		}
        
        painter.setBrush(color);
        
        // Draw event rect
        QRect eventRect(x, trackY, eventWidth, TrackHeight);
        painter.drawRoundedRect(eventRect, 3, 3);
    }
    
    // Draw playhead if active
    if (playheadIndex_ >= 0 && playheadIndex_ < static_cast<int>(events.size())) {
        const InputEvent& ev = events[playheadIndex_];
        double ratio = static_cast<double>(ev.timestampMicroseconds) / totalDuration;
        int playheadX = Margin + static_cast<int>(ratio * contentWidth * zoom_ - scrollOffset_);
        
        // Draw playhead line
        painter.setPen(
			QPen(
				DarkStyle::toneColor(
					"accentDark"),
				2));
        painter.drawLine(playheadX, 15, playheadX, trackY + TrackHeight + 10);
        
        // Draw playhead triangle at top
        QPolygonF triangle;
        triangle << QPointF(playheadX - 6, 10)
                 << QPointF(playheadX + 6, 10)
                 << QPointF(playheadX, 18);
        painter.setBrush(
			DarkStyle::toneColor(
				"accentDark"));
        painter.setPen(Qt::NoPen);
        painter.drawPolygon(triangle);
    }
}

void TimelineWidget::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        int index = eventAtPosition(event->pos());
        selectEvent(index);
    }
}

void TimelineWidget::mouseDoubleClickEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        int index = eventAtPosition(event->pos());
        if (index >= 0) {
            emit eventDoubleClicked(index);
        }
    }
}

void TimelineWidget::mouseMoveEvent(QMouseEvent* event)
{
    int index = eventAtPosition(event->pos());
    
    if (index != hoveredIndex_) {
        hoveredIndex_ = index;
        update();
        
        // Show tooltip
        if (index >= 0 && recording_) {
            const InputEvent& ev = recording_->events()[index];
            QString tooltip;
            
            switch (ev.type) {
                case EventType::MouseMove:
                    tooltip = tr("Mouse Move: Δ(%1, %2)")
                        .arg(ev.mouseDeltaX).arg(ev.mouseDeltaY);
                    break;
                case EventType::MouseButtonDown:
                    tooltip = tr("Mouse Down: Button %1").arg(ev.mouseButton);
                    break;
                case EventType::MouseButtonUp:
                    tooltip = tr("Mouse Up: Button %1").arg(ev.mouseButton);
                    break;
                case EventType::MouseWheel:
                    tooltip = tr("Mouse Wheel: %1").arg(ev.mouseWheelDelta);
                    break;
                case EventType::KeyDown:
                    tooltip = tr("Key Down: 0x%1").arg(ev.keyCode, 2, 16, QChar('0')).toUpper();
                    break;
                case EventType::KeyUp:
                    tooltip = tr("Key Up: 0x%1").arg(ev.keyCode, 2, 16, QChar('0')).toUpper();
                    break;
                case EventType::Wait:
                    tooltip = tr("Wait: %1 ms").arg(ev.waitMicroseconds / 1000);
                    break;
                case EventType::MouseTeleport:
                    tooltip = tr("Teleport: (%1, %2)").arg(ev.mouseX).arg(ev.mouseY);
                    break;
            }
            
            tooltip += tr("\nTime: %1 ms").arg(ev.timestampMicroseconds / 1000);
            QToolTip::showText(event->globalPosition().toPoint(), tooltip, this);
        }
    }
}

void TimelineWidget::wheelEvent(QWheelEvent* event)
{
    if (event->modifiers() & Qt::ControlModifier) {
        // Zoom
        double delta = event->angleDelta().y() / 120.0;
        setZoom(zoom_ * (1.0 + delta * 0.1));
    } else {
        // Scroll
        scrollOffset_ -= event->angleDelta().y() / 2.0;
        scrollOffset_ = qMax(0.0, scrollOffset_);
        update();
    }
    
    event->accept();
}

void TimelineWidget::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    updateLayout();
}

void TimelineWidget::updateLayout()
{
    // Recalculate layout based on current zoom and size
    update();
}

int TimelineWidget::eventAtPosition(const QPoint& pos) const
{
    if (!recording_ || recording_->empty()) {
        return -1;
    }
    
    const auto& events = recording_->events();
    std::uint64_t totalDuration = events.back().timestampMicroseconds;
    if (totalDuration == 0) totalDuration = 1;
    
    int contentWidth = width() - 2 * Margin;
    int trackY = 50;
    
    // Check if in track area
    if (pos.y() < trackY || pos.y() > trackY + TrackHeight) {
        return -1;
    }
    
    // Find event at x position
    for (std::size_t i = 0; i < events.size(); ++i) {
        const InputEvent& ev = events[i];
        
        double startRatio = static_cast<double>(ev.timestampMicroseconds) / totalDuration;
        int x = Margin + static_cast<int>(startRatio * contentWidth * zoom_ - scrollOffset_);
        
        int eventWidth = EventMinWidth;
        if (i + 1 < events.size()) {
            std::uint64_t duration = events[i + 1].timestampMicroseconds - ev.timestampMicroseconds;
            double durationRatio = static_cast<double>(duration) / totalDuration;
            eventWidth = qMax(EventMinWidth, static_cast<int>(durationRatio * contentWidth * zoom_));
        }
        
        if (pos.x() >= x && pos.x() <= x + eventWidth) {
            return static_cast<int>(i);
        }
    }
    
    return -1;
}

QColor TimelineWidget::eventColor(
    EventType type) const
{
    switch (type)
    {
        case EventType::MouseMove:
            return DarkStyle::eventColor(
                "mouseMove");

        case EventType::MouseButtonDown:
        case EventType::MouseButtonUp:
            return DarkStyle::eventColor(
                "mouseClick");

        case EventType::MouseWheel:
            return DarkStyle::eventColor(
                "mouseWheel");

        case EventType::KeyDown:
        case EventType::KeyUp:
            return DarkStyle::eventColor(
                "keyboard");

        case EventType::Wait:
            return DarkStyle::eventColor(
                "wait");

        case EventType::MouseTeleport:
            return DarkStyle::eventColor(
                "teleport");

        default:
            return DarkStyle::toneColor(
                "primary");
    }
}