#include "DarkStyle.h"

#include <QColor>
#include <QStyle>
#include <QStyleFactory>
#include <QWidget>

ThemeType DarkStyle::currentTheme_ = ThemeType::DarkBlue;
AccentColor DarkStyle::currentAccent_ = AccentColor::Cyan;

QString DarkStyle::bgPrimary_ = "#1a1a2e";
QString DarkStyle::bgSecondary_ = "#16213e";
QString DarkStyle::bgTertiary_ = "#0f3460";
QString DarkStyle::bgHover_ = "#3a5f9a";
QString DarkStyle::bgSelected_ = "#4a7fc0";
QString DarkStyle::textPrimary_ = "#ffffff";
QString DarkStyle::textSecondary_ = "#ffffff";
QString DarkStyle::textDisabled_ = "#ffffff";
QString DarkStyle::border_ = "#2a2a4a";
QString DarkStyle::accent_ = "#00d9ff";
QString DarkStyle::accentLight_ = "#33e5ff";
QString DarkStyle::accentDark_ = "#00b8d4";

// Central event-category palette.
//
// These colors identify event types and remain independent of the
// user-selected application accent.
QString DarkStyle::eventMouseMove_ =
    "#00d9ff";

QString DarkStyle::eventMouseClick_ =
    "#ff006e";

QString DarkStyle::eventMouseWheel_ =
    "#aa66ff";

QString DarkStyle::eventKeyboard_ =
    "#00ff88";

QString DarkStyle::eventWait_ =
    "#6f86b3";

QString DarkStyle::eventTeleport_ =
    "#ffaa00";

void DarkStyle::apply(QApplication* app)
{
    apply(app, ThemeType::DarkBlue, AccentColor::Cyan);
}

void DarkStyle::apply(
    QApplication* app,
    ThemeType theme,
    AccentColor accent)
{
    currentTheme_ = theme;
    currentAccent_ = accent;

    updateThemeColors(theme);
    updateAccentColor(accent);

    app->setStyle(QStyleFactory::create("Fusion"));
    app->setPalette(getPalette(theme, accent));
    app->setStyleSheet(getStyleSheet(theme, accent));
}

void DarkStyle::updateThemeColors(ThemeType theme)
{
    switch (theme)
    {
        case ThemeType::DarkBlue:
            bgPrimary_ = "#1a1a2e";
            bgSecondary_ = "#16213e";
            bgTertiary_ = "#0f3460";
            bgHover_ = "#3a5f9a";
            bgSelected_ = "#4a7fc0";
            textPrimary_ = "#ffffff";
            textSecondary_ = "#ffffff";
            textDisabled_ = "#ffffff";
            border_ = "#2a2a4a";
            break;

        case ThemeType::DarkGray:
            bgPrimary_ = "#1e1e1e";
            bgSecondary_ = "#252526";
            bgTertiary_ = "#2d2d30";
            bgHover_ = "#3e3e42";
            bgSelected_ = "#505056";
            textPrimary_ = "#d4d4d4";
            textSecondary_ = "#9a9a9a";
            textDisabled_ = "#7f7f7f";
            border_ = "#3c3c3c";
            break;

        case ThemeType::Light:
            bgPrimary_ = "#f3f3f3";
            bgSecondary_ = "#ffffff";
            bgTertiary_ = "#e8e8e8";
            bgHover_ = "#e0e0e0";
            bgSelected_ = "#d0d0d0";
            textPrimary_ = "#1e1e1e";
            textSecondary_ = "#6e6e6e";
            textDisabled_ = "#8a8a8a";
            border_ = "#d0d0d0";
            break;
    }
}

void DarkStyle::updateAccentColor(AccentColor accent)
{
    QColor color;

    switch (accent)
    {
        case AccentColor::Cyan:
            color = QColor("#00d9ff");
            break;
        case AccentColor::Magenta:
            color = QColor("#ff006e");
            break;
        case AccentColor::Green:
            color = QColor("#00ff88");
            break;
        case AccentColor::Purple:
            color = QColor("#aa66ff");
            break;
        case AccentColor::Yellow:
            color = QColor("#ffaa00");
            break;
        case AccentColor::Red:
            color = QColor("#ff3366");
            break;
        case AccentColor::Orange:
            color = QColor("#ff6b35");
            break;
    }

    accent_ = color.name();
    accentLight_ = color.lighter(120).name();
    accentDark_ = color.darker(145).name();
}

QString DarkStyle::accentColorString()
{
    return accent_;
}

QString DarkStyle::accentColorString(AccentColor accent)
{
    switch (accent)
    {
        case AccentColor::Cyan: return "#00d9ff";
        case AccentColor::Magenta: return "#ff006e";
        case AccentColor::Green: return "#00ff88";
        case AccentColor::Purple: return "#aa66ff";
        case AccentColor::Yellow: return "#ffaa00";
        case AccentColor::Red: return "#ff3366";
        case AccentColor::Orange: return "#ff6b35";
    }

    return "#00d9ff";
}

QString DarkStyle::accentColorLighter()
{
    return accentLight_;
}

QString DarkStyle::accentColorDarker()
{
    return accentDark_;
}

void DarkStyle::setTone(
    QWidget* widget,
    const char* tone)
{
    if (!widget)
    {
        return;
    }

    widget->setProperty(
        "tone",
        tone);

    // A dynamic Qt property does not always trigger an immediate
    // stylesheet recalculation. Repolishing applies the new tone.
    widget->style()->unpolish(
        widget);

    widget->style()->polish(
        widget);

    widget->update();
}

QColor DarkStyle::toneColor(
    const char* tone)
{
    const QString toneName =
        QString::fromLatin1(
            tone ? tone : "");

    if (toneName == "accent")
    {
        return QColor(
            accent_);
    }

    if (toneName == "accentLight")
    {
        return QColor(
            accentLight_);
    }

    if (toneName == "accentDark")
    {
        return QColor(
            accentDark_);
    }

    if (toneName == "secondary")
    {
        return QColor(
            textSecondary_);
    }

    if (toneName == "disabled")
    {
        return QColor(
            textDisabled_);
    }

    // Primary is also the safe fallback for unknown tones.
    return QColor(
        textPrimary_);
}

void DarkStyle::setEventTone(
    QWidget* widget,
    const char* eventTone)
{
    if (!widget)
    {
        return;
    }

    widget->setProperty(
        "eventTone",
        eventTone);

    // Dynamic properties require repolishing before the new
    // stylesheet selector is applied.
    widget->style()->unpolish(
        widget);

    widget->style()->polish(
        widget);

    widget->update();
}

QColor DarkStyle::eventColor(
    const char* eventTone)
{
    const QString eventToneName =
        QString::fromLatin1(
            eventTone
            ? eventTone
            : "");

    if (eventToneName == "mouseMove")
    {
        return QColor(
            eventMouseMove_);
    }

    if (eventToneName == "mouseClick")
    {
        return QColor(
            eventMouseClick_);
    }

    if (eventToneName == "mouseWheel")
    {
        return QColor(
            eventMouseWheel_);
    }

    if (eventToneName == "keyboard")
    {
        return QColor(
            eventKeyboard_);
    }

    if (eventToneName == "wait")
    {
        return QColor(
            eventWait_);
    }

    if (eventToneName == "teleport")
    {
        return QColor(
            eventTeleport_);
    }

    // Primary theme text is the safe fallback.
    return QColor(
        textPrimary_);
}

QPalette DarkStyle::getPalette()
{
    return getPalette(currentTheme_, currentAccent_);
}

QPalette DarkStyle::getPalette(
    ThemeType theme,
    AccentColor accent)
{
    updateThemeColors(theme);
    updateAccentColor(accent);

    QPalette palette;

    const QColor primaryText(
        textPrimary_);

    const QColor secondaryText(
        textSecondary_);

    const QColor disabledText(
        textDisabled_);

    const QColor primaryBackground(
        bgPrimary_);

    const QColor secondaryBackground(
        bgSecondary_);

    const QColor tertiaryBackground(
        bgTertiary_);

    const QColor hoverBackground(
        bgHover_);

    const QColor borderColor(
        border_);

    const QColor accentColor(
        accent_);

    const QColor accentDarkColor(
        accentDark_);

    // Active colors.
    palette.setColor(
        QPalette::Active,
        QPalette::Window,
        primaryBackground);

    palette.setColor(
        QPalette::Active,
        QPalette::WindowText,
        primaryText);

    palette.setColor(
        QPalette::Active,
        QPalette::Base,
        secondaryBackground);

    palette.setColor(
        QPalette::Active,
        QPalette::AlternateBase,
        tertiaryBackground);

    palette.setColor(
        QPalette::Active,
        QPalette::Text,
        primaryText);

    palette.setColor(
        QPalette::Active,
        QPalette::Button,
        secondaryBackground);

    palette.setColor(
        QPalette::Active,
        QPalette::ButtonText,
        primaryText);

    palette.setColor(
        QPalette::Active,
        QPalette::Highlight,
        accentDarkColor);

    palette.setColor(
        QPalette::Active,
        QPalette::HighlightedText,
        primaryText);

    palette.setColor(
        QPalette::Active,
        QPalette::PlaceholderText,
        secondaryText);

    // Inactive windows should remain readable.
    palette.setColor(
        QPalette::Inactive,
        QPalette::Window,
        primaryBackground);

    palette.setColor(
        QPalette::Inactive,
        QPalette::WindowText,
        primaryText);

    palette.setColor(
        QPalette::Inactive,
        QPalette::Base,
        secondaryBackground);

    palette.setColor(
        QPalette::Inactive,
        QPalette::AlternateBase,
        tertiaryBackground);

    palette.setColor(
        QPalette::Inactive,
        QPalette::Text,
        primaryText);

    palette.setColor(
        QPalette::Inactive,
        QPalette::Button,
        secondaryBackground);

    palette.setColor(
        QPalette::Inactive,
        QPalette::ButtonText,
        primaryText);

    palette.setColor(
        QPalette::Inactive,
        QPalette::Highlight,
        accentDarkColor);

    palette.setColor(
        QPalette::Inactive,
        QPalette::HighlightedText,
        primaryText);

    palette.setColor(
        QPalette::Inactive,
        QPalette::PlaceholderText,
        secondaryText);

    // Disabled colors must come from the selected template,
    // never from the selected accent.
    palette.setColor(
        QPalette::Disabled,
        QPalette::Window,
        primaryBackground);

    palette.setColor(
        QPalette::Disabled,
        QPalette::WindowText,
        disabledText);

    palette.setColor(
        QPalette::Disabled,
        QPalette::Base,
        secondaryBackground);

    palette.setColor(
        QPalette::Disabled,
        QPalette::AlternateBase,
        tertiaryBackground);

    palette.setColor(
        QPalette::Disabled,
        QPalette::Text,
        disabledText);

    palette.setColor(
        QPalette::Disabled,
        QPalette::Button,
        secondaryBackground);

    palette.setColor(
        QPalette::Disabled,
        QPalette::ButtonText,
        disabledText);

    palette.setColor(
        QPalette::Disabled,
        QPalette::Highlight,
        hoverBackground);

    palette.setColor(
        QPalette::Disabled,
        QPalette::HighlightedText,
        disabledText);

    palette.setColor(
        QPalette::Disabled,
        QPalette::PlaceholderText,
        disabledText);

    // Shared colors.
    palette.setColor(
        QPalette::ToolTipBase,
        tertiaryBackground);

    palette.setColor(
        QPalette::ToolTipText,
        primaryText);

    palette.setColor(
        QPalette::BrightText,
        primaryText);

    palette.setColor(
        QPalette::Link,
        accentColor);

    /*
     * Fusion uses these roles for bevels, etched text and 3D effects.
     * They must use template colors rather than accent colors, or
     * disabled text can inherit an unwanted blue/accent appearance.
     */
    palette.setColor(
        QPalette::Light,
        hoverBackground);

    palette.setColor(
        QPalette::Midlight,
        tertiaryBackground);

    palette.setColor(
        QPalette::Mid,
        borderColor);

    palette.setColor(
        QPalette::Dark,
        tertiaryBackground.darker(125));

    palette.setColor(
        QPalette::Shadow,
        primaryBackground.darker(160));

    return palette;
}

QString DarkStyle::getStyleSheet()
{
    return getStyleSheet(currentTheme_, currentAccent_);
}

QString DarkStyle::getStyleSheet(
    ThemeType theme,
    AccentColor accent)
{
    updateThemeColors(theme);
    updateAccentColor(accent);

    QString styleSheet = QStringLiteral(R"(
	
QWidget {
    color: %7;
}

QWidget:disabled {
    color: __TEXT_DISABLED__;
}

QMainWindow { background-color: %1; }

QMenuBar {
    background-color: %2;
    color: %7;
    border-bottom: 1px solid %9;
    padding: 2px;
}
QMenuBar::item { background-color: transparent; padding: 6px 12px; border-radius: 4px; }
QMenuBar::item:selected { background-color: __ACCENT_DARK__; color: %7; }
QMenuBar::item:pressed { background-color: __ACCENT_DARK__; color: %7; }

QMenu {
    background-color: %2;
    color: %7;
    border: 1px solid %9;
    border-radius: 4px;
    padding: 4px;
}
QMenu::item { padding: 8px 24px 8px 12px; border-radius: 4px; }
QMenu::item:selected { background-color: __ACCENT_DARK__; color: %7; }
QMenu::separator { height: 1px; background-color: %9; margin: 4px 8px; }

QToolBar {
    background-color: %2;
    border: none;
    border-bottom: 1px solid %9;
    padding: 4px;
    spacing: 4px;
}
QToolBar::separator { width: 1px; background-color: %9; margin: 4px 8px; }
QToolButton { background-color: transparent; color: %7; border: none; border-radius: 4px; padding: 6px 12px; }
QToolButton:hover, QToolButton:pressed { background-color: __ACCENT_DARK__; color: %7; }
QToolButton:checked { background-color: %3; border: 1px solid __ACCENT_COLOR__; }

QPushButton {
    background-color: %3;
    color: %7;
    border: 1px solid %9;
    border-radius: 6px;
    padding: 8px 16px;
    min-width: 80px;
}
QPushButton[compact="true"] {
    padding: 2px 8px;
    min-width: 0;
}
QPushButton:hover { background-color: __ACCENT_DARK__; color: %7; border-color: __ACCENT_COLOR__; }
QPushButton:pressed { background-color: __ACCENT_DARK__; color: %7; }
QPushButton:disabled { background-color: %2; color: __TEXT_DISABLED__; border-color: %9; }
QPushButton[primary="true"] { background-color: __ACCENT_COLOR__; color: #ffffff; border: none; font-weight: bold; }
QPushButton[primary="true"]:hover { background-color: __ACCENT_LIGHT__; color: #ffffff; }
QPushButton[primary="true"]:pressed { background-color: __ACCENT_DARK__; color: #ffffff; }
QPushButton[danger="true"] { background-color: #ff3366; color: #ffffff; border: none; }
QPushButton[danger="true"]:hover { background-color: #ff5588; color: #ffffff; }

QLineEdit {
    background-color: %2;
    color: %7;
    border: 1px solid %9;
    border-radius: 4px;
    padding: 8px;
    selection-background-color: __ACCENT_DARK__;
    selection-color: %7;
}
QLineEdit:focus { border-color: __ACCENT_COLOR__; }
QLineEdit:disabled { background-color: %1; color: __TEXT_DISABLED__; }

QSpinBox, QDoubleSpinBox {
    background-color: %2;
    color: %7;
    border: 1px solid %9;
    border-radius: 4px;
    padding: 6px;
}
QSpinBox:focus, QDoubleSpinBox:focus { border-color: __ACCENT_COLOR__; }
QSpinBox::up-button, QDoubleSpinBox::up-button,
QSpinBox::down-button, QDoubleSpinBox::down-button {
    background-color: %3;
    border: none;
    width: 20px;
}
QSpinBox::up-button:hover, QDoubleSpinBox::up-button:hover,
QSpinBox::down-button:hover, QDoubleSpinBox::down-button:hover {
    background-color: __ACCENT_DARK__;
}

QComboBox {
    background-color: %2;
    color: %7;
    border: 1px solid %9;
    border-radius: 4px;
    padding: 8px;
    min-width: 100px;
}
QComboBox:hover, QComboBox:focus { border-color: __ACCENT_COLOR__; }
QComboBox::drop-down { border: none; width: 24px; }
QComboBox::down-arrow {
    image: none;
    border-left: 5px solid transparent;
    border-right: 5px solid transparent;
    border-top: 6px solid __ACCENT_COLOR__;
    margin-right: 8px;
}

/* All combo-box popup rows use light/readable text. */
QComboBox QAbstractItemView,
QAbstractItemView[comboPopup="true"] {
    background-color: %2;
    color: %7;
    border: 1px solid %9;
    border-radius: 4px;
    outline: none;
    padding: 2px;
    selection-background-color: __ACCENT_DARK__;
    selection-color: %7;
}
QComboBox QAbstractItemView::item,
QAbstractItemView[comboPopup="true"]::item,
QListView[comboPopup="true"]::item {
    background-color: transparent;
    color: %7;
    min-height: 24px;
    padding: 4px 8px;
    border: none;
    border-radius: 3px;
}
QComboBox QAbstractItemView::item:hover,
QComboBox QAbstractItemView::item:selected,
QComboBox QAbstractItemView::item:selected:hover,
QAbstractItemView[comboPopup="true"]::item:hover,
QAbstractItemView[comboPopup="true"]::item:selected,
QAbstractItemView[comboPopup="true"]::item:selected:hover,
QListView[comboPopup="true"]::item:hover,
QListView[comboPopup="true"]::item:selected,
QListView[comboPopup="true"]::item:selected:hover {
    background-color: __ACCENT_DARK__;
    color: %7;
}
QAbstractItemView[comboPopup="true"]::item:disabled,
QListView[comboPopup="true"]::item:disabled {
    background-color: transparent;
    color: __TEXT_DISABLED__;
}

QCheckBox { color: %7; spacing: 8px; }
QCheckBox::indicator { width: 18px; height: 18px; border: 2px solid %9; border-radius: 4px; background-color: %2; }
QCheckBox::indicator:hover { border-color: __ACCENT_COLOR__; }
QCheckBox::indicator:checked { background-color: __ACCENT_COLOR__; border-color: __ACCENT_COLOR__; }

QRadioButton { color: %7; spacing: 8px; }
QRadioButton::indicator { width: 18px; height: 18px; border: 2px solid %9; border-radius: 10px; background-color: %2; }
QRadioButton::indicator:hover { border-color: __ACCENT_COLOR__; }
QRadioButton::indicator:checked { background-color: __ACCENT_COLOR__; border-color: __ACCENT_COLOR__; }

QGroupBox {
    color: __ACCENT_COLOR__;
    border: 1px solid %9;
    border-radius: 6px;
    margin-top: 8px;
    padding-top: 20px;
    font-weight: normal;
}
QGroupBox::title {
    subcontrol-origin: margin;
    subcontrol-position: top left;
    top: 0px;
    left: 10px;
    padding: 0 6px;
    color: __ACCENT_COLOR__;
    font-weight: bold;
    background-color: %1;
}
QGroupBox:disabled, QGroupBox:disabled::title { color: __ACCENT_COLOR__; }
QGroupBox[tone="accent"]::title {
    color: __ACCENT_COLOR__;
}

QGroupBox[tone="accentLight"]::title {
    color: __ACCENT_LIGHT__;
}

QGroupBox[tone="accentDark"]::title {
    color: __ACCENT_DARK__;
}

QGroupBox[tone="primary"]::title {
    color: %7;
}

QGroupBox[tone="secondary"]::title {
    color: %8;
}

QTabWidget::pane { border: 1px solid %9; border-radius: 4px; background-color: %1; }
QTabBar::tab {
    background-color: %2;
    color: %8;
    border: 1px solid %9;
    border-bottom: none;
    border-top-left-radius: 4px;
    border-top-right-radius: 4px;
    padding: 8px 16px;
    margin-right: 2px;
}
QTabBar::tab:selected { background-color: %1; color: __ACCENT_COLOR__; border-bottom: 2px solid __ACCENT_COLOR__; }
QTabBar::tab:hover:!selected { background-color: __ACCENT_DARK__; color: %7; }

QScrollBar:vertical { background-color: %1; width: 12px; border-radius: 6px; margin: 0; }
QScrollBar::handle:vertical { background-color: %9; border-radius: 6px; min-height: 30px; margin: 2px; }
QScrollBar::handle:vertical:hover { background-color: __ACCENT_DARK__; }
QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }
QScrollBar:horizontal { background-color: %1; height: 12px; border-radius: 6px; margin: 0; }
QScrollBar::handle:horizontal { background-color: %9; border-radius: 6px; min-width: 30px; margin: 2px; }
QScrollBar::handle:horizontal:hover { background-color: __ACCENT_DARK__; }
QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal { width: 0; }

QListView, QTreeView, QTableView {
    background-color: %2;
    color: %7;
    border: 1px solid %9;
    border-radius: 4px;
    outline: none;
}
QListView::item, QTreeView::item, QTableView::item { padding: 6px; border-radius: 2px; }
QListView::item:hover, QTreeView::item:hover, QTableView::item:hover {
    background-color: __ACCENT_DARK__;
    color: %7;
}
QListView::item:selected, QTreeView::item:selected, QTableView::item:selected {
    background-color: __ACCENT_DARK__;
    color: %7;
}

QHeaderView::section {
    background-color: %3;
    color: %7;
    border: none;
    border-right: 1px solid %9;
    border-bottom: 1px solid %9;
    padding: 8px;
    font-weight: bold;
}
QHeaderView::section:hover { background-color: __ACCENT_DARK__; color: %7; }

QSlider::groove:horizontal { background-color: %9; height: 6px; border-radius: 3px; }
QSlider::handle:horizontal { background-color: __ACCENT_COLOR__; width: 16px; height: 16px; margin: -5px 0; border-radius: 8px; }
QSlider::handle:horizontal:hover { background-color: __ACCENT_LIGHT__; }
QSlider::sub-page:horizontal { background-color: __ACCENT_COLOR__; border-radius: 3px; }

QProgressBar { background-color: %9; color: %7; border: none; border-radius: 4px; text-align: center; height: 20px; }
QProgressBar::chunk { background-color: __ACCENT_COLOR__; border-radius: 4px; }

QSplitter::handle { background-color: %9; }
QSplitter::handle:horizontal { width: 2px; }
QSplitter::handle:vertical { height: 2px; }
QSplitter::handle:hover { background-color: __ACCENT_COLOR__; }

QStatusBar { background-color: %2; color: %8; border-top: 1px solid %9; }
QStatusBar::item { border: none; }

QToolTip { background-color: %3; color: %7; border: 1px solid __ACCENT_COLOR__; border-radius: 4px; padding: 6px; }

QDockWidget { color: %7; titlebar-close-icon: none; titlebar-normal-icon: none; }
QDockWidget::title { background-color: %3; color: __ACCENT_COLOR__; padding: 8px; border-bottom: 1px solid %9; font-weight: bold; }
QDockWidget::close-button, QDockWidget::float-button { background-color: transparent; border: none; }
QDockWidget::close-button:hover, QDockWidget::float-button:hover { background-color: __ACCENT_DARK__; }

QLabel {
    color: %7;
}

QLabel[heading="true"] {
    font-size: 14px;
    font-weight: bold;
    color: __ACCENT_COLOR__;
}

QLabel[subheading="true"] {
    font-size: 12px;
    color: %8;
}

/* Centralized template and accent tones */
QWidget[tone="accent"] {
    color: __ACCENT_COLOR__;
}

QWidget[tone="accentLight"] {
    color: __ACCENT_LIGHT__;
}

QWidget[tone="accentDark"] {
    color: __ACCENT_DARK__;
}

QWidget[tone="primary"] {
    color: %7;
}

QWidget[tone="secondary"] {
    color: %8;
}

QWidget[tone="disabled"] {
    color: __TEXT_DISABLED__;
}

/* Central event-category colors */
QWidget[eventTone="mouseMove"] {
    color: __EVENT_MOUSE_MOVE__;
}

QWidget[eventTone="mouseClick"] {
    color: __EVENT_MOUSE_CLICK__;
}

QWidget[eventTone="mouseWheel"] {
    color: __EVENT_MOUSE_WHEEL__;
}

QWidget[eventTone="keyboard"] {
    color: __EVENT_KEYBOARD__;
}

QWidget[eventTone="wait"] {
    color: __EVENT_WAIT__;
}

QWidget[eventTone="teleport"] {
    color: __EVENT_TELEPORT__;
}

/* Status headings retain their typography independently of color. */
QLabel[statusHeading="true"] {
    font-size: 14px;
    font-weight: bold;
}


QLabel[recordingIndicator="true"] {
    font-size: 24px;
}

QLabel[bold="true"] {
    font-weight: bold;
}

QLabel[subheading="true"][italic="true"] {
    font-style: italic;
}

QFrame[frameShape="4"] { background-color: %9; max-height: 1px; }
QFrame[frameShape="5"] { background-color: %9; max-width: 1px; }

QDialog { background-color: %1; }
)");

    // Replace template tokens explicitly. This avoids QString::arg()
    // renumbering when a placeholder number is intentionally skipped.
    styleSheet.replace(
        "%1",
        bgPrimary_);

    styleSheet.replace(
        "%2",
        bgSecondary_);

    styleSheet.replace(
        "%3",
        bgTertiary_);

    styleSheet.replace(
        "%4",
        bgHover_);

    styleSheet.replace(
        "%5",
        bgSelected_);

    styleSheet.replace(
        "%7",
        textPrimary_);

    styleSheet.replace(
        "%8",
        textSecondary_);

    styleSheet.replace(
        "%9",
        border_);

    styleSheet.replace(
        "__ACCENT_COLOR__",
        accent_);

    styleSheet.replace(
    "__ACCENT_COLOR__",
    accent_);

	styleSheet.replace(
		"__ACCENT_LIGHT__",
		accentLight_);

	styleSheet.replace(
		"__ACCENT_DARK__",
		accentDark_);

	styleSheet.replace(
		"__TEXT_DISABLED__",
		textDisabled_);

	styleSheet.replace(
		"__EVENT_MOUSE_MOVE__",
		eventMouseMove_);

	styleSheet.replace(
		"__EVENT_MOUSE_CLICK__",
		eventMouseClick_);

	styleSheet.replace(
		"__EVENT_MOUSE_WHEEL__",
		eventMouseWheel_);

	styleSheet.replace(
		"__EVENT_KEYBOARD__",
		eventKeyboard_);

	styleSheet.replace(
		"__EVENT_WAIT__",
		eventWait_);

	styleSheet.replace(
		"__EVENT_TELEPORT__",
		eventTeleport_);

	return styleSheet;

    return styleSheet;
}
