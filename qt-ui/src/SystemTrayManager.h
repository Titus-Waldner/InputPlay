#pragma once

#include <QObject>

class QSystemTrayIcon;
class QMenu;
class QAction;
class MainWindow;

class SystemTrayManager : public QObject
{
    Q_OBJECT

public:
    explicit SystemTrayManager(MainWindow* mainWindow, QObject* parent = nullptr);
    ~SystemTrayManager() override;

    bool isAvailable() const;
    void setVisible(bool visible);
    bool isVisible() const;
    
    void showMessage(const QString& title, const QString& message, 
                     int iconType = 0, int millisecondsTimeoutHint = 3000);
    
    void setRecordingState(bool recording, bool paused = false);
    void setPlaybackState(bool playing, bool paused = false);
    void setIdleState();

signals:
    void showWindowRequested();
    void exitRequested();
    void quickRecordRequested();
    void quickPlayRequested();

private slots:
    void onTrayActivated(int reason);
    void updateIcon();
    void updateMenu();

private:
    void setupTrayIcon();
    void setupContextMenu();
    
    MainWindow* mainWindow_ = nullptr;
    QSystemTrayIcon* trayIcon_ = nullptr;
    QMenu* trayMenu_ = nullptr;
    
    QAction* showAction_ = nullptr;
    QAction* recordAction_ = nullptr;
    QAction* playAction_ = nullptr;
    QAction* exitAction_ = nullptr;
    
    bool recording_ = false;
    bool recordingPaused_ = false;
    bool playing_ = false;
    bool playingPaused_ = false;
};
