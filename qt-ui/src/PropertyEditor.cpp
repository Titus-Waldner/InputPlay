#include "PropertyEditor.h"
#include "DarkStyle.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QSpinBox>
#include <QComboBox>
#include <QLineEdit>
#include <QStackedWidget>
#include <QPushButton>
#include <QGroupBox>
#include <QFrame>

PropertyEditor::PropertyEditor(QWidget* parent)
    : QWidget(parent)
{
    setupUi();
    clear();
}

void PropertyEditor::setupUi()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(12, 12, 12, 12);
    mainLayout->setSpacing(12);
    
    // Title
    titleLabel_ = new QLabel(tr("Event Properties"));
    titleLabel_->setProperty("heading", true);
    mainLayout->addWidget(titleLabel_);
    
    // Event info
    QGroupBox* infoGroup = new QGroupBox(tr("Event Info"));
    QFormLayout* infoLayout = new QFormLayout(infoGroup);
    
    indexLabel_ = new QLabel(tr("-"));
    infoLayout->addRow(tr("Index:"), indexLabel_);
    
    typeCombo_ = new QComboBox();
    typeCombo_->addItem(tr("Mouse Move"), static_cast<int>(EventType::MouseMove));
    typeCombo_->addItem(tr("Mouse Down"), static_cast<int>(EventType::MouseButtonDown));
    typeCombo_->addItem(tr("Mouse Up"), static_cast<int>(EventType::MouseButtonUp));
    typeCombo_->addItem(tr("Mouse Wheel"), static_cast<int>(EventType::MouseWheel));
    typeCombo_->addItem(tr("Key Down"), static_cast<int>(EventType::KeyDown));
    typeCombo_->addItem(tr("Key Up"), static_cast<int>(EventType::KeyUp));
    typeCombo_->addItem(tr("Wait"), static_cast<int>(EventType::Wait));
    typeCombo_->addItem(tr("Teleport"), static_cast<int>(EventType::MouseTeleport));
    infoLayout->addRow(tr("Type:"), typeCombo_);
    
    timestampSpin_ = new QSpinBox();
    timestampSpin_->setRange(0, 2147483647);
    timestampSpin_->setSuffix(tr(" µs"));
    infoLayout->addRow(tr("Timestamp:"), timestampSpin_);
    
    mainLayout->addWidget(infoGroup);
    
    connect(typeCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index) {
        EventType newType = static_cast<EventType>(typeCombo_->itemData(index).toInt());
        panelStack_->setCurrentIndex(panelIndexForEventType(newType));
    });
    
    // Stacked panels for different event types
    panelStack_ = new QStackedWidget();
    
    setupMouseMovePanel();
    setupMouseButtonPanel();
    setupMouseWheelPanel();
    setupKeyPanel();
    setupWaitPanel();
    setupTeleportPanel();
    
    panelStack_->addWidget(mouseMovePanel_);
    panelStack_->addWidget(mouseButtonPanel_);
    panelStack_->addWidget(mouseWheelPanel_);
    panelStack_->addWidget(keyPanel_);
    panelStack_->addWidget(waitPanel_);
    panelStack_->addWidget(teleportPanel_);
    
    mainLayout->addWidget(panelStack_);
    
    // Spacer
    mainLayout->addStretch();
    
    // Action buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    
    revertButton_ = new QPushButton(tr("Revert"));
    revertButton_->setToolTip(tr("Discard changes"));
    connect(revertButton_, &QPushButton::clicked, this, &PropertyEditor::revertChanges);
    buttonLayout->addWidget(revertButton_);
    
    applyButton_ = new QPushButton(tr("Apply"));
    applyButton_->setProperty("primary", true);
    applyButton_->setToolTip(tr("Apply changes to event"));
    connect(applyButton_, &QPushButton::clicked, this, &PropertyEditor::applyChanges);
    buttonLayout->addWidget(applyButton_);
    
    mainLayout->addLayout(buttonLayout);
}

void PropertyEditor::setupMouseMovePanel()
{
    mouseMovePanel_ = new QWidget();
    QFormLayout* layout = new QFormLayout(mouseMovePanel_);
    layout->setContentsMargins(0, 0, 0, 0);
    
    QGroupBox* deltaGroup = new QGroupBox(tr("Movement Delta"));
    QFormLayout* deltaLayout = new QFormLayout(deltaGroup);
    
    moveDeltaXSpin_ = new QSpinBox();
    moveDeltaXSpin_->setRange(-10000, 10000);
    deltaLayout->addRow(tr("ΔX:"), moveDeltaXSpin_);
    
    moveDeltaYSpin_ = new QSpinBox();
    moveDeltaYSpin_->setRange(-10000, 10000);
    deltaLayout->addRow(tr("ΔY:"), moveDeltaYSpin_);
    
    layout->addRow(deltaGroup);
    
    QGroupBox* posGroup = new QGroupBox(tr("Resulting Position"));
    QFormLayout* posLayout = new QFormLayout(posGroup);
    
    moveXSpin_ = new QSpinBox();
    moveXSpin_->setRange(-10000, 100000);
    posLayout->addRow(tr("X:"), moveXSpin_);
    
    moveYSpin_ = new QSpinBox();
    moveYSpin_->setRange(-10000, 100000);
    posLayout->addRow(tr("Y:"), moveYSpin_);
    
    layout->addRow(posGroup);
}

void PropertyEditor::setupMouseButtonPanel()
{
    mouseButtonPanel_ = new QWidget();
    QFormLayout* layout = new QFormLayout(mouseButtonPanel_);
    layout->setContentsMargins(0, 0, 0, 0);
    
    QGroupBox* group = new QGroupBox(tr("Button Properties"));
    QFormLayout* groupLayout = new QFormLayout(group);
    
    buttonCombo_ = new QComboBox();
    buttonCombo_->addItem(tr("Left Button"), 1);
    buttonCombo_->addItem(tr("Right Button"), 2);
    buttonCombo_->addItem(tr("Middle Button"), 3);
    buttonCombo_->addItem(tr("X1 Button"), 4);
    buttonCombo_->addItem(tr("X2 Button"), 5);
    groupLayout->addRow(tr("Button:"), buttonCombo_);
    
    buttonXSpin_ = new QSpinBox();
    buttonXSpin_->setRange(-10000, 100000);
    groupLayout->addRow(tr("X:"), buttonXSpin_);
    
    buttonYSpin_ = new QSpinBox();
    buttonYSpin_->setRange(-10000, 100000);
    groupLayout->addRow(tr("Y:"), buttonYSpin_);
    
    layout->addRow(group);
}

void PropertyEditor::setupMouseWheelPanel()
{
    mouseWheelPanel_ = new QWidget();
    QFormLayout* layout = new QFormLayout(mouseWheelPanel_);
    layout->setContentsMargins(0, 0, 0, 0);
    
    QGroupBox* group = new QGroupBox(tr("Wheel Properties"));
    QFormLayout* groupLayout = new QFormLayout(group);
    
    wheelDeltaSpin_ = new QSpinBox();
    wheelDeltaSpin_->setRange(-10000, 10000);
    wheelDeltaSpin_->setToolTip(tr("Positive = scroll up, Negative = scroll down"));
    groupLayout->addRow(tr("Delta:"), wheelDeltaSpin_);
    
    wheelXSpin_ = new QSpinBox();
    wheelXSpin_->setRange(-10000, 100000);
    groupLayout->addRow(tr("X:"), wheelXSpin_);
    
    wheelYSpin_ = new QSpinBox();
    wheelYSpin_->setRange(-10000, 100000);
    groupLayout->addRow(tr("Y:"), wheelYSpin_);
    
    layout->addRow(group);
}

void PropertyEditor::setupKeyPanel()
{
    keyPanel_ = new QWidget();
    QFormLayout* layout = new QFormLayout(keyPanel_);
    layout->setContentsMargins(0, 0, 0, 0);
    
    QGroupBox* group = new QGroupBox(tr("Key Properties"));
    QFormLayout* groupLayout = new QFormLayout(group);
    
    keyCodeSpin_ = new QSpinBox();
    keyCodeSpin_->setRange(0, 255);
    keyCodeSpin_->setDisplayIntegerBase(16);
    keyCodeSpin_->setPrefix("0x");
    groupLayout->addRow(tr("Key Code:"), keyCodeSpin_);
    
    keyNameEdit_ = new QLineEdit();
    keyNameEdit_->setReadOnly(true);
    keyNameEdit_->setPlaceholderText(tr("Key name"));
    groupLayout->addRow(tr("Key Name:"), keyNameEdit_);
    
    // Update key name when code changes
    connect(keyCodeSpin_, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int value) {
        QString name;
        switch (value) {
            case 0x08: name = "Backspace"; break;
            case 0x09: name = "Tab"; break;
            case 0x0D: name = "Enter"; break;
            case 0x10: name = "Shift"; break;
            case 0x11: name = "Ctrl"; break;
            case 0x12: name = "Alt"; break;
            case 0x1B: name = "Escape"; break;
            case 0x20: name = "Space"; break;
            case 0x25: name = "Left Arrow"; break;
            case 0x26: name = "Up Arrow"; break;
            case 0x27: name = "Right Arrow"; break;
            case 0x28: name = "Down Arrow"; break;
            case 0x2E: name = "Delete"; break;
            default:
                if (value >= 0x30 && value <= 0x39) {
                    name = QChar('0' + (value - 0x30));
                } else if (value >= 0x41 && value <= 0x5A) {
                    name = QChar('A' + (value - 0x41));
                } else if (value >= 0x70 && value <= 0x87) {
                    name = QString("F%1").arg(value - 0x70 + 1);
                } else {
                    name = QString("VK 0x%1").arg(value, 2, 16, QChar('0')).toUpper();
                }
                break;
        }
        keyNameEdit_->setText(name);
    });
    
    layout->addRow(group);
}

void PropertyEditor::setupWaitPanel()
{
    waitPanel_ = new QWidget();
    QFormLayout* layout = new QFormLayout(waitPanel_);
    layout->setContentsMargins(0, 0, 0, 0);
    
    QGroupBox* group = new QGroupBox(tr("Wait Duration"));
    QFormLayout* groupLayout = new QFormLayout(group);
    
    waitMsSpin_ = new QSpinBox();
    waitMsSpin_->setRange(0, 2147483647);
    waitMsSpin_->setSuffix(tr(" ms"));
    waitMsSpin_->setToolTip(tr("Wait time in milliseconds"));
    groupLayout->addRow(tr("Duration:"), waitMsSpin_);
    
    layout->addRow(group);
}

void PropertyEditor::setupTeleportPanel()
{
    teleportPanel_ = new QWidget();
    QFormLayout* layout = new QFormLayout(teleportPanel_);
    layout->setContentsMargins(0, 0, 0, 0);
    
    QGroupBox* group = new QGroupBox(tr("Teleport Position"));
    QFormLayout* groupLayout = new QFormLayout(group);
    
    teleportXSpin_ = new QSpinBox();
    teleportXSpin_->setRange(-10000, 100000);
    groupLayout->addRow(tr("X:"), teleportXSpin_);
    
    teleportYSpin_ = new QSpinBox();
    teleportYSpin_->setRange(-10000, 100000);
    groupLayout->addRow(tr("Y:"), teleportYSpin_);
    
    layout->addRow(group);
}

void PropertyEditor::setEvent(const InputEvent* event, int index)
{
    event_ = event;
    eventIndex_ = index;
    
    if (!event_) {
        clear();
        return;
    }
    
    editedEvent_ = *event_;
    
    setEnabled(true);
    indexLabel_->setText(QString::number(index + 1));
    
    // Set type combo
    for (int i = 0; i < typeCombo_->count(); ++i) {
        if (typeCombo_->itemData(i).toInt() == static_cast<int>(event_->type)) {
            typeCombo_->setCurrentIndex(i);
            break;
        }
    }
    
    timestampSpin_->setValue(static_cast<int>(event_->timestampMicroseconds));
    
    panelStack_->setCurrentIndex(panelIndexForEventType(event_->type));
    updatePanelFromEvent();
}

void PropertyEditor::clear()
{
    event_ = nullptr;
    eventIndex_ = -1;
    
    setEnabled(false);
    indexLabel_->setText(tr("-"));
    typeCombo_->setCurrentIndex(0);
    timestampSpin_->setValue(0);
    
    titleLabel_->setText(tr("Event Properties"));
}

void PropertyEditor::applyChanges()
{
    if (!event_ || eventIndex_ < 0) {
        return;
    }
    
    InputEvent oldEvent = *event_;
    updateEventFromPanel();
    emit eventModified(eventIndex_, oldEvent, editedEvent_);
}

void PropertyEditor::revertChanges()
{
    if (event_) {
        editedEvent_ = *event_;
        updatePanelFromEvent();
    }
}

void PropertyEditor::updatePanelFromEvent()
{
    switch (event_->type) {
        case EventType::MouseMove:
            moveDeltaXSpin_->setValue(event_->mouseDeltaX);
            moveDeltaYSpin_->setValue(event_->mouseDeltaY);
            moveXSpin_->setValue(event_->mouseX);
            moveYSpin_->setValue(event_->mouseY);
            break;
            
        case EventType::MouseButtonDown:
        case EventType::MouseButtonUp:
            for (int i = 0; i < buttonCombo_->count(); ++i) {
                if (buttonCombo_->itemData(i).toInt() == event_->mouseButton) {
                    buttonCombo_->setCurrentIndex(i);
                    break;
                }
            }
            buttonXSpin_->setValue(event_->mouseX);
            buttonYSpin_->setValue(event_->mouseY);
            break;
            
        case EventType::MouseWheel:
            wheelDeltaSpin_->setValue(event_->mouseWheelDelta);
            wheelXSpin_->setValue(event_->mouseX);
            wheelYSpin_->setValue(event_->mouseY);
            break;
            
        case EventType::KeyDown:
        case EventType::KeyUp:
            keyCodeSpin_->setValue(static_cast<int>(event_->keyCode));
            break;
            
        case EventType::Wait:
            waitMsSpin_->setValue(static_cast<int>(event_->waitMicroseconds / 1000));
            break;
            
        case EventType::MouseTeleport:
            teleportXSpin_->setValue(event_->mouseX);
            teleportYSpin_->setValue(event_->mouseY);
            break;
    }
}

void PropertyEditor::updateEventFromPanel()
{
    editedEvent_.type = static_cast<EventType>(typeCombo_->currentData().toInt());
    editedEvent_.timestampMicroseconds = static_cast<std::uint64_t>(timestampSpin_->value());
    
    switch (editedEvent_.type) {
        case EventType::MouseMove:
            editedEvent_.mouseDeltaX = moveDeltaXSpin_->value();
            editedEvent_.mouseDeltaY = moveDeltaYSpin_->value();
            editedEvent_.mouseX = moveXSpin_->value();
            editedEvent_.mouseY = moveYSpin_->value();
            break;
            
        case EventType::MouseButtonDown:
        case EventType::MouseButtonUp:
            editedEvent_.mouseButton = buttonCombo_->currentData().toInt();
            editedEvent_.mouseX = buttonXSpin_->value();
            editedEvent_.mouseY = buttonYSpin_->value();
            break;
            
        case EventType::MouseWheel:
            editedEvent_.mouseWheelDelta = wheelDeltaSpin_->value();
            editedEvent_.mouseX = wheelXSpin_->value();
            editedEvent_.mouseY = wheelYSpin_->value();
            break;
            
        case EventType::KeyDown:
        case EventType::KeyUp:
            editedEvent_.keyCode = static_cast<unsigned int>(keyCodeSpin_->value());
            break;
            
        case EventType::Wait:
            editedEvent_.waitMicroseconds = static_cast<std::uint64_t>(waitMsSpin_->value()) * 1000;
            break;
            
        case EventType::MouseTeleport:
            editedEvent_.mouseX = teleportXSpin_->value();
            editedEvent_.mouseY = teleportYSpin_->value();
            break;
    }
}

int PropertyEditor::panelIndexForEventType(EventType type) const
{
    switch (type) {
        case EventType::MouseMove:
            return 0;
        case EventType::MouseButtonDown:
        case EventType::MouseButtonUp:
            return 1;
        case EventType::MouseWheel:
            return 2;
        case EventType::KeyDown:
        case EventType::KeyUp:
            return 3;
        case EventType::Wait:
            return 4;
        case EventType::MouseTeleport:
            return 5;
        default:
            return 0;
    }
}

QString PropertyEditor::buttonNameFromNumber(int button) const
{
    switch (button) {
        case 1: return tr("Left");
        case 2: return tr("Right");
        case 3: return tr("Middle");
        case 4: return tr("X1");
        case 5: return tr("X2");
        default: return tr("Button %1").arg(button);
    }
}
