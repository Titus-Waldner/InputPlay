#include "ExportDialog.h"
#include "DarkStyle.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>
#include <QComboBox>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QGroupBox>

ExportDialog::ExportDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Export Events"));
    setMinimumWidth(450);
    setupUi();
}

void ExportDialog::setupUi()
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setSpacing(16);
    
    // Title
    QLabel* titleLabel = new QLabel(tr("Export Event Data"));
    titleLabel->setProperty("heading", true);
    layout->addWidget(titleLabel);
    
    // File path
    QGroupBox* fileGroup = new QGroupBox(tr("Output File"));
    QHBoxLayout* fileLayout = new QHBoxLayout(fileGroup);
    
    pathEdit_ = new QLineEdit();
    pathEdit_->setPlaceholderText(tr("Select output file..."));
    fileLayout->addWidget(pathEdit_, 1);
    
    browseButton_ = new QPushButton(tr("Browse..."));
    connect(browseButton_, &QPushButton::clicked, this, &ExportDialog::browseFile);
    fileLayout->addWidget(browseButton_);
    
    layout->addWidget(fileGroup);
    
    // Format options
    QGroupBox* optionsGroup = new QGroupBox(tr("Options"));
    QVBoxLayout* optionsLayout = new QVBoxLayout(optionsGroup);
    
    QHBoxLayout* formatLayout = new QHBoxLayout();
    formatLayout->addWidget(new QLabel(tr("Format:")));
    
    formatCombo_ = new QComboBox();
    formatCombo_->addItem(tr("CSV (Comma Separated)"), static_cast<int>(Format::CSV));
    formatCombo_->addItem(tr("TSV (Tab Separated)"), static_cast<int>(Format::TSV));
    formatCombo_->addItem(tr("JSON"), static_cast<int>(Format::JSON));
    formatLayout->addWidget(formatCombo_);
    formatLayout->addStretch();
    optionsLayout->addLayout(formatLayout);
    
    headersCheck_ = new QCheckBox(tr("Include column headers"));
    headersCheck_->setChecked(true);
    optionsLayout->addWidget(headersCheck_);
    
    selectionCheck_ = new QCheckBox(tr("Export selected events only"));
    optionsLayout->addWidget(selectionCheck_);
    
    layout->addWidget(optionsGroup);
    
    // Info
    QLabel* infoLabel = new QLabel(
        tr("Exports event data including:\n"
           "• Index, Type, Timestamp, Duration\n"
           "• Coordinates (X, Y), Delta values\n"
           "• Key codes, Button numbers"));
    infoLabel->setProperty(
		"subheading",
		true);
    layout->addWidget(infoLabel);
    
    // Buttons
    QDialogButtonBox* buttons = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    buttons->button(QDialogButtonBox::Ok)->setText(tr("Export"));
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(buttons);
}

void ExportDialog::browseFile()
{
    QString filter;
    switch (format()) {
        case Format::CSV:
            filter = tr("CSV Files (*.csv);;All Files (*)");
            break;
        case Format::TSV:
            filter = tr("TSV Files (*.tsv *.txt);;All Files (*)");
            break;
        case Format::JSON:
            filter = tr("JSON Files (*.json);;All Files (*)");
            break;
    }
    
    QString path = QFileDialog::getSaveFileName(
        this, tr("Export Events"), QString(), filter);
    
    if (!path.isEmpty()) {
        pathEdit_->setText(path);
    }
}

QString ExportDialog::filePath() const
{
    return pathEdit_->text();
}

ExportDialog::Format ExportDialog::format() const
{
    return static_cast<Format>(formatCombo_->currentData().toInt());
}

bool ExportDialog::includeHeaders() const
{
    return headersCheck_->isChecked();
}

bool ExportDialog::exportSelection() const
{
    return selectionCheck_->isChecked();
}
