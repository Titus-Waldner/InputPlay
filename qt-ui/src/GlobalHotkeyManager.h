#pragma once

#include <QAbstractNativeEventFilter>
#include <QByteArray>
#include <QList>
#include <QObject>
#include <QString>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

class GlobalHotkeyManager
    : public QObject,
      public QAbstractNativeEventFilter
{
    Q_OBJECT

public:
    enum HotkeyId
    {
        RecordStartStop = 1,
        RecordPause = 2,
        PlaybackStartStop = 3,
        PlaybackPause = 4,
        EmergencyStop = 5
    };

    explicit GlobalHotkeyManager(
        QObject* parent = nullptr);

    ~GlobalHotkeyManager() override;

    bool registerHotkey(
        HotkeyId id,
        int virtualKey,
        Qt::KeyboardModifiers modifiers =
            Qt::NoModifier);

    bool unregisterHotkey(
        HotkeyId id);

    void unregisterAll();

    bool isEnabled() const
    {
        return enabled_;
    }

    void setEnabled(
        bool enabled);

    static QString keyName(
        int virtualKey);

    static int keyFromName(
        const QString& name);

    bool nativeEventFilter(
        const QByteArray& eventType,
        void* message,
        qintptr* result) override;

signals:
    void recordStartStopTriggered();
    void recordPauseTriggered();
    void playbackStartStopTriggered();
    void playbackPauseTriggered();
    void emergencyStopTriggered();

private:
    void handleHotkey(
        int id);

#ifdef _WIN32
    struct RegisteredHotkey
    {
        HotkeyId id;
        int virtualKey;
        UINT modifiers;
        bool registered;
    };

    QList<RegisteredHotkey> registeredHotkeys_;
#endif

    bool enabled_ = false;
};