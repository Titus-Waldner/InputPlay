#include "SystemTrayManager.h"
#include "MainWindow.h"
#include "DarkStyle.h"

#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>
#include <QApplication>
#include <QPainter>
#include <QPixmap>

SystemTrayManager::SystemTrayManager(MainWindow* mainWindow, QObject* parent)
    : QObject(parent)
    , mainWindow_(mainWindow)
{
    setupTrayIcon();
    setupContextMenu();
}

SystemTrayManager::~SystemTrayManager()
{
    if (trayIcon_) {
        trayIcon_->hide();
    }
}

bool SystemTrayManager::isAvailable() const
{
    return QSystemTrayIcon::isSystemTrayAvailable();
}

void SystemTrayManager::setVisible(bool visible)
{
    if (trayIcon_) {
        trayIcon_->setVisible(visible);
    }
}

bool SystemTrayManager::isVisible() const
{
    return trayIcon_ && trayIcon_->isVisible();
}

void SystemTrayManager::showMessage(const QString& title, const QString& message,
                                     int iconType, int millisecondsTimeoutHint)
{
    if (trayIcon_ && trayIcon_->isVisible()) {
        QSystemTrayIcon::MessageIcon icon = static_cast<QSystemTrayIcon::MessageIcon>(iconType);
        trayIcon_->showMessage(title, message, icon, millisecondsTimeoutHint);
    }
}

void SystemTrayManager::setRecordingState(bool recording, bool paused)
{
    recording_ = recording;
    recordingPaused_ = paused;
    playing_ = false;
    playingPaused_ = false;
    updateIcon();
    updateMenu();
}

void SystemTrayManager::setPlaybackState(bool playing, bool paused)
{
    recording_ = false;
    recordingPaused_ = false;
    playing_ = playing;
    playingPaused_ = paused;
    updateIcon();
    updateMenu();
}

void SystemTrayManager::setIdleState()
{
    recording_ = false;
    recordingPaused_ = false;
    playing_ = false;
    playingPaused_ = false;
    updateIcon();
    updateMenu();
}

void SystemTrayManager::setupTrayIcon()
{
    trayIcon_ = new QSystemTrayIcon(this);
    
    // Create a default icon
    QPixmap pixmap(32, 32);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // Draw a play/record symbol
    painter.setBrush(
		DarkStyle::toneColor(
			"accent"));
    painter.setPen(Qt::NoPen);
    
    // Draw a rounded rectangle with play triangle
    painter.drawRoundedRect(2, 2, 28, 28, 6, 6);
    painter.setPen(
		DarkStyle::toneColor(
			"primary"));
    
    QPolygon triangle;
    triangle << QPoint(12, 8) << QPoint(12, 24) << QPoint(24, 16);
    painter.drawPolygon(triangle);
    
    painter.end();
    
    trayIcon_->setIcon(QIcon(pixmap));
    trayIcon_->setToolTip(tr("InputPlay Studio"));
    
    connect(trayIcon_, &QSystemTrayIcon::activated, this, &SystemTrayManager::onTrayActivated);
}

void SystemTrayManager::setupContextMenu()
{
    trayMenu_ = new QMenu();
    
    showAction_ = trayMenu_->addAction(tr("Show Window"));
    connect(showAction_, &QAction::triggered, this, &SystemTrayManager::showWindowRequested);
    
    trayMenu_->addSeparator();
    
    recordAction_ = trayMenu_->addAction(tr("Start Recording"));
    connect(recordAction_, &QAction::triggered, this, &SystemTrayManager::quickRecordRequested);
    
    playAction_ = trayMenu_->addAction(tr("Start Playback"));
    connect(playAction_, &QAction::triggered, this, &SystemTrayManager::quickPlayRequested);
    
    trayMenu_->addSeparator();
    
    exitAction_ = trayMenu_->addAction(tr("Exit"));
    connect(exitAction_, &QAction::triggered, this, &SystemTrayManager::exitRequested);
    
    trayIcon_->setContextMenu(trayMenu_);
}

void SystemTrayManager::onTrayActivated(int reason)
{
    QSystemTrayIcon::ActivationReason activationReason = 
        static_cast<QSystemTrayIcon::ActivationReason>(reason);
    
    if (activationReason == QSystemTrayIcon::DoubleClick ||
        activationReason == QSystemTrayIcon::Trigger) {
        emit showWindowRequested();
    }
}

void SystemTrayManager::updateIcon()
{
    QPixmap pixmap(
        32,
        32);

    pixmap.fill(
        Qt::transparent);

    QPainter painter(
        &pixmap);

    painter.setRenderHint(
        QPainter::Antialiasing);

    QColor backgroundColor;
    QString symbol;

    if (recording_)
    {
        if (recordingPaused_)
        {
            backgroundColor =
                DarkStyle::toneColor(
                    "accentLight");

            symbol = "II";
        }
        else
        {
            backgroundColor =
                DarkStyle::toneColor(
                    "accent");

            symbol = "●";
        }
    }
    else if (playing_)
    {
        if (playingPaused_)
        {
            backgroundColor =
                DarkStyle::toneColor(
                    "accentLight");

            symbol = "II";
        }
        else
        {
            backgroundColor =
                DarkStyle::toneColor(
                    "accent");

            symbol = "▶";
        }
    }
    else
    {
        backgroundColor =
            DarkStyle::toneColor(
                "accentDark");

        symbol = "▶";
    }

    // Draw the tray-icon background.
    painter.setBrush(
        backgroundColor);

    painter.setPen(
        Qt::NoPen);

    painter.drawRoundedRect(
        2,
        2,
        28,
        28,
        6,
        6);

    // Draw the state symbol using the active template text color.
    painter.setPen(
        DarkStyle::toneColor(
            "primary"));

    QFont font;

    font.setPixelSize(
        16);

    font.setBold(
        true);

    painter.setFont(
        font);

    painter.drawText(
        pixmap.rect(),
        Qt::AlignCenter,
        symbol);

    painter.end();

    trayIcon_->setIcon(
        QIcon(
            pixmap));

    QString tooltip =
        tr("InputPlay Studio");

    if (recording_)
    {
        tooltip +=
            recordingPaused_
            ? tr(" - Recording Paused")
            : tr(" - Recording");
    }
    else if (playing_)
    {
        tooltip +=
            playingPaused_
            ? tr(" - Playback Paused")
            : tr(" - Playing");
    }

    trayIcon_->setToolTip(
        tooltip);
}


void SystemTrayManager::updateMenu()
{
    if (recording_) {
        recordAction_->setText(recordingPaused_ ? tr("Resume Recording") : tr("Stop Recording"));
        playAction_->setEnabled(false);
    } else if (playing_) {
        recordAction_->setEnabled(false);
        playAction_->setText(playingPaused_ ? tr("Resume Playback") : tr("Stop Playback"));
    } else {
        recordAction_->setText(tr("Start Recording"));
        recordAction_->setEnabled(true);
        playAction_->setText(tr("Start Playback"));
        playAction_->setEnabled(true);
    }
}
