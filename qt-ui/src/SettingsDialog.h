#pragma once

#include "Settings.h"
#include "DarkStyle.h"

#include <QDialog>

class QTabWidget;
class QSpinBox;
class QComboBox;
class QCheckBox;
class QLineEdit;
class QPushButton;
class QKeySequenceEdit;

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(const Settings& settings, QWidget* parent = nullptr);
    
    Settings settings() const { return settings_; }
    
    // Appearance settings
    ThemeType selectedTheme() const;
    AccentColor selectedAccent() const;

signals:
    void appearanceChanged(ThemeType theme, AccentColor accent);

private slots:
    void accept() override;
    void resetToDefaults();
    void onThemeChanged();
    void onAccentChanged();

private:
	void toggleDefaultLoopsInfinite();
    void setupUi();
    QWidget* createRecordingTab();
    QWidget* createPlaybackTab();
    QWidget* createAppearanceTab();
    QWidget* createGeneralTab();
    
    void loadSettings();
    void saveSettings();
    
    QString keyNameFromCode(int code) const;
    int keyCodeFromName(const QString& name) const;
    
    Settings settings_;
    
    QTabWidget* tabWidget_ = nullptr;
    
    // Recording settings
    QCheckBox* captureMouseCheck_ = nullptr;
    QCheckBox* captureKeyboardCheck_ = nullptr;
    QCheckBox* captureDisplayCheck_ = nullptr;
    
    // Playback settings
    QSpinBox* defaultLoopsSpin_ = nullptr;
	QPushButton* infiniteLoopsButton_ = nullptr;
	bool defaultLoopsInfinite_ = false;
    QCheckBox* dryRunDefaultCheck_ = nullptr;
    QCheckBox* confirmRealPlaybackCheck_ = nullptr;
    
    // Appearance settings
    QComboBox* themeCombo_ = nullptr;
    QComboBox* accentColorCombo_ = nullptr;
    QSpinBox* fontSizeSpin_ = nullptr;
    
    // General settings
    QLineEdit* defaultPathEdit_ = nullptr;
    QPushButton* browseButton_ = nullptr;
    QCheckBox* autoSaveCheck_ = nullptr;
    QSpinBox* autoSaveIntervalSpin_ = nullptr;
    QCheckBox* globalHotkeysCheck_ = nullptr;
    QCheckBox* showTrayIconCheck_ = nullptr;
    QCheckBox* minimizeToTrayCheck_ = nullptr;
    QCheckBox* showNotificationsCheck_ = nullptr;
};
