#pragma once

#include "InputEvent.h"
#include "Recording.h"

#include <QUndoCommand>
#include <vector>

class EventListModel;

// Base class for event commands that need model reference
class EventCommand : public QUndoCommand
{
public:
    EventCommand(EventListModel* model, const QString& text, QUndoCommand* parent = nullptr);
    
protected:
    EventListModel* model_;
};

// Add event at end
class AddEventCommand : public EventCommand
{
public:
    AddEventCommand(EventListModel* model, const InputEvent& event, QUndoCommand* parent = nullptr);
    
    void undo() override;
    void redo() override;
    
private:
    InputEvent event_;
    int row_ = -1;
};

// Insert event at position
class InsertEventCommand : public EventCommand
{
public:
    InsertEventCommand(EventListModel* model, int row, const InputEvent& event, QUndoCommand* parent = nullptr);
    
    void undo() override;
    void redo() override;
    
private:
    InputEvent event_;
    int row_;
};

// Remove single event
class RemoveEventCommand : public EventCommand
{
public:
    RemoveEventCommand(EventListModel* model, int row, QUndoCommand* parent = nullptr);
    
    void undo() override;
    void redo() override;
    
private:
    InputEvent event_;
    int row_;
    bool eventStored_ = false;
};

// Remove multiple events
class RemoveEventsCommand : public EventCommand
{
public:
    RemoveEventsCommand(EventListModel* model, const QList<int>& rows, QUndoCommand* parent = nullptr);
    
    void undo() override;
    void redo() override;
    
private:
    struct StoredEvent {
        int row;
        InputEvent event;
    };
    
    QList<int> rows_;
    std::vector<StoredEvent> storedEvents_;
};

// Modify event properties
class ModifyEventCommand : public EventCommand
{
public:
    ModifyEventCommand(EventListModel* model, int row, const InputEvent& oldEvent, 
                       const InputEvent& newEvent, QUndoCommand* parent = nullptr);
    
    void undo() override;
    void redo() override;
    
    int id() const override { return 1; }
    bool mergeWith(const QUndoCommand* other) override;
    
private:
    int row_;
    InputEvent oldEvent_;
    InputEvent newEvent_;
};

// Batch modify (e.g., scale timing)
class BatchModifyCommand : public EventCommand
{
public:
    BatchModifyCommand(EventListModel* model, const QString& description,
                       const std::vector<std::pair<int, InputEvent>>& oldEvents,
                       const std::vector<std::pair<int, InputEvent>>& newEvents,
                       QUndoCommand* parent = nullptr);
    
    void undo() override;
    void redo() override;
    
private:
    std::vector<std::pair<int, InputEvent>> oldEvents_;
    std::vector<std::pair<int, InputEvent>> newEvents_;
};

// Clear all events
class ClearEventsCommand : public EventCommand
{
public:
    ClearEventsCommand(EventListModel* model, QUndoCommand* parent = nullptr);
    
    void undo() override;
    void redo() override;
    
private:
    std::vector<InputEvent> events_;
    int startX_ = 0;
    int startY_ = 0;
    bool hasStartPos_ = false;
};

// Move event up or down
class MoveEventCommand : public EventCommand
{
public:
    MoveEventCommand(EventListModel* model, int fromRow, int toRow, QUndoCommand* parent = nullptr);
    
    void undo() override;
    void redo() override;
    
private:
    int fromRow_;
    int toRow_;
};
