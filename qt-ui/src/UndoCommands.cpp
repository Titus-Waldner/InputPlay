#include "UndoCommands.h"
#include "EventListModel.h"

// EventCommand base
EventCommand::EventCommand(EventListModel* model, const QString& text, QUndoCommand* parent)
    : QUndoCommand(text, parent)
    , model_(model)
{
}

// AddEventCommand
AddEventCommand::AddEventCommand(EventListModel* model, const InputEvent& event, QUndoCommand* parent)
    : EventCommand(model, QObject::tr("Add Event"), parent)
    , event_(event)
{
}

void AddEventCommand::undo()
{
    if (row_ >= 0) {
        model_->removeEventInternal(row_);
    }
}

void AddEventCommand::redo()
{
    row_ = model_->addEventInternal(event_);
}

// InsertEventCommand
InsertEventCommand::InsertEventCommand(EventListModel* model, int row, const InputEvent& event, QUndoCommand* parent)
    : EventCommand(model, QObject::tr("Insert Event"), parent)
    , event_(event)
    , row_(row)
{
}

void InsertEventCommand::undo()
{
    model_->removeEventInternal(row_);
}

void InsertEventCommand::redo()
{
    model_->insertEventInternal(row_, event_);
}

// RemoveEventCommand
RemoveEventCommand::RemoveEventCommand(EventListModel* model, int row, QUndoCommand* parent)
    : EventCommand(model, QObject::tr("Delete Event"), parent)
    , row_(row)
{
}

void RemoveEventCommand::undo()
{
    model_->insertEventInternal(row_, event_);
}

void RemoveEventCommand::redo()
{
    if (!eventStored_) {
        const InputEvent* evt = model_->eventAt(row_);
        if (evt) {
            event_ = *evt;
            eventStored_ = true;
        }
    }
    model_->removeEventInternal(row_);
}

// RemoveEventsCommand
RemoveEventsCommand::RemoveEventsCommand(EventListModel* model, const QList<int>& rows, QUndoCommand* parent)
    : EventCommand(model, QObject::tr("Delete %1 Events").arg(rows.size()), parent)
    , rows_(rows)
{
    // Sort descending for proper removal order
    std::sort(rows_.begin(), rows_.end(), std::greater<int>());
}

void RemoveEventsCommand::undo()
{
    // Restore in ascending order
    for (auto it = storedEvents_.rbegin(); it != storedEvents_.rend(); ++it) {
        model_->insertEventInternal(it->row, it->event);
    }
}

void RemoveEventsCommand::redo()
{
    if (storedEvents_.empty()) {
        // Store events before removal (in descending order)
        for (int row : rows_) {
            const InputEvent* evt = model_->eventAt(row);
            if (evt) {
                storedEvents_.push_back({row, *evt});
            }
        }
    }
    
    // Remove in descending order
    for (int row : rows_) {
        model_->removeEventInternal(row);
    }
}

// ModifyEventCommand
ModifyEventCommand::ModifyEventCommand(EventListModel* model, int row, 
                                       const InputEvent& oldEvent, const InputEvent& newEvent,
                                       QUndoCommand* parent)
    : EventCommand(model, QObject::tr("Modify Event"), parent)
    , row_(row)
    , oldEvent_(oldEvent)
    , newEvent_(newEvent)
{
}

void ModifyEventCommand::undo()
{
    model_->updateEventInternal(row_, oldEvent_);
}

void ModifyEventCommand::redo()
{
    model_->updateEventInternal(row_, newEvent_);
}

bool ModifyEventCommand::mergeWith(const QUndoCommand* other)
{
    if (other->id() != id()) {
        return false;
    }
    
    const ModifyEventCommand* cmd = static_cast<const ModifyEventCommand*>(other);
    if (cmd->row_ != row_) {
        return false;
    }
    
    // Keep original old state, update to newest state
    newEvent_ = cmd->newEvent_;
    return true;
}

// BatchModifyCommand
BatchModifyCommand::BatchModifyCommand(EventListModel* model, const QString& description,
                                       const std::vector<std::pair<int, InputEvent>>& oldEvents,
                                       const std::vector<std::pair<int, InputEvent>>& newEvents,
                                       QUndoCommand* parent)
    : EventCommand(model, description, parent)
    , oldEvents_(oldEvents)
    , newEvents_(newEvents)
{
}

void BatchModifyCommand::undo()
{
    for (const auto& [row, event] : oldEvents_) {
        model_->updateEventInternal(row, event);
    }
}

void BatchModifyCommand::redo()
{
    for (const auto& [row, event] : newEvents_) {
        model_->updateEventInternal(row, event);
    }
}

// ClearEventsCommand
ClearEventsCommand::ClearEventsCommand(EventListModel* model, QUndoCommand* parent)
    : EventCommand(model, QObject::tr("Clear All Events"), parent)
{
}

void ClearEventsCommand::undo()
{
    Recording* recording = model_->recording();
    if (!recording) return;
    
    for (const auto& event : events_) {
        model_->addEventInternal(event);
    }
    
    if (hasStartPos_) {
        recording->setStartingCursorPosition(startX_, startY_);
    }
}

void ClearEventsCommand::redo()
{
    Recording* recording = model_->recording();
    if (!recording) return;
    
    // Store events before clear
    if (events_.empty() && recording->eventCount() > 0) {
        events_ = recording->events();
        hasStartPos_ = recording->hasStartingCursorPosition();
        if (hasStartPos_) {
            startX_ = recording->startingCursorX();
            startY_ = recording->startingCursorY();
        }
    }
    
    model_->clearInternal();
}

// MoveEventCommand
MoveEventCommand::MoveEventCommand(EventListModel* model, int fromRow, int toRow, QUndoCommand* parent)
    : EventCommand(model, QObject::tr("Move Event"), parent)
    , fromRow_(fromRow)
    , toRow_(toRow)
{
}

void MoveEventCommand::undo()
{
    model_->moveEventInternal(toRow_, fromRow_);
}

void MoveEventCommand::redo()
{
    model_->moveEventInternal(fromRow_, toRow_);
}
