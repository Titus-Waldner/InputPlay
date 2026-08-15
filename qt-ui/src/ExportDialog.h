#pragma once

#include <QDialog>
#include <QString>

class QLineEdit;
class QCheckBox;
class QComboBox;
class QPushButton;

class ExportDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ExportDialog(QWidget* parent = nullptr);
    
    enum class Format {
        CSV,
        TSV,
        JSON
    };
    
    QString filePath() const;
    Format format() const;
    bool includeHeaders() const;
    bool exportSelection() const;

private slots:
    void browseFile();

private:
    void setupUi();
    
    QLineEdit* pathEdit_ = nullptr;
    QPushButton* browseButton_ = nullptr;
    QComboBox* formatCombo_ = nullptr;
    QCheckBox* headersCheck_ = nullptr;
    QCheckBox* selectionCheck_ = nullptr;
};
