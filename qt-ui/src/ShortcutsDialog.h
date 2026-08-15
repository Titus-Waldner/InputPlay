#pragma once

#include <QDialog>

class QTableWidget;

class ShortcutsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ShortcutsDialog(QWidget* parent = nullptr);

private:
    void setupUi();
    void populateShortcuts();
    
    QTableWidget* table_ = nullptr;
};
