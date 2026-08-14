#include "GoToEventDialog.h"
#include "DarkStyle.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QSpinBox>
#include <QDialogButtonBox>
#include <QPushButton>

GoToEventDialog::GoToEventDialog(int maxEvent, int currentEvent, QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Go to Event"));
    setFixedSize(300, 120);
    setupUi(maxEvent, currentEvent);
}

void GoToEventDialog::setupUi(int maxEvent, int currentEvent)
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setSpacing(16);
    
    // Event number input
    QHBoxLayout* inputLayout = new QHBoxLayout();
    
    QLabel* label = new QLabel(tr("Event number:"));
    inputLayout->addWidget(label);
    
    eventSpin_ = new QSpinBox();
    eventSpin_->setRange(1, maxEvent);
    eventSpin_->setValue(currentEvent > 0 ? currentEvent : 1);
    eventSpin_->selectAll();
    eventSpin_->setFocus();
    inputLayout->addWidget(eventSpin_, 1);
    
    QLabel* rangeLabel =
		new QLabel(
			tr("(1 - %1)")
				.arg(maxEvent));

	DarkStyle::setTone(
		rangeLabel,
		"secondary");

	inputLayout->addWidget(
		rangeLabel);
    
    layout->addLayout(inputLayout);
    
    // Buttons
    QDialogButtonBox* buttons = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(buttons);
    
    // Enter key triggers OK
    connect(eventSpin_, &QSpinBox::editingFinished, this, [this]() {
        // Don't auto-accept, just validate
    });
}

int GoToEventDialog::selectedEvent() const
{
    return eventSpin_->value();
}
