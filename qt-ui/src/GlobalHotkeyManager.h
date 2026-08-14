#pragma once

#include <QObject>
#include <QTimer>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

class GlobalHotkeyManager : public QObject
{
    Q_OBJECT

public:
    explicit GlobalHotkeyManager(QObject* parent = nullptr);
    ~GlobalHotkeyManager() override;

    // Hotkey IDs
    enum HotkeyId {
        RecordStartStop = 1,
        RecordPause = 2,
        PlaybackStartStop = 3,
        PlaybackPause = 4,
        EmergencyStop = 5
    };

    bool registerHotkey(HotkeyId id, int virtualKey, Qt::KeyboardModifiers modifiers = Qt::NoModifier);
    bool unregisterHotkey(HotkeyId id);
    void unregisterAll();
    
    bool isEnabled() const { return enabled_; }
    void setEnabled(bool enabled);

    static QString keyName(int virtualKey);
    static int keyFromName(const QString& name);

signals:
    void recordStartStopTriggered();
    void recordPauseTriggered();
    void playbackStartStopTriggered();
    void playbackPauseTriggered();
    void emergencyStopTriggered();

private slots:
    void pollHotkeys();

private:
    QTimer* pollTimer_ = nullptr;
    bool enabled_ = false;
    
#ifdef _WIN32
    struct RegisteredHotkey {
        int id;
        int virtualKey;
        UINT modifiers;
        bool registered;
    };
    QList<RegisteredHotkey> registeredHotkeys_;
#endif
};
