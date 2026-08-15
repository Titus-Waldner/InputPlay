#pragma once

#include <QDialog>

class QSpinBox;
class QDialogButtonBox;

class GoToEventDialog : public QDialog
{
    Q_OBJECT

public:
    explicit GoToEventDialog(int maxEvent, int currentEvent, QWidget* parent = nullptr);
    
    int selectedEvent() const;

private:
    void setupUi(int maxEvent, int currentEvent);
    
    QSpinBox* eventSpin_ = nullptr;
};
