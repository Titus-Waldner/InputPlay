#include "GlobalHotkeyManager.h"

#include <QCoreApplication>

GlobalHotkeyManager::GlobalHotkeyManager(
    QObject* parent)
    : QObject(parent)
{
    if (QCoreApplication::instance())
    {
        QCoreApplication::instance()
            ->installNativeEventFilter(this);
    }
}

GlobalHotkeyManager::~GlobalHotkeyManager()
{
    if (QCoreApplication::instance())
    {
        QCoreApplication::instance()
            ->removeNativeEventFilter(this);
    }

    unregisterAll();
}

bool GlobalHotkeyManager::registerHotkey(
    HotkeyId id,
    int virtualKey,
    Qt::KeyboardModifiers modifiers)
{
#ifdef _WIN32
    UINT windowsModifiers =
        MOD_NOREPEAT;

    if (modifiers.testFlag(Qt::AltModifier))
    {
        windowsModifiers |= MOD_ALT;
    }

    if (modifiers.testFlag(Qt::ControlModifier))
    {
        windowsModifiers |= MOD_CONTROL;
    }

    if (modifiers.testFlag(Qt::ShiftModifier))
    {
        windowsModifiers |= MOD_SHIFT;
    }

    if (modifiers.testFlag(Qt::MetaModifier))
    {
        windowsModifiers |= MOD_WIN;
    }

    unregisterHotkey(id);

    const BOOL registered =
        RegisterHotKey(
            nullptr,
            static_cast<int>(id),
            windowsModifiers,
            static_cast<UINT>(virtualKey));

    if (!registered)
    {
        return false;
    }

    RegisteredHotkey hotkey;

    hotkey.id =
        id;

    hotkey.virtualKey =
        virtualKey;

    hotkey.modifiers =
        windowsModifiers;

    hotkey.registered =
        true;

    registeredHotkeys_.append(
        hotkey);

    return true;
#else
    Q_UNUSED(id);
    Q_UNUSED(virtualKey);
    Q_UNUSED(modifiers);

    return false;
#endif
}

bool GlobalHotkeyManager::unregisterHotkey(
    HotkeyId id)
{
#ifdef _WIN32
    for (int index = 0;
         index < registeredHotkeys_.size();
         ++index)
    {
        if (registeredHotkeys_[index].id != id)
        {
            continue;
        }

        if (registeredHotkeys_[index].registered)
        {
            UnregisterHotKey(
                nullptr,
                static_cast<int>(id));
        }

        registeredHotkeys_.removeAt(
            index);

        return true;
    }
#else
    Q_UNUSED(id);
#endif

    return false;
}

void GlobalHotkeyManager::unregisterAll()
{
#ifdef _WIN32
    for (const RegisteredHotkey& hotkey
         : registeredHotkeys_)
    {
        if (!hotkey.registered)
        {
            continue;
        }

        UnregisterHotKey(
            nullptr,
            static_cast<int>(hotkey.id));
    }

    registeredHotkeys_.clear();
#endif
}

void GlobalHotkeyManager::setEnabled(
    bool enabled)
{
    enabled_ =
        enabled;
}

bool GlobalHotkeyManager::nativeEventFilter(
    const QByteArray& eventType,
    void* message,
    qintptr* result)
{
    Q_UNUSED(eventType);
    Q_UNUSED(result);

#ifdef _WIN32
    if (!enabled_
        || !message)
    {
        return false;
    }

    MSG* nativeMessage =
        static_cast<MSG*>(message);

    if (nativeMessage->message != WM_HOTKEY)
    {
        return false;
    }

    const int hotkeyId =
        static_cast<int>(
            nativeMessage->wParam);

    handleHotkey(
        hotkeyId);
#else
    Q_UNUSED(message);
#endif

    /*
     * Return false so Qt may continue processing the native message.
     * The hotkey action has already been dispatched.
     */
    return false;
}

void GlobalHotkeyManager::handleHotkey(
    int id)
{
    switch (id)
    {
        case RecordStartStop:
            emit recordStartStopTriggered();
            break;

        case RecordPause:
            emit recordPauseTriggered();
            break;

        case PlaybackStartStop:
            emit playbackStartStopTriggered();
            break;

        case PlaybackPause:
            emit playbackPauseTriggered();
            break;

        case EmergencyStop:
            emit emergencyStopTriggered();
            break;

        default:
            break;
    }
}

QString GlobalHotkeyManager::keyName(
    int virtualKey)
{
#ifdef _WIN32
    switch (virtualKey)
    {
        case VK_F1: return QStringLiteral("F1");
        case VK_F2: return QStringLiteral("F2");
        case VK_F3: return QStringLiteral("F3");
        case VK_F4: return QStringLiteral("F4");
        case VK_F5: return QStringLiteral("F5");
        case VK_F6: return QStringLiteral("F6");
        case VK_F7: return QStringLiteral("F7");
        case VK_F8: return QStringLiteral("F8");
        case VK_F9: return QStringLiteral("F9");
        case VK_F10: return QStringLiteral("F10");
        case VK_F11: return QStringLiteral("F11");
        case VK_F12: return QStringLiteral("F12");
        case VK_ESCAPE: return QStringLiteral("Escape");
        case VK_PAUSE: return QStringLiteral("Pause");
        case VK_SCROLL: return QStringLiteral("Scroll Lock");
        case VK_INSERT: return QStringLiteral("Insert");
        case VK_DELETE: return QStringLiteral("Delete");
        case VK_HOME: return QStringLiteral("Home");
        case VK_END: return QStringLiteral("End");
        case VK_PRIOR: return QStringLiteral("Page Up");
        case VK_NEXT: return QStringLiteral("Page Down");

        default:
            break;
    }

    if (virtualKey >= 0x30
        && virtualKey <= 0x39)
    {
        return QString::number(
            virtualKey - 0x30);
    }

    if (virtualKey >= 0x41
        && virtualKey <= 0x5A)
    {
        return QString(
            QChar(virtualKey));
    }

    return QStringLiteral("0x%1")
        .arg(
            virtualKey,
            2,
            16,
            QChar('0'))
        .toUpper();
#else
    return QStringLiteral("Key %1")
        .arg(virtualKey);
#endif
}

int GlobalHotkeyManager::keyFromName(
    const QString& name)
{
#ifdef _WIN32
    if (name == QStringLiteral("F1")) return VK_F1;
    if (name == QStringLiteral("F2")) return VK_F2;
    if (name == QStringLiteral("F3")) return VK_F3;
    if (name == QStringLiteral("F4")) return VK_F4;
    if (name == QStringLiteral("F5")) return VK_F5;
    if (name == QStringLiteral("F6")) return VK_F6;
    if (name == QStringLiteral("F7")) return VK_F7;
    if (name == QStringLiteral("F8")) return VK_F8;
    if (name == QStringLiteral("F9")) return VK_F9;
    if (name == QStringLiteral("F10")) return VK_F10;
    if (name == QStringLiteral("F11")) return VK_F11;
    if (name == QStringLiteral("F12")) return VK_F12;
    if (name == QStringLiteral("Escape")) return VK_ESCAPE;
    if (name == QStringLiteral("Pause")) return VK_PAUSE;
    if (name == QStringLiteral("Scroll Lock")) return VK_SCROLL;
    if (name == QStringLiteral("Insert")) return VK_INSERT;
    if (name == QStringLiteral("Delete")) return VK_DELETE;
    if (name == QStringLiteral("Home")) return VK_HOME;
    if (name == QStringLiteral("End")) return VK_END;
    if (name == QStringLiteral("Page Up")) return VK_PRIOR;
    if (name == QStringLiteral("Page Down")) return VK_NEXT;

    if (name.length() == 1)
    {
        const QChar character =
            name.at(0).toUpper();

        if (character >= QChar('0')
            && character <= QChar('9'))
        {
            return 0x30
                + character.unicode()
                - QChar('0').unicode();
        }

        if (character >= QChar('A')
            && character <= QChar('Z'))
        {
            return character.unicode();
        }
    }
#else
    Q_UNUSED(name);
#endif

    return 0;
}