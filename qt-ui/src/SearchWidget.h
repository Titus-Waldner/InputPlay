#pragma once

#include "Recording.h"

#include <QWidget>
#include <QLineEdit>
#include <QList>

class QLabel;
class QPushButton;
class QCheckBox;
class QComboBox;

class SearchWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SearchWidget(QWidget* parent = nullptr);
    
    void setRecording(Recording* recording);
    void focusSearch();
    
    QList<int> findMatches() const;

signals:
    void searchChanged();
    void navigateToResult(int eventIndex);
    void highlightResults(const QList<int>& indices);

public slots:
    void nextResult();
    void previousResult();
    void clearSearch();

private slots:
    void onSearchTextChanged(const QString& text);
    void updateResults();

private:
    void setupUi();
    bool eventMatchesSearch(const InputEvent& event, int index) const;
    QString eventToSearchString(const InputEvent& event) const;
    QString getKeyName(int keyCode) const;
    
    Recording* recording_ = nullptr;
    
    QLineEdit* searchEdit_ = nullptr;
    QCheckBox* caseSensitiveCheck_ = nullptr;
    QCheckBox* regexCheck_ = nullptr;
    QComboBox* filterTypeCombo_ = nullptr;
    
    QPushButton* prevButton_ = nullptr;
    QPushButton* nextButton_ = nullptr;
    QPushButton* clearButton_ = nullptr;
    
    QLabel* resultsLabel_ = nullptr;
    
    QList<int> matchingIndices_;
    int currentResultIndex_ = -1;
};
