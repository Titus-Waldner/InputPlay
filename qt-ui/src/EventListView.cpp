#include "EventListView.h"
#include "EventListModel.h"

#include <QHeaderView>
#include <QContextMenuEvent>
#include <QMouseEvent>

EventListView::EventListView(QWidget* parent)
    : QTableView(parent)
{
    setupUi();
    setupContextMenu();
}

void EventListView::setupUi()
{
    // Selection behavior
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    
    // Appearance
    setAlternatingRowColors(true);
    setShowGrid(false);
    setSortingEnabled(false);
    setWordWrap(false);
    
    // Header setup
    horizontalHeader()->setStretchLastSection(false);
    horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    horizontalHeader()->setHighlightSections(false);
    
    verticalHeader()->setVisible(false);
    verticalHeader()->setDefaultSectionSize(28);
    
    // Scrolling
    setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
}

void EventListView::setupContextMenu()
{
    contextMenu_ = new QMenu(this);
    
    addBeforeAction_ = contextMenu_->addAction(tr("Insert Event Before"));
    connect(addBeforeAction_, &QAction::triggered, this, &EventListView::addEventBefore);
    
    addAfterAction_ = contextMenu_->addAction(tr("Insert Event After"));
    connect(addAfterAction_, &QAction::triggered, this, &EventListView::addEventAfter);
    
    addAtEndAction_ = contextMenu_->addAction(tr("Add Event at End"));
    connect(addAtEndAction_, &QAction::triggered, this, &EventListView::addEventAtEnd);
    
    contextMenu_->addSeparator();
    
    duplicateAction_ = contextMenu_->addAction(tr("Duplicate"));
    duplicateAction_->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_D));
    connect(duplicateAction_, &QAction::triggered, this, &EventListView::duplicateSelected);
    
    contextMenu_->addSeparator();
    
    moveUpAction_ = contextMenu_->addAction(tr("Move Up"));
    moveUpAction_->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Up));
    connect(moveUpAction_, &QAction::triggered, this, &EventListView::moveSelectedUp);
    
    moveDownAction_ = contextMenu_->addAction(tr("Move Down"));
    moveDownAction_->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Down));
    connect(moveDownAction_, &QAction::triggered, this, &EventListView::moveSelectedDown);
    
    contextMenu_->addSeparator();
    
    copyAction_ = contextMenu_->addAction(tr("Copy"));
    copyAction_->setShortcut(QKeySequence::Copy);
    
    pasteAction_ = contextMenu_->addAction(tr("Paste"));
    pasteAction_->setShortcut(QKeySequence::Paste);
    pasteAction_->setEnabled(false);
    
    contextMenu_->addSeparator();
    
    deleteAction_ = contextMenu_->addAction(tr("Delete"));
    deleteAction_->setShortcut(QKeySequence::Delete);
    connect(deleteAction_, &QAction::triggered, this, &EventListView::deleteSelected);
}

void EventListView::setModel(EventListModel* model)
{
    model_ = model;
    QTableView::setModel(model);
    
    // Set column widths
    if (model_) {
        setColumnWidth(EventListModel::ColumnIndex, 50);
        setColumnWidth(EventListModel::ColumnType, 100);
        setColumnWidth(EventListModel::ColumnDetails, 250);
        setColumnWidth(EventListModel::ColumnTimestamp, 80);
        setColumnWidth(EventListModel::ColumnDuration, 80);
        
        // Stretch details column
        horizontalHeader()->setSectionResizeMode(EventListModel::ColumnDetails, QHeaderView::Stretch);
    }
}

int EventListView::selectedEventIndex() const
{
    QModelIndexList selection = selectedIndexes();
    if (selection.isEmpty()) {
        return -1;
    }
    return selection.first().row();
}

QList<int> EventListView::selectedEventIndices() const
{
    QList<int> indices;
    QModelIndexList selection = selectedIndexes();
    
    for (const QModelIndex& idx : selection) {
        if (idx.column() == 0 && !indices.contains(idx.row())) {
            indices.append(idx.row());
        }
    }
    
    std::sort(indices.begin(), indices.end());
    return indices;
}

void EventListView::selectEvent(int index)
{
    if (!model_ || index < 0 || index >= model_->rowCount()) {
        clearSelection();
        return;
    }
    
    QModelIndex modelIndex = model_->index(index, 0);
    setCurrentIndex(modelIndex);
    selectRow(index);
    scrollTo(modelIndex);
}

void EventListView::deleteSelected()
{
    QList<int> indices = selectedEventIndices();
    if (indices.isEmpty()) {
        return;
    }
    
    emit deleteEventsRequested(indices);
    // Deletion is now handled through undo commands in MainWindow
}

void EventListView::addEventBefore()
{
    int index = selectedEventIndex();
    if (index < 0) {
        index = 0;
    }
    emit addEventRequested(index - 1);
}

void EventListView::addEventAfter()
{
    int index = selectedEventIndex();
    emit addEventRequested(index);
}

void EventListView::addEventAtEnd()
{
    emit addEventRequested(-1);  // -1 signals "at end"
}

void EventListView::contextMenuEvent(QContextMenuEvent* event)
{
    bool hasSelection = !selectedIndexes().isEmpty();
    int index = selectedEventIndex();
    bool canMoveUp = index > 0;
    bool canMoveDown = model_ && index >= 0 && index < model_->rowCount() - 1;
    
    addBeforeAction_->setEnabled(hasSelection);
    addAfterAction_->setEnabled(hasSelection);
    duplicateAction_->setEnabled(hasSelection);
    moveUpAction_->setEnabled(canMoveUp);
    moveDownAction_->setEnabled(canMoveDown);
    deleteAction_->setEnabled(hasSelection);
    copyAction_->setEnabled(hasSelection);
    
    contextMenu_->exec(event->globalPos());
}

void EventListView::selectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
    QTableView::selectionChanged(selected, deselected);
    
    int index = selectedEventIndex();
    emit eventSelected(index);
}

void EventListView::mouseDoubleClickEvent(QMouseEvent* event)
{
    QTableView::mouseDoubleClickEvent(event);
    
    QModelIndex index = indexAt(event->pos());
    if (index.isValid()) {
        emit eventDoubleClicked(index.row());
    }
}

void EventListView::duplicateSelected()
{
    int index = selectedEventIndex();
    if (index >= 0) {
        emit duplicateRequested(index);
    }
}

void EventListView::moveSelectedUp()
{
    int index = selectedEventIndex();
    if (index > 0) {
        emit moveUpRequested(index);
    }
}

void EventListView::moveSelectedDown()
{
    int index = selectedEventIndex();
    if (index >= 0 && model_ && index < model_->rowCount() - 1) {
        emit moveDownRequested(index);
    }
}
