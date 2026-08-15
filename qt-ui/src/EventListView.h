#pragma once

#include "InputEvent.h"

#include <QTableView>
#include <QMenu>

class EventListModel;

class EventListView : public QTableView
{
    Q_OBJECT

public:
    explicit EventListView(QWidget* parent = nullptr);
    
    void setModel(EventListModel* model);
    EventListModel* eventModel() const { return model_; }
    
    int selectedEventIndex() const;
    QList<int> selectedEventIndices() const;

signals:
    void eventSelected(int index);
    void eventDoubleClicked(int index);
    void addEventRequested(int afterIndex);
    void deleteEventsRequested(QList<int> indices);
    void duplicateRequested(int index);
    void moveUpRequested(int index);
    void moveDownRequested(int index);

public slots:
    void selectEvent(int index);
    void deleteSelected();
    void addEventBefore();
    void addEventAfter();
    void addEventAtEnd();
    void duplicateSelected();
    void moveSelectedUp();
    void moveSelectedDown();

protected:
    void contextMenuEvent(QContextMenuEvent* event) override;
    void selectionChanged(const QItemSelection& selected, const QItemSelection& deselected) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;

private:
    void setupUi();
    void setupContextMenu();
    
    EventListModel* model_ = nullptr;
    QMenu* contextMenu_ = nullptr;
    
    QAction* addBeforeAction_ = nullptr;
    QAction* addAfterAction_ = nullptr;
    QAction* addAtEndAction_ = nullptr;
    QAction* duplicateAction_ = nullptr;
    QAction* moveUpAction_ = nullptr;
    QAction* moveDownAction_ = nullptr;
    QAction* deleteAction_ = nullptr;
    QAction* copyAction_ = nullptr;
    QAction* pasteAction_ = nullptr;
};
