#include "EventListModel.h"
#include "DarkStyle.h"

#include <QColor>
#include <QBrush>
#include <QFont>

EventListModel::EventListModel(QObject* parent)
    : QAbstractTableModel(parent)
{
}

void EventListModel::setRecording(Recording* recording)
{
    beginResetModel();
    recording_ = recording;
    endResetModel();
}

int EventListModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid() || !recording_) {
        return 0;
    }
    return static_cast<int>(recording_->eventCount());
}

int EventListModel::columnCount(const QModelIndex& parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return ColumnCount;
}

QVariant EventListModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || !recording_) {
        return QVariant();
    }
    
    int row = index.row();
    if (row < 0 || row >= static_cast<int>(recording_->eventCount())) {
        return QVariant();
    }
    
    const InputEvent& event = recording_->events()[row];
    
    switch (role) {
        case Qt::DisplayRole:
            switch (index.column()) {
                case ColumnIndex:
                    return row + 1;
                case ColumnType:
                    return eventTypeToString(event.type);
                case ColumnDetails:
                    return eventDetailsString(event);
                case ColumnTimestamp:
                    return formatTimestamp(event.timestampMicroseconds);
                case ColumnDuration:
                    if (row + 1 < static_cast<int>(recording_->eventCount())) {
                        std::uint64_t duration = recording_->events()[row + 1].timestampMicroseconds 
                                               - event.timestampMicroseconds;
                        return formatDuration(duration);
                    }
                    return "-";
                default:
                    return QVariant();
            }
            
        case Qt::ForegroundRole:
            if (index.column() == ColumnType) {
                return QBrush(eventTypeColor(event.type));
            }
            return QVariant();
            
        case Qt::BackgroundRole:
            // Subtle background tint based on event type
            if (colorCodedRows_) {
                QColor bgColor = eventTypeColor(event.type);
                bgColor.setAlpha(20); // Very subtle
                return QBrush(bgColor);
            }
            return QVariant();
            
        case Qt::FontRole:
            if (index.column() == ColumnType) {
                QFont font;
                font.setBold(true);
                return font;
            }
            return QVariant();
            
        case Qt::TextAlignmentRole:
            if (index.column() == ColumnIndex || 
                index.column() == ColumnTimestamp ||
                index.column() == ColumnDuration) {
                return static_cast<int>(
					Qt::AlignRight | Qt::AlignVCenter);
            }
            return static_cast<int>(
				Qt::AlignLeft | Qt::AlignVCenter);
            
        case Qt::UserRole:
            // Return the event type for filtering/sorting
            return static_cast<int>(event.type);
            
        default:
            return QVariant();
    }
}

QVariant EventListModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole) {
        return QVariant();
    }
    
    switch (section) {
        case ColumnIndex:
            return tr("#");
        case ColumnType:
            return tr("Type");
        case ColumnDetails:
            return tr("Details");
        case ColumnTimestamp:
            return tr("Time");
        case ColumnDuration:
            return tr("Duration");
        default:
            return QVariant();
    }
}

Qt::ItemFlags EventListModel::flags(const QModelIndex& index) const
{
    if (!index.isValid()) {
        return Qt::NoItemFlags;
    }
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

bool EventListModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    Q_UNUSED(index);
    Q_UNUSED(value);
    Q_UNUSED(role);
    // We handle editing through PropertyEditor, not inline
    return false;
}

void EventListModel::addEvent(const InputEvent& event)
{
    if (!recording_) return;
    
    int row = static_cast<int>(recording_->eventCount());
    beginInsertRows(QModelIndex(), row, row);
    recording_->addEvent(event);
    endInsertRows();
    
    emit eventAdded(row);
}

void EventListModel::insertEvent(int row, const InputEvent& event)
{
    if (!recording_) return;
    
    if (row < 0 || row > static_cast<int>(recording_->eventCount())) {
        return;
    }
    
    beginInsertRows(QModelIndex(), row, row);
    
    // Need to insert into the recording's events vector
    // Since Recording doesn't expose insert, we need to rebuild
    std::vector<InputEvent> events = recording_->events();
    events.insert(events.begin() + row, event);
    recording_->clear();
    for (const auto& e : events) {
        recording_->addEvent(e);
    }
    
    endInsertRows();
    emit eventAdded(row);
}

void EventListModel::removeEvent(int row)
{
    if (!recording_) return;
    
    if (row < 0 || row >= static_cast<int>(recording_->eventCount())) {
        return;
    }
    
    beginRemoveRows(QModelIndex(), row, row);
    
    std::vector<InputEvent> events = recording_->events();
    events.erase(events.begin() + row);
    recording_->clear();
    for (const auto& e : events) {
        recording_->addEvent(e);
    }
    
    endRemoveRows();
    emit eventRemoved(row);
}

void EventListModel::removeEvents(const QList<int>& rows)
{
    if (!recording_ || rows.isEmpty()) return;
    
    // Sort in descending order to remove from back first
    QList<int> sortedRows = rows;
    std::sort(sortedRows.begin(), sortedRows.end(), std::greater<int>());
    
    for (int row : sortedRows) {
        removeEvent(row);
    }
}

void EventListModel::updateEvent(int row, const InputEvent& event)
{
    if (!recording_) return;
    
    if (row < 0 || row >= static_cast<int>(recording_->eventCount())) {
        return;
    }
    
    std::vector<InputEvent> events = recording_->events();
    events[row] = event;
    recording_->clear();
    for (const auto& e : events) {
        recording_->addEvent(e);
    }
    
    emit dataChanged(index(row, 0), index(row, ColumnCount - 1));
    emit eventUpdated(row);
}

void EventListModel::refreshEvent(int row)
{
    if (row >= 0 && row < rowCount()) {
        emit dataChanged(index(row, 0), index(row, ColumnCount - 1));
    }
}

void EventListModel::clear()
{
    if (!recording_) return;
    
    beginResetModel();
    recording_->clear();
    endResetModel();
}

const InputEvent* EventListModel::eventAt(int row) const
{
    if (!recording_ || row < 0 || row >= static_cast<int>(recording_->eventCount())) {
        return nullptr;
    }
    return &recording_->events()[row];
}

QString EventListModel::eventTypeToString(EventType type) const
{
    switch (type) {
        case EventType::MouseMove:
            return tr("Mouse Move");
        case EventType::MouseButtonDown:
            return tr("Mouse Down");
        case EventType::MouseButtonUp:
            return tr("Mouse Up");
        case EventType::MouseWheel:
            return tr("Mouse Wheel");
        case EventType::KeyDown:
            return tr("Key Down");
        case EventType::KeyUp:
            return tr("Key Up");
        case EventType::Wait:
            return tr("Wait");
        case EventType::MouseTeleport:
            return tr("Teleport");
        default:
            return tr("Unknown");
    }
}

QString EventListModel::eventDetailsString(const InputEvent& event) const
{
    switch (event.type) {
        case EventType::MouseMove:
            return tr("Δ(%1, %2) → (%3, %4)")
                .arg(event.mouseDeltaX)
                .arg(event.mouseDeltaY)
                .arg(event.mouseX)
                .arg(event.mouseY);
                
        case EventType::MouseButtonDown:
        case EventType::MouseButtonUp:
            {
                QString button;
                switch (event.mouseButton) {
                    case 1: button = tr("Left"); break;
                    case 2: button = tr("Right"); break;
                    case 3: button = tr("Middle"); break;
                    case 4: button = tr("X1"); break;
                    case 5: button = tr("X2"); break;
                    default: button = tr("Button %1").arg(event.mouseButton);
                }
                return tr("%1 at (%2, %3)")
                    .arg(button)
                    .arg(event.mouseX)
                    .arg(event.mouseY);
            }
            
        case EventType::MouseWheel:
            return tr("Delta: %1 at (%2, %3)")
                .arg(event.mouseWheelDelta)
                .arg(event.mouseX)
                .arg(event.mouseY);
                
        case EventType::KeyDown:
        case EventType::KeyUp:
            {
                // Get key name from virtual key code
                QString keyName = QString("0x%1").arg(event.keyCode, 2, 16, QChar('0')).toUpper();
                
                // Common key names
                switch (event.keyCode) {
                    case 0x08: keyName = "Backspace"; break;
                    case 0x09: keyName = "Tab"; break;
                    case 0x0D: keyName = "Enter"; break;
                    case 0x10: keyName = "Shift"; break;
                    case 0x11: keyName = "Ctrl"; break;
                    case 0x12: keyName = "Alt"; break;
                    case 0x1B: keyName = "Escape"; break;
                    case 0x20: keyName = "Space"; break;
                    case 0x25: keyName = "Left"; break;
                    case 0x26: keyName = "Up"; break;
                    case 0x27: keyName = "Right"; break;
                    case 0x28: keyName = "Down"; break;
                    case 0x2E: keyName = "Delete"; break;
                    default:
                        if (event.keyCode >= 0x30 && event.keyCode <= 0x39) {
                            keyName = QChar('0' + (event.keyCode - 0x30));
                        } else if (event.keyCode >= 0x41 && event.keyCode <= 0x5A) {
                            keyName = QChar('A' + (event.keyCode - 0x41));
                        } else if (event.keyCode >= 0x70 && event.keyCode <= 0x87) {
                            keyName = tr("F%1").arg(event.keyCode - 0x70 + 1);
                        }
                        break;
                }
                
                return tr("Key: %1 (0x%2)")
                    .arg(keyName)
                    .arg(event.keyCode, 2, 16, QChar('0')).toUpper();
            }
            
        case EventType::Wait:
            return formatDuration(event.waitMicroseconds);
            
        case EventType::MouseTeleport:
            return tr("→ (%1, %2)")
                .arg(event.mouseX)
                .arg(event.mouseY);
                
        default:
            return QString();
    }
}

QColor EventListModel::eventTypeColor(
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

void EventListModel::setColorCodedRows(bool enabled)
{
    if (colorCodedRows_ != enabled) {
        colorCodedRows_ = enabled;
        if (recording_ && !recording_->empty()) {
            emit dataChanged(index(0, 0), index(rowCount() - 1, columnCount() - 1));
        }
    }
}

QString EventListModel::formatTimestamp(std::uint64_t microseconds) const
{
    double seconds = microseconds / 1'000'000.0;
    
    if (seconds < 60.0) {
        return tr("%1s").arg(seconds, 0, 'f', 3);
    }
    
    int minutes = static_cast<int>(seconds / 60.0);
    double remainingSeconds = seconds - (minutes * 60.0);
    
    return tr("%1m %2s").arg(minutes).arg(remainingSeconds, 0, 'f', 1);
}

QString EventListModel::formatDuration(std::uint64_t microseconds) const
{
    if (microseconds < 1000) {
        return tr("%1 µs").arg(microseconds);
    }
    
    if (microseconds < 1'000'000) {
        return tr("%1 ms").arg(microseconds / 1000.0, 0, 'f', 1);
    }
    
    return tr("%1 s").arg(microseconds / 1'000'000.0, 0, 'f', 3);
}

// Internal methods for undo commands
int EventListModel::addEventInternal(const InputEvent& event)
{
    if (!recording_) return -1;
    
    int row = static_cast<int>(recording_->eventCount());
    beginInsertRows(QModelIndex(), row, row);
    recording_->addEvent(event);
    endInsertRows();
    
    return row;
}

void EventListModel::insertEventInternal(int row, const InputEvent& event)
{
    if (!recording_) return;
    
    if (row < 0 || row > static_cast<int>(recording_->eventCount())) {
        return;
    }
    
    beginInsertRows(QModelIndex(), row, row);
    
    std::vector<InputEvent> events = recording_->events();
    events.insert(events.begin() + row, event);
    recording_->clear();
    for (const auto& e : events) {
        recording_->addEvent(e);
    }
    
    endInsertRows();
}

void EventListModel::removeEventInternal(int row)
{
    if (!recording_) return;
    
    if (row < 0 || row >= static_cast<int>(recording_->eventCount())) {
        return;
    }
    
    beginRemoveRows(QModelIndex(), row, row);
    
    std::vector<InputEvent> events = recording_->events();
    events.erase(events.begin() + row);
    recording_->clear();
    for (const auto& e : events) {
        recording_->addEvent(e);
    }
    
    endRemoveRows();
}

void EventListModel::updateEventInternal(int row, const InputEvent& event)
{
    if (!recording_) return;
    
    if (row < 0 || row >= static_cast<int>(recording_->eventCount())) {
        return;
    }
    
    std::vector<InputEvent> events = recording_->events();
    events[row] = event;
    recording_->clear();
    for (const auto& e : events) {
        recording_->addEvent(e);
    }
    
    emit dataChanged(index(row, 0), index(row, ColumnCount - 1));
}

void EventListModel::clearInternal()
{
    if (!recording_) return;
    
    beginResetModel();
    recording_->clear();
    endResetModel();
}

void EventListModel::moveEventInternal(int fromRow, int toRow)
{
    if (!recording_) return;
    
    int eventCount = static_cast<int>(recording_->eventCount());
    if (fromRow < 0 || fromRow >= eventCount || toRow < 0 || toRow >= eventCount) {
        return;
    }
    
    if (fromRow == toRow) {
        return;
    }
    
    // Get events and perform move
    std::vector<InputEvent> events = recording_->events();
    InputEvent movedEvent = events[fromRow];
    events.erase(events.begin() + fromRow);
    events.insert(events.begin() + toRow, movedEvent);
    
    // Update model with proper notifications
    beginMoveRows(QModelIndex(), fromRow, fromRow, QModelIndex(), toRow > fromRow ? toRow + 1 : toRow);
    
    recording_->clear();
    for (const auto& e : events) {
        recording_->addEvent(e);
    }
    
    endMoveRows();
    
    emit eventMoved(fromRow, toRow);
}
