#pragma once

#include "InputEvent.h"

#include <QDialog>

class QComboBox;
class QSpinBox;
class QDoubleSpinBox;
class QStackedWidget;
class QDialogButtonBox;
class QLineEdit;

class EventCreationDialog : public QDialog
{
    Q_OBJECT

public:
    explicit EventCreationDialog(QWidget* parent = nullptr);
    
    InputEvent createdEvent() const { return event_; }
    
    void setTimestamp(std::uint64_t timestamp);
    void setEventType(EventType type);

private slots:
    void onTypeChanged(int index);
    void accept() override;

private:
    void setupUi();
    void setupMouseMovePanel();
    void setupMouseButtonPanel();
    void setupMouseWheelPanel();
    void setupKeyPanel();
    void setupWaitPanel();
    void setupTeleportPanel();
    
    void populateEventFromUi();
    int panelIndexForType(EventType type) const;
    
    InputEvent event_;
    
    // Type selection
    QComboBox* typeCombo_ = nullptr;
    QSpinBox* timestampSpin_ = nullptr;
    
    // Panels
    QStackedWidget* panelStack_ = nullptr;
    
    // Mouse Move
    QSpinBox* moveDeltaXSpin_ = nullptr;
    QSpinBox* moveDeltaYSpin_ = nullptr;
    QSpinBox* moveXSpin_ = nullptr;
    QSpinBox* moveYSpin_ = nullptr;
    
    // Mouse Button
    QComboBox* buttonActionCombo_ = nullptr;
    QComboBox* buttonCombo_ = nullptr;
    QSpinBox* buttonXSpin_ = nullptr;
    QSpinBox* buttonYSpin_ = nullptr;
    
    // Mouse Wheel
    QSpinBox* wheelDeltaSpin_ = nullptr;
    QSpinBox* wheelXSpin_ = nullptr;
    QSpinBox* wheelYSpin_ = nullptr;
    
    // Key
    QComboBox* keyActionCombo_ = nullptr;
    QComboBox* commonKeyCombo_ = nullptr;
    QSpinBox* keyCodeSpin_ = nullptr;
    QLineEdit* keyNameDisplay_ = nullptr;
    
    // Wait
    QDoubleSpinBox* waitSecondsSpin_ = nullptr;
    
    // Teleport
    QSpinBox* teleportXSpin_ = nullptr;
    QSpinBox* teleportYSpin_ = nullptr;
    
    QDialogButtonBox* buttonBox_ = nullptr;
};
