#include "EventCreationDialog.h"
#include "DarkStyle.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QComboBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QStackedWidget>
#include <QDialogButtonBox>
#include <QLabel>
#include <QGroupBox>
#include <QLineEdit>

EventCreationDialog::EventCreationDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Create Event"));
    setMinimumWidth(400);
    setupUi();
}

void EventCreationDialog::setupUi()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    // Event type selection
    QGroupBox* typeGroup = new QGroupBox(tr("Event Type"));
    QFormLayout* typeLayout = new QFormLayout(typeGroup);
    
    typeCombo_ = new QComboBox();
    typeCombo_->addItem(tr("Mouse Move"), static_cast<int>(EventType::MouseMove));
    typeCombo_->addItem(tr("Mouse Button"), -1);  // Special: will set down/up
    typeCombo_->addItem(tr("Mouse Wheel"), static_cast<int>(EventType::MouseWheel));
    typeCombo_->addItem(tr("Key Press"), -2);  // Special: will set down/up
    typeCombo_->addItem(tr("Wait"), static_cast<int>(EventType::Wait));
    typeCombo_->addItem(tr("Mouse Teleport"), static_cast<int>(EventType::MouseTeleport));
    typeLayout->addRow(tr("Type:"), typeCombo_);
    
    timestampSpin_ = new QSpinBox();
    timestampSpin_->setRange(0, 2147483647);
    timestampSpin_->setSuffix(tr(" ms"));
    timestampSpin_->setToolTip(tr("Event timestamp in milliseconds"));
    typeLayout->addRow(tr("Timestamp:"), timestampSpin_);
    
    mainLayout->addWidget(typeGroup);
    
    connect(typeCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &EventCreationDialog::onTypeChanged);
    
    // Stacked panels for type-specific options
    panelStack_ = new QStackedWidget();
    
    setupMouseMovePanel();
    setupMouseButtonPanel();
    setupMouseWheelPanel();
    setupKeyPanel();
    setupWaitPanel();
    setupTeleportPanel();
    
    mainLayout->addWidget(panelStack_);
    
    // Buttons
    buttonBox_ = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox_, &QDialogButtonBox::accepted, this, &EventCreationDialog::accept);
    connect(buttonBox_, &QDialogButtonBox::rejected, this, &QDialog::reject);
    
    mainLayout->addWidget(buttonBox_);
    
    // Set default type
    onTypeChanged(0);
}

void EventCreationDialog::setupMouseMovePanel()
{
    QWidget* panel = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(panel);
    
    QGroupBox* group = new QGroupBox(tr("Mouse Movement"));
    QFormLayout* formLayout = new QFormLayout(group);
    
    QHBoxLayout* deltaLayout = new QHBoxLayout();
    moveDeltaXSpin_ = new QSpinBox();
    moveDeltaXSpin_->setRange(-10000, 10000);
    moveDeltaXSpin_->setPrefix(tr("ΔX: "));
    deltaLayout->addWidget(moveDeltaXSpin_);
    
    moveDeltaYSpin_ = new QSpinBox();
    moveDeltaYSpin_->setRange(-10000, 10000);
    moveDeltaYSpin_->setPrefix(tr("ΔY: "));
    deltaLayout->addWidget(moveDeltaYSpin_);
    formLayout->addRow(tr("Delta:"), deltaLayout);
    
    QHBoxLayout* posLayout = new QHBoxLayout();
    moveXSpin_ = new QSpinBox();
    moveXSpin_->setRange(-10000, 100000);
    moveXSpin_->setPrefix(tr("X: "));
    posLayout->addWidget(moveXSpin_);
    
    moveYSpin_ = new QSpinBox();
    moveYSpin_->setRange(-10000, 100000);
    moveYSpin_->setPrefix(tr("Y: "));
    posLayout->addWidget(moveYSpin_);
    formLayout->addRow(tr("Position:"), posLayout);
    
    layout->addWidget(group);
    layout->addStretch();
    
    panelStack_->addWidget(panel);
}

void EventCreationDialog::setupMouseButtonPanel()
{
    QWidget* panel = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(panel);
    
    QGroupBox* group = new QGroupBox(tr("Mouse Button"));
    QFormLayout* formLayout = new QFormLayout(group);
    
    buttonActionCombo_ = new QComboBox();
    buttonActionCombo_->addItem(tr("Press (Down)"), static_cast<int>(EventType::MouseButtonDown));
    buttonActionCombo_->addItem(tr("Release (Up)"), static_cast<int>(EventType::MouseButtonUp));
    buttonActionCombo_->addItem(tr("Click (Down + Up)"), -1);
    formLayout->addRow(tr("Action:"), buttonActionCombo_);
    
    buttonCombo_ = new QComboBox();
    buttonCombo_->addItem(tr("Left Button"), 1);
    buttonCombo_->addItem(tr("Right Button"), 2);
    buttonCombo_->addItem(tr("Middle Button"), 3);
    buttonCombo_->addItem(tr("X1 Button"), 4);
    buttonCombo_->addItem(tr("X2 Button"), 5);
    formLayout->addRow(tr("Button:"), buttonCombo_);
    
    QHBoxLayout* posLayout = new QHBoxLayout();
    buttonXSpin_ = new QSpinBox();
    buttonXSpin_->setRange(-10000, 100000);
    buttonXSpin_->setPrefix(tr("X: "));
    posLayout->addWidget(buttonXSpin_);
    
    buttonYSpin_ = new QSpinBox();
    buttonYSpin_->setRange(-10000, 100000);
    buttonYSpin_->setPrefix(tr("Y: "));
    posLayout->addWidget(buttonYSpin_);
    formLayout->addRow(tr("Position:"), posLayout);
    
    layout->addWidget(group);
    layout->addStretch();
    
    panelStack_->addWidget(panel);
}

void EventCreationDialog::setupMouseWheelPanel()
{
    QWidget* panel = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(panel);
    
    QGroupBox* group = new QGroupBox(tr("Mouse Wheel"));
    QFormLayout* formLayout = new QFormLayout(group);
    
    wheelDeltaSpin_ = new QSpinBox();
    wheelDeltaSpin_->setRange(-10000, 10000);
    wheelDeltaSpin_->setValue(120);
    wheelDeltaSpin_->setToolTip(tr("Positive = scroll up, Negative = scroll down. 120 = one notch."));
    formLayout->addRow(tr("Delta:"), wheelDeltaSpin_);
    
    QHBoxLayout* posLayout = new QHBoxLayout();
    wheelXSpin_ = new QSpinBox();
    wheelXSpin_->setRange(-10000, 100000);
    wheelXSpin_->setPrefix(tr("X: "));
    posLayout->addWidget(wheelXSpin_);
    
    wheelYSpin_ = new QSpinBox();
    wheelYSpin_->setRange(-10000, 100000);
    wheelYSpin_->setPrefix(tr("Y: "));
    posLayout->addWidget(wheelYSpin_);
    formLayout->addRow(tr("Position:"), posLayout);
    
    layout->addWidget(group);
    layout->addStretch();
    
    panelStack_->addWidget(panel);
}

void EventCreationDialog::setupKeyPanel()
{
    QWidget* panel = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(panel);
    
    QGroupBox* group = new QGroupBox(tr("Keyboard"));
    QFormLayout* formLayout = new QFormLayout(group);
    
    keyActionCombo_ = new QComboBox();
    keyActionCombo_->addItem(tr("Press (Down)"), static_cast<int>(EventType::KeyDown));
    keyActionCombo_->addItem(tr("Release (Up)"), static_cast<int>(EventType::KeyUp));
    keyActionCombo_->addItem(tr("Tap (Down + Up)"), -1);
    formLayout->addRow(tr("Action:"), keyActionCombo_);
    
    commonKeyCombo_ = new QComboBox();
    commonKeyCombo_->addItem(tr("-- Select Common Key --"), 0);
    commonKeyCombo_->addItem(tr("A"), 0x41);
    commonKeyCombo_->addItem(tr("B"), 0x42);
    commonKeyCombo_->addItem(tr("C"), 0x43);
    commonKeyCombo_->addItem(tr("D"), 0x44);
    commonKeyCombo_->addItem(tr("E"), 0x45);
    commonKeyCombo_->addItem(tr("W"), 0x57);
    commonKeyCombo_->addItem(tr("S"), 0x53);
    commonKeyCombo_->addItem(tr("Space"), 0x20);
    commonKeyCombo_->addItem(tr("Enter"), 0x0D);
    commonKeyCombo_->addItem(tr("Escape"), 0x1B);
    commonKeyCombo_->addItem(tr("Tab"), 0x09);
    commonKeyCombo_->addItem(tr("Shift"), 0x10);
    commonKeyCombo_->addItem(tr("Ctrl"), 0x11);
    commonKeyCombo_->addItem(tr("Alt"), 0x12);
    commonKeyCombo_->addItem(tr("Backspace"), 0x08);
    commonKeyCombo_->addItem(tr("Delete"), 0x2E);
    commonKeyCombo_->addItem(tr("Left Arrow"), 0x25);
    commonKeyCombo_->addItem(tr("Up Arrow"), 0x26);
    commonKeyCombo_->addItem(tr("Right Arrow"), 0x27);
    commonKeyCombo_->addItem(tr("Down Arrow"), 0x28);
    commonKeyCombo_->addItem(tr("F1"), 0x70);
    commonKeyCombo_->addItem(tr("F2"), 0x71);
    commonKeyCombo_->addItem(tr("F3"), 0x72);
    commonKeyCombo_->addItem(tr("F4"), 0x73);
    commonKeyCombo_->addItem(tr("F5"), 0x74);
    formLayout->addRow(tr("Common Keys:"), commonKeyCombo_);
    
    keyCodeSpin_ = new QSpinBox();
    keyCodeSpin_->setRange(0, 255);
    keyCodeSpin_->setDisplayIntegerBase(16);
    keyCodeSpin_->setPrefix("0x");
    formLayout->addRow(tr("Key Code:"), keyCodeSpin_);
    
    keyNameDisplay_ = new QLineEdit();
    keyNameDisplay_->setReadOnly(true);
    formLayout->addRow(tr("Key Name:"), keyNameDisplay_);
    
    // Connect common key to code
    connect(commonKeyCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index) {
        int code = commonKeyCombo_->itemData(index).toInt();
        if (code > 0) {
            keyCodeSpin_->setValue(code);
        }
    });
    
    // Update name when code changes
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
            case 0x25: name = "Left"; break;
            case 0x26: name = "Up"; break;
            case 0x27: name = "Right"; break;
            case 0x28: name = "Down"; break;
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
        }
        keyNameDisplay_->setText(name);
    });
    
    layout->addWidget(group);
    layout->addStretch();
    
    panelStack_->addWidget(panel);
}

void EventCreationDialog::setupWaitPanel()
{
    QWidget* panel = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(panel);
    
    QGroupBox* group = new QGroupBox(tr("Wait Duration"));
    QFormLayout* formLayout = new QFormLayout(group);
    
    waitSecondsSpin_ = new QDoubleSpinBox();
    waitSecondsSpin_->setRange(0.001, 3600.0);
    waitSecondsSpin_->setDecimals(3);
    waitSecondsSpin_->setValue(0.1);
    waitSecondsSpin_->setSuffix(tr(" seconds"));
    waitSecondsSpin_->setToolTip(tr("Wait time in seconds (0.001 = 1ms)"));
    formLayout->addRow(tr("Duration:"), waitSecondsSpin_);
    
    layout->addWidget(group);
    layout->addStretch();
    
    panelStack_->addWidget(panel);
}

void EventCreationDialog::setupTeleportPanel()
{
    QWidget* panel = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(panel);
    
    QGroupBox* group = new QGroupBox(tr("Teleport Position"));
    QFormLayout* formLayout = new QFormLayout(group);
    
    QHBoxLayout* posLayout = new QHBoxLayout();
    teleportXSpin_ = new QSpinBox();
    teleportXSpin_->setRange(-10000, 100000);
    teleportXSpin_->setPrefix(tr("X: "));
    posLayout->addWidget(teleportXSpin_);
    
    teleportYSpin_ = new QSpinBox();
    teleportYSpin_->setRange(-10000, 100000);
    teleportYSpin_->setPrefix(tr("Y: "));
    posLayout->addWidget(teleportYSpin_);
    formLayout->addRow(tr("Position:"), posLayout);
    
    QLabel* hint = new QLabel(tr("Teleport events are used to move the cursor\n"
                                  "to a specific position, typically after a pause."));
    hint->setProperty("subheading", true);
    layout->addWidget(group);
    layout->addWidget(hint);
    layout->addStretch();
    
    panelStack_->addWidget(panel);
}

void EventCreationDialog::onTypeChanged(int index)
{
    panelStack_->setCurrentIndex(index);
}

void EventCreationDialog::setTimestamp(std::uint64_t timestamp)
{
    timestampSpin_->setValue(static_cast<int>(timestamp / 1000));  // Convert µs to ms
}

void EventCreationDialog::setEventType(EventType type)
{
    int panelIndex = panelIndexForType(type);
    
    for (int i = 0; i < typeCombo_->count(); ++i) {
        int data = typeCombo_->itemData(i).toInt();
        if (data == static_cast<int>(type) || 
            (data == -1 && (type == EventType::MouseButtonDown || type == EventType::MouseButtonUp)) ||
            (data == -2 && (type == EventType::KeyDown || type == EventType::KeyUp))) {
            typeCombo_->setCurrentIndex(i);
            break;
        }
    }
    
    panelStack_->setCurrentIndex(panelIndex);
}

int EventCreationDialog::panelIndexForType(EventType type) const
{
    switch (type) {
        case EventType::MouseMove: return 0;
        case EventType::MouseButtonDown:
        case EventType::MouseButtonUp: return 1;
        case EventType::MouseWheel: return 2;
        case EventType::KeyDown:
        case EventType::KeyUp: return 3;
        case EventType::Wait: return 4;
        case EventType::MouseTeleport: return 5;
        default: return 0;
    }
}

void EventCreationDialog::accept()
{
    populateEventFromUi();
    QDialog::accept();
}

void EventCreationDialog::populateEventFromUi()
{
    event_ = InputEvent();
    event_.timestampMicroseconds = static_cast<std::uint64_t>(timestampSpin_->value()) * 1000;
    
    int typeData = typeCombo_->currentData().toInt();
    int panelIndex = typeCombo_->currentIndex();
    
    switch (panelIndex) {
        case 0:  // Mouse Move
            event_.type = EventType::MouseMove;
            event_.mouseDeltaX = moveDeltaXSpin_->value();
            event_.mouseDeltaY = moveDeltaYSpin_->value();
            event_.mouseX = moveXSpin_->value();
            event_.mouseY = moveYSpin_->value();
            break;
            
        case 1:  // Mouse Button
            {
                int action = buttonActionCombo_->currentData().toInt();
                if (action == -1) {
                    // Click = Down (we'll just create Down, user can add Up separately)
                    event_.type = EventType::MouseButtonDown;
                } else {
                    event_.type = static_cast<EventType>(action);
                }
                event_.mouseButton = buttonCombo_->currentData().toInt();
                event_.mouseX = buttonXSpin_->value();
                event_.mouseY = buttonYSpin_->value();
            }
            break;
            
        case 2:  // Mouse Wheel
            event_.type = EventType::MouseWheel;
            event_.mouseWheelDelta = wheelDeltaSpin_->value();
            event_.mouseX = wheelXSpin_->value();
            event_.mouseY = wheelYSpin_->value();
            break;
            
        case 3:  // Key
            {
                int action = keyActionCombo_->currentData().toInt();
                if (action == -1) {
                    // Tap = Down (user can add Up separately)
                    event_.type = EventType::KeyDown;
                } else {
                    event_.type = static_cast<EventType>(action);
                }
                event_.keyCode = static_cast<unsigned int>(keyCodeSpin_->value());
            }
            break;
            
        case 4:  // Wait
            event_.type = EventType::Wait;
            event_.waitMicroseconds = static_cast<std::uint64_t>(waitSecondsSpin_->value() * 1'000'000);
            break;
            
        case 5:  // Teleport
            event_.type = EventType::MouseTeleport;
            event_.mouseX = teleportXSpin_->value();
            event_.mouseY = teleportYSpin_->value();
            break;
    }
}
