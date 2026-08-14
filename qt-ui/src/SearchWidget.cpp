#include "SearchWidget.h"
#include "DarkStyle.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QCheckBox>
#include <QComboBox>
#include <QRegularExpression>

SearchWidget::SearchWidget(QWidget* parent)
    : QWidget(parent)
{
    setupUi();
}

void SearchWidget::setupUi()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(8, 8, 8, 8);
    mainLayout->setSpacing(8);
    
    // Search bar row
    QHBoxLayout* searchLayout = new QHBoxLayout();
    searchLayout->setSpacing(4);
    
    QLabel* searchIcon = new QLabel(tr("🔍"));
    searchLayout->addWidget(searchIcon);
    
    searchEdit_ = new QLineEdit();
    searchEdit_->setPlaceholderText(tr("Search events..."));
    searchEdit_->setClearButtonEnabled(true);
    connect(searchEdit_, &QLineEdit::textChanged, this, &SearchWidget::onSearchTextChanged);
    connect(searchEdit_, &QLineEdit::returnPressed, this, &SearchWidget::nextResult);
    searchLayout->addWidget(searchEdit_, 1);
    
    prevButton_ = new QPushButton(tr("◀"));
    prevButton_->setMaximumWidth(30);
    prevButton_->setToolTip(tr("Previous result (Shift+F3)"));
    prevButton_->setEnabled(false);
    connect(prevButton_, &QPushButton::clicked, this, &SearchWidget::previousResult);
    searchLayout->addWidget(prevButton_);
    
    nextButton_ = new QPushButton(tr("▶"));
    nextButton_->setMaximumWidth(30);
    nextButton_->setToolTip(tr("Next result (F3)"));
    nextButton_->setEnabled(false);
    connect(nextButton_, &QPushButton::clicked, this, &SearchWidget::nextResult);
    searchLayout->addWidget(nextButton_);
    
    clearButton_ = new QPushButton(tr("Clear"));
    clearButton_->setToolTip(tr("Clear search"));
    connect(clearButton_, &QPushButton::clicked, this, &SearchWidget::clearSearch);
    searchLayout->addWidget(clearButton_);
    
    mainLayout->addLayout(searchLayout);
    
    // Options row
    QHBoxLayout* optionsLayout = new QHBoxLayout();
    optionsLayout->setSpacing(12);
    
    caseSensitiveCheck_ = new QCheckBox(tr("Case sensitive"));
    connect(caseSensitiveCheck_, &QCheckBox::toggled, this, &SearchWidget::updateResults);
    optionsLayout->addWidget(caseSensitiveCheck_);
    
    regexCheck_ = new QCheckBox(tr("Regex"));
    regexCheck_->setToolTip(tr("Use regular expressions"));
    connect(regexCheck_, &QCheckBox::toggled, this, &SearchWidget::updateResults);
    optionsLayout->addWidget(regexCheck_);
    
    optionsLayout->addSpacing(20);
    
    QLabel* filterLabel = new QLabel(tr("Type:"));
    optionsLayout->addWidget(filterLabel);
    
    filterTypeCombo_ = new QComboBox();
    filterTypeCombo_->addItem(tr("All Types"), -1);
    filterTypeCombo_->addItem(tr("Mouse Move"), static_cast<int>(EventType::MouseMove));
    filterTypeCombo_->addItem(tr("Mouse Down"), static_cast<int>(EventType::MouseButtonDown));
    filterTypeCombo_->addItem(tr("Mouse Up"), static_cast<int>(EventType::MouseButtonUp));
    filterTypeCombo_->addItem(tr("Mouse Wheel"), static_cast<int>(EventType::MouseWheel));
    filterTypeCombo_->addItem(tr("Key Down"), static_cast<int>(EventType::KeyDown));
    filterTypeCombo_->addItem(tr("Key Up"), static_cast<int>(EventType::KeyUp));
    filterTypeCombo_->addItem(tr("Wait"), static_cast<int>(EventType::Wait));
    filterTypeCombo_->addItem(tr("Teleport"), static_cast<int>(EventType::MouseTeleport));
    connect(filterTypeCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &SearchWidget::updateResults);
    optionsLayout->addWidget(filterTypeCombo_);
    
    optionsLayout->addStretch();
    
    resultsLabel_ =
		new QLabel(
			tr(""));

	DarkStyle::setTone(
		resultsLabel_,
		"secondary");

	optionsLayout->addWidget(
		resultsLabel_);
    
    mainLayout->addLayout(optionsLayout);
    
    setMaximumHeight(80);
}

void SearchWidget::setRecording(Recording* recording)
{
    recording_ = recording;
    updateResults();
}

void SearchWidget::focusSearch()
{
    searchEdit_->setFocus();
    searchEdit_->selectAll();
}

QList<int> SearchWidget::findMatches() const
{
    return matchingIndices_;
}

void SearchWidget::nextResult()
{
    if (matchingIndices_.isEmpty()) return;
    
    currentResultIndex_++;
    if (currentResultIndex_ >= matchingIndices_.size()) {
        currentResultIndex_ = 0;
    }
    
    resultsLabel_->setText(tr("%1 of %2").arg(currentResultIndex_ + 1).arg(matchingIndices_.size()));
    emit navigateToResult(matchingIndices_[currentResultIndex_]);
}

void SearchWidget::previousResult()
{
    if (matchingIndices_.isEmpty()) return;
    
    currentResultIndex_--;
    if (currentResultIndex_ < 0) {
        currentResultIndex_ = matchingIndices_.size() - 1;
    }
    
    resultsLabel_->setText(tr("%1 of %2").arg(currentResultIndex_ + 1).arg(matchingIndices_.size()));
    emit navigateToResult(matchingIndices_[currentResultIndex_]);
}

void SearchWidget::clearSearch()
{
    searchEdit_->clear();
    filterTypeCombo_->setCurrentIndex(0);
    matchingIndices_.clear();
    currentResultIndex_ = -1;
    resultsLabel_->clear();
    emit highlightResults(QList<int>());
}

void SearchWidget::onSearchTextChanged(const QString& text)
{
    Q_UNUSED(text);
    updateResults();
}

void SearchWidget::updateResults()
{
    matchingIndices_.clear();
    currentResultIndex_ = -1;
    
    if (!recording_ || recording_->empty()) {
        resultsLabel_->setText(tr("No events"));
        prevButton_->setEnabled(false);
        nextButton_->setEnabled(false);
        emit highlightResults(QList<int>());
        return;
    }
    
    const auto& events = recording_->events();
    
    for (size_t i = 0; i < events.size(); ++i) {
        if (eventMatchesSearch(events[i], static_cast<int>(i))) {
            matchingIndices_.append(static_cast<int>(i));
        }
    }
    
    bool hasResults = !matchingIndices_.isEmpty();
    prevButton_->setEnabled(hasResults);
    nextButton_->setEnabled(hasResults);
    
    QString searchText = searchEdit_->text().trimmed();
    if (searchText.isEmpty()
		&& filterTypeCombo_->currentIndex() == 0)
	{
		resultsLabel_->setText(
			tr("%1 events")
				.arg(
					events.size()));

		DarkStyle::setTone(
			resultsLabel_,
			"secondary");
	}
	else if (hasResults)
	{
		resultsLabel_->setText(
			tr("%1 matches")
				.arg(
					matchingIndices_.size()));

		DarkStyle::setTone(
			resultsLabel_,
			"accent");
	}
	else
	{
		resultsLabel_->setText(
			tr("No matches"));

		DarkStyle::setTone(
			resultsLabel_,
			"accentDark");
	}
    
    emit searchChanged();
    emit highlightResults(matchingIndices_);
}

bool SearchWidget::eventMatchesSearch(const InputEvent& event, int index) const
{
    // Check type filter
    int typeFilter = filterTypeCombo_->currentData().toInt();
    if (typeFilter != -1) {
        if (static_cast<int>(event.type) != typeFilter) {
            // Special case: Mouse Down and Mouse Up are both "clicks"
            if (typeFilter == static_cast<int>(EventType::MouseButtonDown)) {
                if (event.type != EventType::MouseButtonDown && 
                    event.type != EventType::MouseButtonUp) {
                    return false;
                }
            } else if (typeFilter == static_cast<int>(EventType::KeyDown)) {
                if (event.type != EventType::KeyDown && 
                    event.type != EventType::KeyUp) {
                    return false;
                }
            } else {
                return false;
            }
        }
    }
    
    // Check text search
    QString searchText = searchEdit_->text().trimmed();
    if (searchText.isEmpty()) {
        return true; // Type filter only
    }
    
    QString eventString = eventToSearchString(event);
    eventString += QString(" #%1").arg(index + 1); // Include index in search
    
    if (regexCheck_->isChecked()) {
        QRegularExpression::PatternOptions options = QRegularExpression::NoPatternOption;
        if (!caseSensitiveCheck_->isChecked()) {
            options |= QRegularExpression::CaseInsensitiveOption;
        }
        QRegularExpression regex(searchText, options);
        return regex.match(eventString).hasMatch();
    } else {
        Qt::CaseSensitivity cs = caseSensitiveCheck_->isChecked() 
            ? Qt::CaseSensitive 
            : Qt::CaseInsensitive;
        return eventString.contains(searchText, cs);
    }
}

QString SearchWidget::eventToSearchString(const InputEvent& event) const
{
    QString result;
    
    // Type
    switch (event.type) {
        case EventType::MouseMove:
            result = QString("MouseMove Move delta %1,%2 position %3,%4")
                .arg(event.mouseDeltaX).arg(event.mouseDeltaY)
                .arg(event.mouseX).arg(event.mouseY);
            break;
        case EventType::MouseButtonDown:
            result = QString("MouseDown Click button %1 position %2,%3")
                .arg(event.mouseButton).arg(event.mouseX).arg(event.mouseY);
            if (event.mouseButton == 1) result += " left";
            else if (event.mouseButton == 2) result += " right";
            else if (event.mouseButton == 3) result += " middle";
            break;
        case EventType::MouseButtonUp:
            result = QString("MouseUp Release button %1 position %2,%3")
                .arg(event.mouseButton).arg(event.mouseX).arg(event.mouseY);
            if (event.mouseButton == 1) result += " left";
            else if (event.mouseButton == 2) result += " right";
            else if (event.mouseButton == 3) result += " middle";
            break;
        case EventType::MouseWheel:
            result = QString("MouseWheel Scroll delta %1 position %2,%3")
                .arg(event.mouseWheelDelta).arg(event.mouseX).arg(event.mouseY);
            break;
        case EventType::KeyDown:
            result = QString("KeyDown Press key 0x%1 %2")
                .arg(event.keyCode, 2, 16, QChar('0'))
                .arg(getKeyName(event.keyCode));
            break;
        case EventType::KeyUp:
            result = QString("KeyUp Release key 0x%1 %2")
                .arg(event.keyCode, 2, 16, QChar('0'))
                .arg(getKeyName(event.keyCode));
            break;
        case EventType::Wait:
            result = QString("Wait delay %1 microseconds %2 milliseconds")
                .arg(event.waitMicroseconds)
                .arg(event.waitMicroseconds / 1000);
            break;
        case EventType::MouseTeleport:
            result = QString("Teleport Jump position %1,%2")
                .arg(event.mouseX).arg(event.mouseY);
            break;
        default:
            result = "Unknown";
    }
    
    // Add timestamp
    result += QString(" time %1").arg(event.timestampMicroseconds);
    
    return result;
}

QString SearchWidget::getKeyName(int keyCode) const
{
    switch (keyCode) {
        case 0x08: return "Backspace";
        case 0x09: return "Tab";
        case 0x0D: return "Enter Return";
        case 0x10: return "Shift";
        case 0x11: return "Ctrl Control";
        case 0x12: return "Alt";
        case 0x1B: return "Escape Esc";
        case 0x20: return "Space";
        case 0x21: return "PageUp";
        case 0x22: return "PageDown";
        case 0x23: return "End";
        case 0x24: return "Home";
        case 0x25: return "Left Arrow";
        case 0x26: return "Up Arrow";
        case 0x27: return "Right Arrow";
        case 0x28: return "Down Arrow";
        case 0x2D: return "Insert";
        case 0x2E: return "Delete";
        default:
            if (keyCode >= 0x30 && keyCode <= 0x39) {
                return QString("%1").arg(QChar('0' + (keyCode - 0x30)));
            } else if (keyCode >= 0x41 && keyCode <= 0x5A) {
                return QString("%1").arg(QChar('A' + (keyCode - 0x41)));
            } else if (keyCode >= 0x70 && keyCode <= 0x87) {
                return QString("F%1").arg(keyCode - 0x70 + 1);
            }
            return QString();
    }
}
