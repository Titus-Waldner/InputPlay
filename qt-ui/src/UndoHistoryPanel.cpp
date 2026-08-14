#include "UndoHistoryPanel.h"
#include "DarkStyle.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QUndoStack>

UndoHistoryPanel::UndoHistoryPanel(QWidget* parent)
    : QWidget(parent)
{
    setupUi();
}

void UndoHistoryPanel::setupUi()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(8, 8, 8, 8);
    mainLayout->setSpacing(8);
    
    // Title
    titleLabel_ = new QLabel(tr("Undo History"));
    titleLabel_->setProperty("heading", true);
    mainLayout->addWidget(titleLabel_);
    
    // Button row
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(4);
    
    undoButton_ = new QPushButton(tr("Undo"));
    undoButton_->setEnabled(false);
    undoButton_->setToolTip(tr("Undo last action (Ctrl+Z)"));
    connect(undoButton_, &QPushButton::clicked, this, [this]() {
        if (undoStack_) undoStack_->undo();
    });
    buttonLayout->addWidget(undoButton_);
    
    redoButton_ = new QPushButton(tr("Redo"));
    redoButton_->setEnabled(false);
    redoButton_->setToolTip(tr("Redo last undone action (Ctrl+Y)"));
    connect(redoButton_, &QPushButton::clicked, this, [this]() {
        if (undoStack_) undoStack_->redo();
    });
    buttonLayout->addWidget(redoButton_);
    
    buttonLayout->addStretch();
    
    clearButton_ = new QPushButton(tr("Clear"));
    clearButton_->setEnabled(false);
    clearButton_->setToolTip(tr("Clear undo history"));
    connect(clearButton_, &QPushButton::clicked, this, [this]() {
        if (undoStack_) undoStack_->clear();
    });
    buttonLayout->addWidget(clearButton_);
    
    mainLayout->addLayout(buttonLayout);
    
    // History list
    historyList_ = new QListWidget();
    historyList_->setAlternatingRowColors(true);
    historyList_->setSelectionMode(QAbstractItemView::SingleSelection);
    connect(historyList_, &QListWidget::itemDoubleClicked, this, &UndoHistoryPanel::onItemDoubleClicked);
    mainLayout->addWidget(historyList_, 1);
    
    // Status label
	statusLabel_ = new QLabel(tr("No actions"));

	DarkStyle::setTone(
		statusLabel_,
		"secondary");

	mainLayout->addWidget(
		statusLabel_);
	}

void UndoHistoryPanel::setUndoStack(QUndoStack* stack)
{
    if (undoStack_) {
        disconnect(undoStack_, nullptr, this, nullptr);
    }
    
    undoStack_ = stack;
    
    if (undoStack_) {
        connect(undoStack_, &QUndoStack::indexChanged, this, &UndoHistoryPanel::onIndexChanged);
        connect(undoStack_, &QUndoStack::cleanChanged, this, &UndoHistoryPanel::updateList);
        connect(undoStack_, &QUndoStack::canUndoChanged, undoButton_, &QPushButton::setEnabled);
        connect(undoStack_, &QUndoStack::canRedoChanged, redoButton_, &QPushButton::setEnabled);
        
        undoButton_->setEnabled(undoStack_->canUndo());
        redoButton_->setEnabled(undoStack_->canRedo());
    }
    
    updateList();
}

void UndoHistoryPanel::onIndexChanged(int index)
{
    Q_UNUSED(index);
    updateList();
}

void UndoHistoryPanel::onItemDoubleClicked(QListWidgetItem* item)
{
    if (!undoStack_) return;
    
    int targetIndex = historyList_->row(item);
    undoStack_->setIndex(targetIndex);
}

void UndoHistoryPanel::updateList()
{
    historyList_->clear();

    if (!undoStack_)
    {
        statusLabel_->setText(
            tr("No undo stack"));

        DarkStyle::setTone(
            statusLabel_,
            "secondary");

        clearButton_->setEnabled(
            false);

        return;
    }

    const int count =
        undoStack_->count();

    const int currentIndex =
        undoStack_->index();

    clearButton_->setEnabled(
        count > 0);

    // The list row and undo-stack index intentionally match:
    //
    // Row 0 = Initial State
    // Row 1 = State after command 1
    // Row 2 = State after command 2
    // ...
    //
    // This makes the current state appear at row currentIndex.

    QListWidgetItem* initialItem =
        new QListWidgetItem(
            tr("● Initial State"));

    initialItem->setForeground(
        QColor(
            DarkStyle::textSecondary()));

    if (currentIndex == 0)
    {
        initialItem->setText(
            tr("→ Initial State"));

        initialItem->setBackground(
            QColor(
                DarkStyle::bgHover()));

        initialItem->setForeground(
            QColor(
                DarkStyle::accentColorString()));
    }

    historyList_->addItem(
        initialItem);

    // Add each command to the history list.
    for (int commandIndex = 0;
         commandIndex < count;
         ++commandIndex)
    {
        const QUndoCommand* command =
            undoStack_->command(
                commandIndex);

        QString commandText =
            command->text();

        if (commandText.isEmpty())
        {
            commandText =
                tr("Action %1")
                    .arg(
                        commandIndex + 1);
        }

        // A command at commandIndex produces the state represented
        // by list row commandIndex + 1.
        const int stateIndex =
            commandIndex + 1;

        const bool isCurrent =
            stateIndex == currentIndex;

        const bool isExecuted =
            stateIndex <= currentIndex;

        QListWidgetItem* item =
            new QListWidgetItem();

        if (isCurrent)
        {
            item->setText(
                QString("→ %1")
                    .arg(
                        commandText));

            item->setBackground(
                QColor(
                    DarkStyle::bgHover()));

            item->setForeground(
                QColor(
                    DarkStyle::accentColorString()));
        }
        else if (isExecuted)
        {
            item->setText(
                QString("✓ %1")
                    .arg(
                        commandText));

            item->setForeground(
                QColor(
                    DarkStyle::accentColorLighter()));
        }
        else
        {
            item->setText(
                QString("○ %1")
                    .arg(
                        commandText));

            item->setForeground(
                QColor(
                    DarkStyle::textSecondary()));
        }

        historyList_->addItem(
            item);
    }

    // Keep the current state visible.
    if (currentIndex >= 0
        && currentIndex < historyList_->count())
    {
        historyList_->scrollToItem(
            historyList_->item(
                currentIndex));
    }

    // Update the summary label.
    if (count == 0)
    {
        statusLabel_->setText(
            tr("No actions"));
    }
    else
    {
        const int undoable =
            currentIndex;

        const int redoable =
            count - currentIndex;

        statusLabel_->setText(
            tr(
                "%1 action(s), "
                "%2 undoable, "
                "%3 redoable")
                .arg(count)
                .arg(undoable)
                .arg(redoable));
    }

    DarkStyle::setTone(
        statusLabel_,
        "secondary");
}