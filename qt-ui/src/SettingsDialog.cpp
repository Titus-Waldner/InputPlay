#include "SettingsDialog.h"
#include "DarkStyle.h"

#include <QAbstractItemView>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QTabWidget>
#include <QSpinBox>
#include <QComboBox>
#include <QCheckBox>
#include <QLineEdit>
#include <QPushButton>
#include <QGroupBox>
#include <QLabel>
#include <QFileDialog>
#include <QDialogButtonBox>
#include <QPainter>
#include <QPixmap>
#include <QSettings>

SettingsDialog::SettingsDialog(const Settings& settings, QWidget* parent)
    : QDialog(parent)
    , settings_(settings)
{
    setWindowTitle(tr("Settings"));
    setMinimumSize(500, 450);
    resize(550, 500);
    
    setupUi();
    loadSettings();
}

void SettingsDialog::setupUi()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    tabWidget_ = new QTabWidget();
    tabWidget_->addTab(createRecordingTab(), tr("Recording"));
    tabWidget_->addTab(createPlaybackTab(), tr("Playback"));
    tabWidget_->addTab(createAppearanceTab(), tr("Appearance"));
    tabWidget_->addTab(createGeneralTab(), tr("General"));
    
    mainLayout->addWidget(tabWidget_);
    
    // Button box
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    
    QPushButton* resetButton = new QPushButton(tr("Reset to Defaults"));
    connect(resetButton, &QPushButton::clicked, this, &SettingsDialog::resetToDefaults);
    buttonLayout->addWidget(resetButton);
    
    buttonLayout->addStretch();
    
    QPushButton* cancelButton = new QPushButton(tr("Cancel"));
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
    buttonLayout->addWidget(cancelButton);
    
    QPushButton* okButton = new QPushButton(tr("OK"));
    okButton->setProperty("primary", true);
    okButton->setDefault(true);
    connect(okButton, &QPushButton::clicked, this, &QDialog::accept);
    buttonLayout->addWidget(okButton);
    
    mainLayout->addLayout(buttonLayout);
}

QWidget* SettingsDialog::createRecordingTab()
{
    QWidget* tab = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(tab);
    
    // Active-workspace hotkeys.
	QGroupBox* hotkeyGroup =
		new QGroupBox(
			tr("Active Workspace Hotkeys"));

	QFormLayout* hotkeyLayout =
		new QFormLayout(
			hotkeyGroup);

	QLabel* startStopHotkeyLabel =
		new QLabel(
			tr("F9"),
			hotkeyGroup);

	startStopHotkeyLabel->setProperty(
		"bold",
		true);

	hotkeyLayout->addRow(
		tr("Start / Stop:"),
		startStopHotkeyLabel);

	QLabel* pauseResumeHotkeyLabel =
		new QLabel(
			tr("F10"),
			hotkeyGroup);

	pauseResumeHotkeyLabel->setProperty(
		"bold",
		true);

	hotkeyLayout->addRow(
		tr("Pause / Resume:"),
		pauseResumeHotkeyLabel);

	QLabel* emergencyStopHotkeyLabel =
		new QLabel(
			tr("Ctrl+Shift+Escape"),
			hotkeyGroup);

	emergencyStopHotkeyLabel->setProperty(
		"bold",
		true);

	hotkeyLayout->addRow(
		tr("Emergency Stop:"),
		emergencyStopHotkeyLabel);

	QLabel* hotkeyExplanationLabel =
		new QLabel(
			tr(
				"F9 and F10 control the active workspace: "
				"Playback & Editing or Recording."),
			hotkeyGroup);

	hotkeyExplanationLabel->setWordWrap(
		true);

	hotkeyExplanationLabel->setProperty(
		"subheading",
		true);

	hotkeyLayout->addRow(
		hotkeyExplanationLabel);

	layout->addWidget(
		hotkeyGroup);
		
	
	
    layout->addWidget(hotkeyGroup);
    
    // Capture options
    QGroupBox* captureGroup = new QGroupBox(tr("Capture Options"));
    QVBoxLayout* captureLayout = new QVBoxLayout(captureGroup);
    
    captureMouseCheck_ = new QCheckBox(tr("Capture mouse input"));
    captureMouseCheck_->setChecked(true);
    captureLayout->addWidget(captureMouseCheck_);
    
    captureKeyboardCheck_ = new QCheckBox(tr("Capture keyboard input"));
    captureKeyboardCheck_->setChecked(true);
    captureLayout->addWidget(captureKeyboardCheck_);
    
    captureDisplayCheck_ = new QCheckBox(tr("Record display configuration"));
    captureDisplayCheck_->setChecked(true);
    captureDisplayCheck_->setToolTip(tr("Store monitor layout for compatibility checking"));
    captureLayout->addWidget(captureDisplayCheck_);
    
    layout->addWidget(captureGroup);
    
    layout->addStretch();
    
    return tab;
}

QWidget* SettingsDialog::createPlaybackTab()
{
    QWidget* tab =
        new QWidget();

    QVBoxLayout* layout =
        new QVBoxLayout(
            tab);

    // Active-workspace hotkeys.
    QGroupBox* hotkeyGroup =
        new QGroupBox(
            tr("Active Workspace Hotkeys"));

    QFormLayout* hotkeyLayout =
        new QFormLayout(
            hotkeyGroup);

    QLabel* startStopHotkeyLabel =
        new QLabel(
            tr("F9"),
            hotkeyGroup);

    startStopHotkeyLabel->setProperty(
        "bold",
        true);

    hotkeyLayout->addRow(
        tr("Start / Stop:"),
        startStopHotkeyLabel);

    QLabel* pauseResumeHotkeyLabel =
        new QLabel(
            tr("F10"),
            hotkeyGroup);

    pauseResumeHotkeyLabel->setProperty(
        "bold",
        true);

    hotkeyLayout->addRow(
        tr("Pause / Resume:"),
        pauseResumeHotkeyLabel);

    QLabel* emergencyStopHotkeyLabel =
        new QLabel(
            tr("Ctrl+Shift+Escape"),
            hotkeyGroup);

    emergencyStopHotkeyLabel->setProperty(
        "bold",
        true);

    hotkeyLayout->addRow(
        tr("Emergency Stop:"),
        emergencyStopHotkeyLabel);

    QLabel* hotkeyExplanationLabel =
        new QLabel(
            tr(
                "F9 and F10 control the active workspace: "
                "Playback & Editing or Recording."),
            hotkeyGroup);

    hotkeyExplanationLabel->setWordWrap(
        true);

    hotkeyExplanationLabel->setProperty(
        "subheading",
        true);

    hotkeyLayout->addRow(
        hotkeyExplanationLabel);

    layout->addWidget(
        hotkeyGroup);

    // Playback default options.
    QGroupBox* optionsGroup =
        new QGroupBox(
            tr("Default Options"));

    QFormLayout* optionsLayout =
        new QFormLayout(
            optionsGroup);

    QWidget* defaultLoopsWidget =
        new QWidget(
            optionsGroup);

    QHBoxLayout* defaultLoopsLayout =
        new QHBoxLayout(
            defaultLoopsWidget);

    defaultLoopsLayout->setContentsMargins(
        0,
        0,
        0,
        0);

    defaultLoopsLayout->setSpacing(
        8);

    defaultLoopsSpin_ =
        new QSpinBox(
            defaultLoopsWidget);

    defaultLoopsSpin_->setRange(
        1,
        9999);

    defaultLoopsSpin_->setValue(
        1);

    defaultLoopsSpin_->setButtonSymbols(
        QAbstractSpinBox::NoButtons);

    defaultLoopsSpin_->setMinimumWidth(
        100);

    defaultLoopsLayout->addWidget(
        defaultLoopsSpin_);

    infiniteLoopsButton_ =
        new QPushButton(
            tr("Set ∞"),
            defaultLoopsWidget);

    infiniteLoopsButton_->setProperty(
        "compact",
        true);

    infiniteLoopsButton_->setToolTip(
        tr("Use infinite playback as the default"));

    connect(
        infiniteLoopsButton_,
        &QPushButton::clicked,
        this,
        &SettingsDialog::toggleDefaultLoopsInfinite);

    defaultLoopsLayout->addWidget(
        infiniteLoopsButton_);

    defaultLoopsLayout->addStretch();

    optionsLayout->addRow(
        tr("Default loop count:"),
        defaultLoopsWidget);

    dryRunDefaultCheck_ =
        new QCheckBox(
            tr("Default to dry-run mode"));

    dryRunDefaultCheck_->setChecked(
        true);

    dryRunDefaultCheck_->setToolTip(
        tr(
            "New playbacks will use dry-run mode "
            "by default (safe)"));

    optionsLayout->addRow(
        dryRunDefaultCheck_);

    confirmRealPlaybackCheck_ =
        new QCheckBox(
            tr("Confirm before real playback"));

    confirmRealPlaybackCheck_->setChecked(
        true);

    confirmRealPlaybackCheck_->setToolTip(
        tr(
            "Show confirmation dialog before "
            "sending actual input"));

    optionsLayout->addRow(
        confirmRealPlaybackCheck_);

    layout->addWidget(
        optionsGroup);

    layout->addStretch();

    return tab;
}

QWidget* SettingsDialog::createAppearanceTab()
{
    QWidget* tab = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(tab);
    
    QGroupBox* themeGroup = new QGroupBox(tr("Theme"));
    QFormLayout* themeLayout = new QFormLayout(themeGroup);
    
    themeCombo_ = new QComboBox();
    themeCombo_->addItem(tr("Dark Blue (Default)"), static_cast<int>(ThemeType::DarkBlue));
    themeCombo_->addItem(tr("Dark Gray"), static_cast<int>(ThemeType::DarkGray));
    themeCombo_->addItem(tr("Light"), static_cast<int>(ThemeType::Light));
	themeCombo_->view()->setProperty(
		"comboPopup",
		true);
	themeCombo_->view()->setMouseTracking(
		true);
    connect(themeCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &SettingsDialog::onThemeChanged);
    themeLayout->addRow(tr("Theme:"), themeCombo_);
    
    accentColorCombo_ = new QComboBox();
	accentColorCombo_->view()->setProperty(
		"comboPopup",
		true);
	accentColorCombo_->view()->setMouseTracking(
		true);
    accentColorCombo_->setIconSize(QSize(16, 16));
    
    // Helper to create color swatch icon
    auto createColorIcon = [](const QString& color) {
        QPixmap pixmap(16, 16);
        pixmap.fill(Qt::transparent);
        QPainter painter(&pixmap);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setBrush(QColor(color));
        painter.setPen(Qt::NoPen);
        painter.drawRoundedRect(2, 2, 12, 12, 2, 2);
        return QIcon(pixmap);
    };
    
	accentColorCombo_->addItem(
		createColorIcon(
			DarkStyle::accentColorString(
				AccentColor::Cyan)),
		tr("Cyan"),
		static_cast<int>(
			AccentColor::Cyan));

	accentColorCombo_->addItem(
		createColorIcon(
			DarkStyle::accentColorString(
				AccentColor::Magenta)),
		tr("Magenta"),
		static_cast<int>(
			AccentColor::Magenta));

	accentColorCombo_->addItem(
		createColorIcon(
			DarkStyle::accentColorString(
				AccentColor::Green)),
		tr("Green"),
		static_cast<int>(
			AccentColor::Green));

	accentColorCombo_->addItem(
		createColorIcon(
			DarkStyle::accentColorString(
				AccentColor::Purple)),
		tr("Purple"),
		static_cast<int>(
			AccentColor::Purple));

	accentColorCombo_->addItem(
		createColorIcon(
			DarkStyle::accentColorString(
				AccentColor::Yellow)),
		tr("Yellow"),
		static_cast<int>(
			AccentColor::Yellow));

	accentColorCombo_->addItem(
		createColorIcon(
			DarkStyle::accentColorString(
				AccentColor::Red)),
		tr("Red"),
		static_cast<int>(
			AccentColor::Red));

	accentColorCombo_->addItem(
		createColorIcon(
			DarkStyle::accentColorString(
				AccentColor::Orange)),
		tr("Orange"),
		static_cast<int>(
			AccentColor::Orange));
    connect(accentColorCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &SettingsDialog::onAccentChanged);
    themeLayout->addRow(tr("Accent Color:"), accentColorCombo_);
    
    // Set current values
    themeCombo_->setCurrentIndex(static_cast<int>(DarkStyle::currentTheme()));
    accentColorCombo_->setCurrentIndex(static_cast<int>(DarkStyle::currentAccent()));
    
    layout->addWidget(themeGroup);
    
    QGroupBox* fontGroup = new QGroupBox(tr("Font"));
    QFormLayout* fontLayout = new QFormLayout(fontGroup);
    
    fontSizeSpin_ = new QSpinBox();
    fontSizeSpin_->setRange(8, 16);
    fontSizeSpin_->setValue(10);
    fontSizeSpin_->setSuffix(tr(" pt"));
    fontLayout->addRow(tr("Font Size:"), fontSizeSpin_);
    
    layout->addWidget(fontGroup);
    
    layout->addStretch();
    
    // Note about live preview
    QLabel* note = new QLabel(tr("Theme and accent color changes are applied immediately as a live preview."));
    note->setProperty("subheading", true);
    note->setWordWrap(true);
    layout->addWidget(note);
    
    return tab;
}

QWidget* SettingsDialog::createGeneralTab()
{
    QWidget* tab = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(tab);
    
    QGroupBox* pathGroup = new QGroupBox(tr("File Locations"));
    QFormLayout* pathLayout = new QFormLayout(pathGroup);
    
    QHBoxLayout* defaultPathLayout = new QHBoxLayout();
    defaultPathEdit_ = new QLineEdit();
    defaultPathEdit_->setPlaceholderText(tr("Default macro save location"));
    defaultPathLayout->addWidget(defaultPathEdit_);
    
    browseButton_ = new QPushButton(tr("Browse..."));
    connect(browseButton_, &QPushButton::clicked, this, [this]() {
        QString dir = QFileDialog::getExistingDirectory(this, tr("Select Default Directory"),
            defaultPathEdit_->text());
        if (!dir.isEmpty()) {
            defaultPathEdit_->setText(dir);
        }
    });
    defaultPathLayout->addWidget(browseButton_);
    
    pathLayout->addRow(tr("Default Path:"), defaultPathLayout);
    
    layout->addWidget(pathGroup);
    
    QGroupBox* saveGroup = new QGroupBox(tr("Auto-Save"));
    QVBoxLayout* saveLayout = new QVBoxLayout(saveGroup);
    
    autoSaveCheck_ = new QCheckBox(tr("Enable auto-save"));
    saveLayout->addWidget(autoSaveCheck_);
    
    QHBoxLayout* intervalLayout = new QHBoxLayout();
    intervalLayout->addWidget(new QLabel(tr("Save interval:")));
    
    autoSaveIntervalSpin_ = new QSpinBox();
    autoSaveIntervalSpin_->setRange(1, 60);
    autoSaveIntervalSpin_->setValue(5);
    autoSaveIntervalSpin_->setSuffix(tr(" minutes"));
    autoSaveIntervalSpin_->setEnabled(false);
    intervalLayout->addWidget(autoSaveIntervalSpin_);
    intervalLayout->addStretch();
    
    saveLayout->addLayout(intervalLayout);
    
    connect(autoSaveCheck_, &QCheckBox::toggled, autoSaveIntervalSpin_, &QSpinBox::setEnabled);
    
    layout->addWidget(saveGroup);
    
    // Global Hotkeys group
    QGroupBox* hotkeyGroup = new QGroupBox(tr("Global Hotkeys"));
    QVBoxLayout* hotkeyLayout = new QVBoxLayout(hotkeyGroup);
    
    globalHotkeysCheck_ = new QCheckBox(tr("Enable global hotkeys"));
    globalHotkeysCheck_->setChecked(true);
    globalHotkeysCheck_->setToolTip(tr("Allow controlling recording/playback with hotkeys even when the window is not focused"));
    hotkeyLayout->addWidget(globalHotkeysCheck_);
    
    QLabel* hotkeyHint =
		new QLabel(
			tr(
				"Active workspace hotkeys: "
				"F9 (Start/Stop), "
				"F10 (Pause/Resume), "
				"Ctrl+Shift+Escape (Emergency Stop)"));
    hotkeyHint->setProperty("subheading", true);
    hotkeyHint->setWordWrap(true);
    hotkeyLayout->addWidget(hotkeyHint);
    
    // System Tray group
    QGroupBox* trayGroup = new QGroupBox(tr("System Tray"));
    QVBoxLayout* trayLayout = new QVBoxLayout(trayGroup);
    
    showTrayIconCheck_ = new QCheckBox(tr("Show system tray icon"));
    showTrayIconCheck_->setChecked(true);
    trayLayout->addWidget(showTrayIconCheck_);
    
    minimizeToTrayCheck_ = new QCheckBox(tr("Minimize to system tray"));
    minimizeToTrayCheck_->setChecked(false);
    minimizeToTrayCheck_->setEnabled(showTrayIconCheck_->isChecked());
    trayLayout->addWidget(minimizeToTrayCheck_);
    
    showNotificationsCheck_ = new QCheckBox(tr("Show notifications"));
    showNotificationsCheck_->setChecked(true);
    showNotificationsCheck_->setEnabled(showTrayIconCheck_->isChecked());
    trayLayout->addWidget(showNotificationsCheck_);
    
    connect(showTrayIconCheck_, &QCheckBox::toggled, minimizeToTrayCheck_, &QCheckBox::setEnabled);
    connect(showTrayIconCheck_, &QCheckBox::toggled, showNotificationsCheck_, &QCheckBox::setEnabled);
    
    layout->addWidget(trayGroup);
    
    layout->addStretch();
    
    return tab;
}

void SettingsDialog::toggleDefaultLoopsInfinite()
{
    defaultLoopsInfinite_ =
        !defaultLoopsInfinite_;

    defaultLoopsSpin_->setEnabled(
        !defaultLoopsInfinite_);

    if (defaultLoopsInfinite_)
    {
        defaultLoopsSpin_->setSpecialValueText(
            tr("∞"));

        defaultLoopsSpin_->setValue(
            defaultLoopsSpin_->minimum());

        infiniteLoopsButton_->setText(
            tr("Set Finite"));

        infiniteLoopsButton_->setToolTip(
            tr("Use a finite playback loop count"));
    }
    else
    {
        defaultLoopsSpin_->setSpecialValueText(
            QString());

        defaultLoopsSpin_->setValue(
            1);

        infiniteLoopsButton_->setText(
            tr("Set ∞"));

        infiniteLoopsButton_->setToolTip(
            tr("Use infinite playback as the default"));
    }
}

void SettingsDialog::loadSettings()
{
 
    defaultLoopsInfinite_ =
		settings_.defaultLoops == 0;

	if (defaultLoopsInfinite_)
	{
		defaultLoopsSpin_->setSpecialValueText(
			tr("∞"));

		defaultLoopsSpin_->setValue(
			defaultLoopsSpin_->minimum());

		defaultLoopsSpin_->setEnabled(
			false);

		infiniteLoopsButton_->setText(
			tr("Set Finite"));

		infiniteLoopsButton_->setToolTip(
			tr("Use a finite playback loop count"));
	}
	else
	{
		defaultLoopsSpin_->setSpecialValueText(
			QString());

		defaultLoopsSpin_->setValue(
			settings_.defaultLoops);

		defaultLoopsSpin_->setEnabled(
			true);

		infiniteLoopsButton_->setText(
			tr("Set ∞"));

		infiniteLoopsButton_->setToolTip(
			tr("Use infinite playback as the default"));
	}

	QSettings playbackSettings(
		"InputPlay",
		"Studio");

	dryRunDefaultCheck_->setChecked(
		playbackSettings.value(
			"playback/dryRunDefault",
			true)
			.toBool());

	confirmRealPlaybackCheck_->setChecked(
		playbackSettings.value(
			"playback/confirmRealPlayback",
			true)
			.toBool());
			
	
    
    // Load QSettings for GUI-specific settings
    QSettings qsettings("InputPlay", "Studio");
    globalHotkeysCheck_->setChecked(qsettings.value("hotkeysEnabled", true).toBool());
    showTrayIconCheck_->setChecked(qsettings.value("showTrayIcon", true).toBool());
    minimizeToTrayCheck_->setChecked(qsettings.value("minimizeToTray", false).toBool());
    showNotificationsCheck_->setChecked(qsettings.value("showNotifications", true).toBool());
}

void SettingsDialog::saveSettings()
{

   
    
    settings_.defaultLoops =
		defaultLoopsInfinite_
		? 0
		: defaultLoopsSpin_->value();

    
    // Save QSettings for GUI-specific settings
    QSettings qsettings("InputPlay", "Studio");
	
	qsettings.setValue(
		"playback/defaultLoops",
		defaultLoopsInfinite_
		? 0
		: defaultLoopsSpin_->value());

	qsettings.setValue(
		"playback/dryRunDefault",
		dryRunDefaultCheck_->isChecked());

	qsettings.setValue(
		"playback/confirmRealPlayback",
		confirmRealPlaybackCheck_->isChecked());
	
    qsettings.setValue("hotkeysEnabled", globalHotkeysCheck_->isChecked());
    qsettings.setValue("showTrayIcon", showTrayIconCheck_->isChecked());
    qsettings.setValue("minimizeToTray", minimizeToTrayCheck_->isChecked());
    qsettings.setValue("showNotifications", showNotificationsCheck_->isChecked());
}

void SettingsDialog::accept()
{
    saveSettings();
    QDialog::accept();
}

void SettingsDialog::resetToDefaults()
{
    
    defaultLoopsInfinite_ =
		false;

	defaultLoopsSpin_->setSpecialValueText(
		QString());

	defaultLoopsSpin_->setEnabled(
		true);

	defaultLoopsSpin_->setValue(
		1);

	infiniteLoopsButton_->setText(
		tr("Set ∞"));

	infiniteLoopsButton_->setToolTip(
		tr("Use infinite playback as the default"));
		
    // Reset appearance
    themeCombo_->setCurrentIndex(0);
    accentColorCombo_->setCurrentIndex(0);
    fontSizeSpin_->setValue(10);
    
    // Reset general
    dryRunDefaultCheck_->setChecked(true);
    confirmRealPlaybackCheck_->setChecked(true);
    captureMouseCheck_->setChecked(true);
    captureKeyboardCheck_->setChecked(true);
    captureDisplayCheck_->setChecked(true);
    autoSaveCheck_->setChecked(false);
    autoSaveIntervalSpin_->setValue(5);
}

ThemeType SettingsDialog::selectedTheme() const
{
    return static_cast<ThemeType>(themeCombo_->currentData().toInt());
}

AccentColor SettingsDialog::selectedAccent() const
{
    return static_cast<AccentColor>(accentColorCombo_->currentData().toInt());
}

void SettingsDialog::onThemeChanged()
{
    emit appearanceChanged(selectedTheme(), selectedAccent());
}

void SettingsDialog::onAccentChanged()
{
    emit appearanceChanged(selectedTheme(), selectedAccent());
}
