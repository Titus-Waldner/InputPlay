#include "EventFilterWidget.h"
#include "DarkStyle.h"

#include <QHBoxLayout>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>

EventFilterWidget::EventFilterWidget(QWidget* parent)
    : QWidget(parent)
{
    // Enable all types by default
    enabledTypes_ = {
        EventType::MouseMove,
        EventType::MouseButtonDown,
        EventType::MouseButtonUp,
        EventType::MouseWheel,
        EventType::KeyDown,
        EventType::KeyUp,
        EventType::Wait,
        EventType::MouseTeleport
    };
    
    setupUi();
}

void EventFilterWidget::setupUi()
{
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(8);
    
    // Search icon/label
    QLabel* searchLabel = new QLabel(tr("🔍"));
    layout->addWidget(searchLabel);
    
    // Search text field
    searchEdit_ = new QLineEdit();
    searchEdit_->setPlaceholderText(tr("Search events..."));
    searchEdit_->setClearButtonEnabled(true);
    searchEdit_->setMinimumWidth(150);
    searchEdit_->setMaximumWidth(250);
    connect(searchEdit_, &QLineEdit::textChanged, this, &EventFilterWidget::onSearchTextChanged);
    layout->addWidget(searchEdit_);
    
    // Type filter dropdown
    QLabel* filterLabel = new QLabel(tr("Type:"));
    layout->addWidget(filterLabel);
    
    typeFilterCombo_ = new QComboBox();
    typeFilterCombo_->addItem(tr("All Events"), -1);
    typeFilterCombo_->addItem(tr("Mouse Move"), static_cast<int>(EventType::MouseMove));
    typeFilterCombo_->addItem(tr("Mouse Buttons"), -2);  // Special: both down and up
    typeFilterCombo_->addItem(tr("Mouse Wheel"), static_cast<int>(EventType::MouseWheel));
    typeFilterCombo_->addItem(tr("Keyboard"), -3);  // Special: both down and up
    typeFilterCombo_->addItem(tr("Wait"), static_cast<int>(EventType::Wait));
    typeFilterCombo_->addItem(tr("Teleport"), static_cast<int>(EventType::MouseTeleport));
    connect(typeFilterCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &EventFilterWidget::onTypeFilterChanged);
    layout->addWidget(typeFilterCombo_);
    
    // Clear filter button
    clearButton_ = new QPushButton(tr("Clear"));
    clearButton_->setToolTip(tr("Clear all filters"));
    connect(clearButton_, &QPushButton::clicked, this, &EventFilterWidget::clear);
    layout->addWidget(clearButton_);
    
    layout->addStretch();
}

QString EventFilterWidget::searchText() const
{
    return searchEdit_->text();
}

QSet<EventType> EventFilterWidget::enabledTypes() const
{
    return enabledTypes_;
}

bool EventFilterWidget::matchesFilter(const InputEvent& event, int index) const
{
    // Check type filter
    if (!enabledTypes_.contains(event.type)) {
        return false;
    }
    
    // Check search text
    QString search = searchEdit_->text().trimmed();
    if (search.isEmpty()) {
        return true;
    }
    
    // Search in various fields
    QString searchLower = search.toLower();
    
    // Check index
    if (QString::number(index + 1).contains(search)) {
        return true;
    }
    
    // Check type name
    QString typeName;
    switch (event.type) {
        case EventType::MouseMove: typeName = "mouse move"; break;
        case EventType::MouseButtonDown: typeName = "mouse down button"; break;
        case EventType::MouseButtonUp: typeName = "mouse up button"; break;
        case EventType::MouseWheel: typeName = "mouse wheel scroll"; break;
        case EventType::KeyDown: typeName = "key down keyboard"; break;
        case EventType::KeyUp: typeName = "key up keyboard"; break;
        case EventType::Wait: typeName = "wait delay pause"; break;
        case EventType::MouseTeleport: typeName = "teleport jump"; break;
    }
    if (typeName.contains(searchLower)) {
        return true;
    }
    
    // Check coordinates
    QString coords = QString("%1,%2").arg(event.mouseX).arg(event.mouseY);
    if (coords.contains(search)) {
        return true;
    }
    
    // Check key code
    if (event.type == EventType::KeyDown || event.type == EventType::KeyUp) {
        if (QString::number(event.keyCode).contains(search)) {
            return true;
        }
    }
    
    // Check timestamp
    if (QString::number(event.timestampMicroseconds).contains(search)) {
        return true;
    }
    
    return false;
}

void EventFilterWidget::clear()
{
    searchEdit_->clear();
    typeFilterCombo_->setCurrentIndex(0);  // "All Events"
    
    enabledTypes_ = {
        EventType::MouseMove,
        EventType::MouseButtonDown,
        EventType::MouseButtonUp,
        EventType::MouseWheel,
        EventType::KeyDown,
        EventType::KeyUp,
        EventType::Wait,
        EventType::MouseTeleport
    };
    
    emit filterChanged();
}

void EventFilterWidget::onSearchTextChanged()
{
    emit filterChanged();
}

void EventFilterWidget::onTypeFilterChanged()
{
    int data = typeFilterCombo_->currentData().toInt();
    
    enabledTypes_.clear();
    
    if (data == -1) {
        // All events
        enabledTypes_ = {
            EventType::MouseMove,
            EventType::MouseButtonDown,
            EventType::MouseButtonUp,
            EventType::MouseWheel,
            EventType::KeyDown,
            EventType::KeyUp,
            EventType::Wait,
            EventType::MouseTeleport
        };
    } else if (data == -2) {
        // Mouse buttons
        enabledTypes_ = { EventType::MouseButtonDown, EventType::MouseButtonUp };
    } else if (data == -3) {
        // Keyboard
        enabledTypes_ = { EventType::KeyDown, EventType::KeyUp };
    } else {
        // Single type
        enabledTypes_.insert(static_cast<EventType>(data));
    }
    
    emit filterChanged();
}

void EventFilterWidget::setTypeFilter(EventType type)
{
    // Find the combo box item that matches the type
    int targetIndex = 0;
    
    switch (type) {
        case EventType::MouseMove:
            targetIndex = 1;
            break;
        case EventType::MouseButtonDown:
        case EventType::MouseButtonUp:
            targetIndex = 2;  // "Mouse Buttons"
            break;
        case EventType::MouseWheel:
            targetIndex = 3;
            break;
        case EventType::KeyDown:
        case EventType::KeyUp:
            targetIndex = 4;  // "Keyboard"
            break;
        case EventType::Wait:
            targetIndex = 5;
            break;
        case EventType::MouseTeleport:
            targetIndex = 6;
            break;
    }
    
    typeFilterCombo_->setCurrentIndex(targetIndex);
}
