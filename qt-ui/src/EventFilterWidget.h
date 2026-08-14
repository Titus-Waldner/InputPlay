#pragma once

#include "InputEvent.h"

#include <QWidget>
#include <QSet>

class QLineEdit;
class QComboBox;
class QCheckBox;
class QPushButton;

class EventFilterWidget : public QWidget
{
    Q_OBJECT

public:
    explicit EventFilterWidget(QWidget* parent = nullptr);
    
    QString searchText() const;
    QSet<EventType> enabledTypes() const;
    bool matchesFilter(const InputEvent& event, int index) const;
    
    void clear();
    void setTypeFilter(EventType type);

signals:
    void filterChanged();

private slots:
    void onSearchTextChanged();
    void onTypeFilterChanged();

private:
    void setupUi();
    
    QLineEdit* searchEdit_ = nullptr;
    QComboBox* typeFilterCombo_ = nullptr;
    QPushButton* clearButton_ = nullptr;
    
    // Type filter checkboxes (in dropdown or separate)
    QSet<EventType> enabledTypes_;
};
