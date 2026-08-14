#pragma once

#include <QDialog>

class QDoubleSpinBox;
class QSpinBox;
class QRadioButton;
class QCheckBox;
class QDialogButtonBox;

class BatchTimingDialog : public QDialog
{
    Q_OBJECT

public:
    explicit BatchTimingDialog(QWidget* parent = nullptr);
    
    enum class Operation {
        Scale,
        Offset,
        SetMinDelay
    };
    
    Operation operation() const;
    double scaleFactor() const;
    int offsetMs() const;
    int minDelayMs() const;
    bool applyToSelection() const;

private:
    void setupUi();
    void updatePreview();
    
    QRadioButton* scaleRadio_ = nullptr;
    QRadioButton* offsetRadio_ = nullptr;
    QRadioButton* minDelayRadio_ = nullptr;
    
    QDoubleSpinBox* scaleSpin_ = nullptr;
    QSpinBox* offsetSpin_ = nullptr;
    QSpinBox* minDelaySpin_ = nullptr;
    
    QCheckBox* selectionOnlyCheck_ = nullptr;
};
