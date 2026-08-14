#pragma once

#include "InputEvent.h"

#include <QWidget>

class QLabel;
class QSpinBox;
class QComboBox;
class QLineEdit;
class QStackedWidget;
class QFormLayout;
class QPushButton;

class PropertyEditor : public QWidget
{
    Q_OBJECT

public:
    explicit PropertyEditor(QWidget* parent = nullptr);
    
    void setEvent(const InputEvent* event, int index);
    void clear();
    
    const InputEvent* currentEvent() const { return event_; }
    int currentIndex() const { return eventIndex_; }

signals:
    void eventModified(int index, const InputEvent& oldEvent, const InputEvent& newEvent);

public slots:
    void applyChanges();
    void revertChanges();
    
    InputEvent editedEventCopy() const { return editedEvent_; }

private:
    void setupUi();
    void setupMouseMovePanel();
    void setupMouseButtonPanel();
    void setupMouseWheelPanel();
    void setupKeyPanel();
    void setupWaitPanel();
    void setupTeleportPanel();
    
    void updatePanelFromEvent();
    void updateEventFromPanel();
    
    int panelIndexForEventType(EventType type) const;
    QString buttonNameFromNumber(int button) const;
    
    const InputEvent* event_ = nullptr;
    InputEvent editedEvent_;
    int eventIndex_ = -1;
    
    // Common widgets
    QLabel* titleLabel_ = nullptr;
    QLabel* indexLabel_ = nullptr;
    QComboBox* typeCombo_ = nullptr;
    QSpinBox* timestampSpin_ = nullptr;
    
    // Stacked widget for type-specific editors
    QStackedWidget* panelStack_ = nullptr;
    
    // Mouse Move panel
    QWidget* mouseMovePanel_ = nullptr;
    QSpinBox* moveDeltaXSpin_ = nullptr;
    QSpinBox* moveDeltaYSpin_ = nullptr;
    QSpinBox* moveXSpin_ = nullptr;
    QSpinBox* moveYSpin_ = nullptr;
    
    // Mouse Button panel
    QWidget* mouseButtonPanel_ = nullptr;
    QComboBox* buttonCombo_ = nullptr;
    QSpinBox* buttonXSpin_ = nullptr;
    QSpinBox* buttonYSpin_ = nullptr;
    
    // Mouse Wheel panel
    QWidget* mouseWheelPanel_ = nullptr;
    QSpinBox* wheelDeltaSpin_ = nullptr;
    QSpinBox* wheelXSpin_ = nullptr;
    QSpinBox* wheelYSpin_ = nullptr;
    
    // Key panel
    QWidget* keyPanel_ = nullptr;
    QSpinBox* keyCodeSpin_ = nullptr;
    QLineEdit* keyNameEdit_ = nullptr;
    
    // Wait panel
    QWidget* waitPanel_ = nullptr;
    QSpinBox* waitMsSpin_ = nullptr;
    
    // Teleport panel
    QWidget* teleportPanel_ = nullptr;
    QSpinBox* teleportXSpin_ = nullptr;
    QSpinBox* teleportYSpin_ = nullptr;
    
    // Action buttons
    QPushButton* applyButton_ = nullptr;
    QPushButton* revertButton_ = nullptr;
};
