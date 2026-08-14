#include "ShortcutsDialog.h"
#include "DarkStyle.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTableWidget>
#include <QHeaderView>
#include <QPushButton>
#include <QLabel>

ShortcutsDialog::ShortcutsDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Keyboard Shortcuts"));
    setMinimumSize(500, 500);
    resize(550, 600);
    
    setupUi();
    populateShortcuts();
}

void ShortcutsDialog::setupUi()
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setSpacing(16);
    
    // Title
    QLabel* titleLabel = new QLabel(tr("Keyboard Shortcuts"));
    titleLabel->setProperty("heading", true);
    layout->addWidget(titleLabel);
    
    // Table
    table_ = new QTableWidget();
    table_->setColumnCount(2);
    table_->setHorizontalHeaderLabels({tr("Action"), tr("Shortcut")});
    table_->horizontalHeader()->setStretchLastSection(true);
    table_->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    table_->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    table_->verticalHeader()->setVisible(false);
    table_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table_->setSelectionBehavior(QAbstractItemView::SelectRows);
    table_->setAlternatingRowColors(true);
    table_->setShowGrid(false);
    layout->addWidget(table_);
    
    // Close button
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    
    QPushButton* closeButton = new QPushButton(tr("Close"));
    connect(closeButton, &QPushButton::clicked, this, &QDialog::accept);
    buttonLayout->addWidget(closeButton);
    
    layout->addLayout(buttonLayout);
}

void ShortcutsDialog::populateShortcuts()
{
    struct Shortcut {
        QString action;
        QString shortcut;
    };
    
    QList<Shortcut> shortcuts = {
        // File
        {tr("New Macro"), "Ctrl+N"},
        {tr("Open Macro"), "Ctrl+O"},
        {tr("Save Macro"), "Ctrl+S"},
        {tr("Save Macro As"), "Ctrl+Shift+S"},
        
        // Edit
        {tr("Undo"), "Ctrl+Z"},
        {tr("Redo"), "Ctrl+Y / Ctrl+Shift+Z"},
        {tr("Add Event"), "Ctrl+E"},
        {tr("Insert Event Before"), "Ctrl+Shift+E"},
        {tr("Duplicate Event"), "Ctrl+D"},
        {tr("Move Event Up"), "Ctrl+Up"},
        {tr("Move Event Down"), "Ctrl+Down"},
        {tr("Delete Event(s)"), "Delete"},
        {tr("Select All"), "Ctrl+A"},
        {tr("Settings"), "Ctrl+,"},
        
        // View
        {tr("Toggle List/Timeline View"), "Ctrl+Tab"},
        {tr("Zoom In Timeline"), "Ctrl++"},
        {tr("Zoom Out Timeline"), "Ctrl+-"},
        {tr("Reset Zoom"), "Ctrl+0"},
        
        // Macro (Recording/Playback)
        {tr("Start/Stop Recording"), "F9"},
        {tr("Start/Stop Playback"), "F10"},
        {tr("Pause/Resume"), "F11"},
        {tr("Emergency Stop (Global)"), "Ctrl+Shift+Escape"},
        
        // Playback
        {tr("Play/Pause (Window)"), "Space"},
        {tr("Stop Playback"), "Escape"},
        
        // Navigation
        {tr("Go to Event"), "Ctrl+G"},
        {tr("Find/Filter Events"), "Ctrl+F"},
        
        // Context Menu
        {tr("Copy Event(s)"), "Ctrl+C"},
        {tr("Paste Event(s)"), "Ctrl+V"},
    };
    
    table_->setRowCount(shortcuts.size());
    
    for (int i = 0; i < shortcuts.size(); ++i) {
        QTableWidgetItem* actionItem = new QTableWidgetItem(shortcuts[i].action);
        QTableWidgetItem* shortcutItem = new QTableWidgetItem(shortcuts[i].shortcut);
        
        // Style the shortcut column
        QFont monoFont("Consolas", 9);
        shortcutItem->setFont(monoFont);
        shortcutItem->setForeground(
			DarkStyle::toneColor(
				"accent"));
				
        table_->setItem(i, 0, actionItem);
        table_->setItem(i, 1, shortcutItem);
    }
    
    table_->resizeRowsToContents();
}
