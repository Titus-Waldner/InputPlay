#pragma once

#include "Recording.h"
#include "InputEvent.h"

#include <QAbstractTableModel>
#include <QString>

class EventListModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    enum Column {
        ColumnIndex = 0,
        ColumnType,
        ColumnDetails,
        ColumnTimestamp,
        ColumnDuration,
        ColumnCount
    };

    explicit EventListModel(QObject* parent = nullptr);
    
    void setRecording(Recording* recording);
    Recording* recording() const { return recording_; }
    
    // QAbstractTableModel interface
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    
    // Editing support
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
    
    // Operations
    void addEvent(const InputEvent& event);
    void insertEvent(int row, const InputEvent& event);
    void removeEvent(int row);
    void removeEvents(const QList<int>& rows);
    void updateEvent(int row, const InputEvent& event);
    void refreshEvent(int row);
    void clear();
    
    // Helpers
    const InputEvent* eventAt(int row) const;
    QString eventTypeToString(EventType type) const;
    QString eventDetailsString(const InputEvent& event) const;
    QColor eventTypeColor(EventType type) const;
    
    // Display options
    void setColorCodedRows(bool enabled);
    bool colorCodedRows() const { return colorCodedRows_; }
    
    // Internal methods for undo commands (don't emit signals for undo stack)
    int addEventInternal(const InputEvent& event);
    void insertEventInternal(int row, const InputEvent& event);
    void removeEventInternal(int row);
    void updateEventInternal(int row, const InputEvent& event);
    void moveEventInternal(int fromRow, int toRow);
    void clearInternal();

signals:
    void eventAdded(int row);
    void eventRemoved(int row);
    void eventUpdated(int row);
    void eventMoved(int fromRow, int toRow);

private:
    Recording* recording_ = nullptr;
    bool colorCodedRows_ = true;
    
    friend class AddEventCommand;
    friend class InsertEventCommand;
    friend class RemoveEventCommand;
    friend class RemoveEventsCommand;
    friend class ModifyEventCommand;
    friend class BatchModifyCommand;
    friend class ClearEventsCommand;
    friend class MoveEventCommand;
    
    QString formatTimestamp(std::uint64_t microseconds) const;
    QString formatDuration(std::uint64_t microseconds) const;
};
