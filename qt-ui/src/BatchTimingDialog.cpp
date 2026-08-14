#include "BatchTimingDialog.h"
#include "DarkStyle.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QRadioButton>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QGroupBox>
#include <QButtonGroup>

BatchTimingDialog::BatchTimingDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Batch Timing Operation"));
    setMinimumWidth(400);
    setupUi();
}

void BatchTimingDialog::setupUi()
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setSpacing(16);
    
    // Title
    QLabel* titleLabel = new QLabel(tr("Modify Event Timing"));
    titleLabel->setProperty("heading", true);
    layout->addWidget(titleLabel);
    
    // Operation selection
    QGroupBox* opGroup = new QGroupBox(tr("Operation"));
    QVBoxLayout* opLayout = new QVBoxLayout(opGroup);
    
    QButtonGroup* buttonGroup = new QButtonGroup(this);
    
    // Scale option
    QHBoxLayout* scaleLayout = new QHBoxLayout();
    scaleRadio_ = new QRadioButton(tr("Scale timing by:"));
    scaleRadio_->setChecked(true);
    buttonGroup->addButton(scaleRadio_);
    scaleLayout->addWidget(scaleRadio_);
    
    scaleSpin_ = new QDoubleSpinBox();
    scaleSpin_->setRange(0.1, 10.0);
    scaleSpin_->setValue(1.0);
    scaleSpin_->setSingleStep(0.1);
    scaleSpin_->setDecimals(2);
    scaleSpin_->setSuffix("x");
    scaleSpin_->setToolTip(tr("Multiply all delays by this factor\n0.5 = twice as fast, 2.0 = twice as slow"));
    scaleLayout->addWidget(scaleSpin_);
    scaleLayout->addStretch();
    opLayout->addLayout(scaleLayout);
    
    // Offset option
    QHBoxLayout* offsetLayout = new QHBoxLayout();
    offsetRadio_ = new QRadioButton(tr("Offset timing by:"));
    buttonGroup->addButton(offsetRadio_);
    offsetLayout->addWidget(offsetRadio_);
    
    offsetSpin_ = new QSpinBox();
    offsetSpin_->setRange(-60000, 60000);
    offsetSpin_->setValue(0);
    offsetSpin_->setSuffix(" ms");
    offsetSpin_->setToolTip(tr("Add/subtract milliseconds from all delays\nPositive = slower, Negative = faster"));
    offsetLayout->addWidget(offsetSpin_);
    offsetLayout->addStretch();
    opLayout->addLayout(offsetLayout);
    
    // Minimum delay option
    QHBoxLayout* minDelayLayout = new QHBoxLayout();
    minDelayRadio_ = new QRadioButton(tr("Set minimum delay:"));
    buttonGroup->addButton(minDelayRadio_);
    minDelayLayout->addWidget(minDelayRadio_);
    
    minDelaySpin_ = new QSpinBox();
    minDelaySpin_->setRange(0, 10000);
    minDelaySpin_->setValue(10);
    minDelaySpin_->setSuffix(" ms");
    minDelaySpin_->setToolTip(tr("Ensure all delays are at least this long"));
    minDelayLayout->addWidget(minDelaySpin_);
    minDelayLayout->addStretch();
    opLayout->addLayout(minDelayLayout);
    
    layout->addWidget(opGroup);
    
    // Scope
    selectionOnlyCheck_ = new QCheckBox(tr("Apply to selected events only"));
    selectionOnlyCheck_->setToolTip(tr("If unchecked, applies to all events"));
    layout->addWidget(selectionOnlyCheck_);
    
    // Enable/disable spin boxes based on selection
    connect(scaleRadio_, &QRadioButton::toggled, scaleSpin_, &QWidget::setEnabled);
    connect(offsetRadio_, &QRadioButton::toggled, offsetSpin_, &QWidget::setEnabled);
    connect(minDelayRadio_, &QRadioButton::toggled, minDelaySpin_, &QWidget::setEnabled);
    
    // Initial state
    offsetSpin_->setEnabled(false);
    minDelaySpin_->setEnabled(false);
    
    // Buttons
    QDialogButtonBox* buttons = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(buttons);
}

BatchTimingDialog::Operation BatchTimingDialog::operation() const
{
    if (scaleRadio_->isChecked()) return Operation::Scale;
    if (offsetRadio_->isChecked()) return Operation::Offset;
    return Operation::SetMinDelay;
}

double BatchTimingDialog::scaleFactor() const
{
    return scaleSpin_->value();
}

int BatchTimingDialog::offsetMs() const
{
    return offsetSpin_->value();
}

int BatchTimingDialog::minDelayMs() const
{
    return minDelaySpin_->value();
}

bool BatchTimingDialog::applyToSelection() const
{
    return selectionOnlyCheck_->isChecked();
}
