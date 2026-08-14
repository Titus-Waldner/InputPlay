#include "GlobalHotkeyManager.h"

#ifdef _WIN32
#include <windows.h>
#endif

GlobalHotkeyManager::GlobalHotkeyManager(QObject* parent)
    : QObject(parent)
    , pollTimer_(new QTimer(this))
{
    connect(pollTimer_, &QTimer::timeout, this, &GlobalHotkeyManager::pollHotkeys);
}

GlobalHotkeyManager::~GlobalHotkeyManager()
{
    unregisterAll();
}

bool GlobalHotkeyManager::registerHotkey(HotkeyId id, int virtualKey, Qt::KeyboardModifiers modifiers)
{
#ifdef _WIN32
    // Convert Qt modifiers to Windows modifiers
    UINT winMods = 0;
    if (modifiers & Qt::AltModifier) winMods |= MOD_ALT;
    if (modifiers & Qt::ControlModifier) winMods |= MOD_CONTROL;
    if (modifiers & Qt::ShiftModifier) winMods |= MOD_SHIFT;
    
    // First unregister if already registered
    unregisterHotkey(id);
    
    // Register the hotkey
    if (RegisterHotKey(nullptr, id, winMods | MOD_NOREPEAT, virtualKey)) {
        RegisteredHotkey hk;
        hk.id = id;
        hk.virtualKey = virtualKey;
        hk.modifiers = winMods;
        hk.registered = true;
        registeredHotkeys_.append(hk);
        
        // Start polling if not already
        if (!pollTimer_->isActive() && enabled_) {
            pollTimer_->start(50); // Poll every 50ms
        }
        
        return true;
    }
    
    return false;
#else
    Q_UNUSED(id);
    Q_UNUSED(virtualKey);
    Q_UNUSED(modifiers);
    return false;
#endif
}

bool GlobalHotkeyManager::unregisterHotkey(HotkeyId id)
{
#ifdef _WIN32
    for (int i = 0; i < registeredHotkeys_.size(); ++i) {
        if (registeredHotkeys_[i].id == id) {
            if (registeredHotkeys_[i].registered) {
                UnregisterHotKey(nullptr, id);
            }
            registeredHotkeys_.removeAt(i);
            return true;
        }
    }
#else
    Q_UNUSED(id);
#endif
    return false;
}

void GlobalHotkeyManager::unregisterAll()
{
#ifdef _WIN32
    for (const auto& hk : registeredHotkeys_) {
        if (hk.registered) {
            UnregisterHotKey(nullptr, hk.id);
        }
    }
    registeredHotkeys_.clear();
#endif
    
    pollTimer_->stop();
}

void GlobalHotkeyManager::setEnabled(bool enabled)
{
    enabled_ = enabled;
    
    if (enabled && !registeredHotkeys_.isEmpty()) {
        pollTimer_->start(50);
    } else {
        pollTimer_->stop();
    }
}

void GlobalHotkeyManager::pollHotkeys()
{
    if (!enabled_) {
        return;
    }

#ifdef _WIN32
    MSG msg;
    while (PeekMessage(&msg, nullptr, WM_HOTKEY, WM_HOTKEY, PM_REMOVE)) {
        if (msg.message == WM_HOTKEY) {
            int id = static_cast<int>(msg.wParam);
            
            switch (id) {
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
            }
        }
    }
#endif
}

QString GlobalHotkeyManager::keyName(int virtualKey)
{
#ifdef _WIN32
    switch (virtualKey) {
        case VK_F1: return "F1";
        case VK_F2: return "F2";
        case VK_F3: return "F3";
        case VK_F4: return "F4";
        case VK_F5: return "F5";
        case VK_F6: return "F6";
        case VK_F7: return "F7";
        case VK_F8: return "F8";
        case VK_F9: return "F9";
        case VK_F10: return "F10";
        case VK_F11: return "F11";
        case VK_F12: return "F12";
        case VK_ESCAPE: return "Escape";
        case VK_PAUSE: return "Pause";
        case VK_SCROLL: return "Scroll Lock";
        case VK_INSERT: return "Insert";
        case VK_DELETE: return "Delete";
        case VK_HOME: return "Home";
        case VK_END: return "End";
        case VK_PRIOR: return "Page Up";
        case VK_NEXT: return "Page Down";
        default:
            if (virtualKey >= 0x30 && virtualKey <= 0x39) {
                return QString::number(virtualKey - 0x30);
            }
            if (virtualKey >= 0x41 && virtualKey <= 0x5A) {
                return QString(QChar(virtualKey));
            }
            return QString("0x%1").arg(virtualKey, 2, 16, QChar('0')).toUpper();
    }
#else
    return QString("Key %1").arg(virtualKey);
#endif
}

int GlobalHotkeyManager::keyFromName(const QString& name)
{
#ifdef _WIN32
    if (name == "F1") return VK_F1;
    if (name == "F2") return VK_F2;
    if (name == "F3") return VK_F3;
    if (name == "F4") return VK_F4;
    if (name == "F5") return VK_F5;
    if (name == "F6") return VK_F6;
    if (name == "F7") return VK_F7;
    if (name == "F8") return VK_F8;
    if (name == "F9") return VK_F9;
    if (name == "F10") return VK_F10;
    if (name == "F11") return VK_F11;
    if (name == "F12") return VK_F12;
    if (name == "Escape") return VK_ESCAPE;
    if (name == "Pause") return VK_PAUSE;
    if (name == "Scroll Lock") return VK_SCROLL;
    if (name == "Insert") return VK_INSERT;
    if (name == "Delete") return VK_DELETE;
    if (name == "Home") return VK_HOME;
    if (name == "End") return VK_END;
    if (name == "Page Up") return VK_PRIOR;
    if (name == "Page Down") return VK_NEXT;
    
    if (name.length() == 1) {
        QChar c = name[0].toUpper();
        if (c >= '0' && c <= '9') {
            return 0x30 + (c.unicode() - '0');
        }
        if (c >= 'A' && c <= 'Z') {
            return c.unicode();
        }
    }
#else
    Q_UNUSED(name);
#endif
    return 0;
}
