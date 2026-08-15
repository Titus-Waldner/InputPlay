#pragma once

#include <QApplication>
#include <QColor>
#include <QPalette>
#include <QString>

class QWidget;

enum class ThemeType
{
    DarkBlue,
    DarkGray,
    Light
};

enum class AccentColor
{
    Cyan,
    Magenta,
    Green,
    Purple,
    Yellow,
    Red,
    Orange
};

class DarkStyle
{
public:
    // Apply the default appearance.
    static void apply(
        QApplication* app);

    // Apply a specific theme and accent.
    static void apply(
        QApplication* app,
        ThemeType theme,
        AccentColor accent);

    static ThemeType currentTheme()
    {
        return currentTheme_;
    }

    static AccentColor currentAccent()
    {
        return currentAccent_;
    }

    // Selected application accent.
    static QString accentColorString();

    static QString accentColorString(
        AccentColor accent);

    static QString accentColorLighter();
    static QString accentColorDarker();

    // Apply a template or selected-accent tone to a widget.
    //
    // Approved values:
    //   accent
    //   accentLight
    //   accentDark
    //   primary
    //   secondary
    //   disabled
    static void setTone(
        QWidget* widget,
        const char* tone);

    static QColor toneColor(
        const char* tone);

    // Apply a centralized event-category color to a widget.
    //
    // Approved values:
    //   mouseMove
    //   mouseClick
    //   mouseWheel
    //   keyboard
    //   wait
    //   teleport
    static void setEventTone(
        QWidget* widget,
        const char* eventTone);

    static QColor eventColor(
        const char* eventTone);

    // Active template colors.
    static QString bgPrimary()
    {
        return bgPrimary_;
    }

    static QString bgSecondary()
    {
        return bgSecondary_;
    }

    static QString bgTertiary()
    {
        return bgTertiary_;
    }

    static QString bgHover()
    {
        return bgHover_;
    }

    static QString bgSelected()
    {
        return bgSelected_;
    }

    static QString textPrimary()
    {
        return textPrimary_;
    }

    static QString textSecondary()
    {
        return textSecondary_;
    }

    static QString textDisabled()
    {
        return textDisabled_;
    }

    static QString border()
    {
        return border_;
    }

    // Generate the Qt application palette and stylesheet.
    static QString getStyleSheet();

    static QString getStyleSheet(
        ThemeType theme,
        AccentColor accent);

    static QPalette getPalette();

    static QPalette getPalette(
        ThemeType theme,
        AccentColor accent);

private:
    static void updateThemeColors(
        ThemeType theme);

    static void updateAccentColor(
        AccentColor accent);

    static ThemeType currentTheme_;
    static AccentColor currentAccent_;

    // Active template colors.
    static QString bgPrimary_;
    static QString bgSecondary_;
    static QString bgTertiary_;
    static QString bgHover_;
    static QString bgSelected_;

    static QString textPrimary_;
    static QString textSecondary_;
    static QString textDisabled_;

    static QString border_;

    // Active selected accent.
    static QString accent_;
    static QString accentLight_;
    static QString accentDark_;

    // Central event-category palette.
    static QString eventMouseMove_;
    static QString eventMouseClick_;
    static QString eventMouseWheel_;
    static QString eventKeyboard_;
    static QString eventWait_;
    static QString eventTeleport_;
};