#pragma once

#include <QWidget>
#include <QListWidget>

class QUndoStack;
class QLabel;
class QPushButton;

class UndoHistoryPanel : public QWidget
{
    Q_OBJECT

public:
    explicit UndoHistoryPanel(QWidget* parent = nullptr);
    
    void setUndoStack(QUndoStack* stack);

private slots:
    void onIndexChanged(int index);
    void onItemDoubleClicked(QListWidgetItem* item);
    void updateList();

private:
    void setupUi();
    
    QUndoStack* undoStack_ = nullptr;
    
    QLabel* titleLabel_ = nullptr;
    QListWidget* historyList_ = nullptr;
    QLabel* statusLabel_ = nullptr;
    QPushButton* undoButton_ = nullptr;
    QPushButton* redoButton_ = nullptr;
    QPushButton* clearButton_ = nullptr;
};
