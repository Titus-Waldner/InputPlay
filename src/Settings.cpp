#include "Settings.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <string>

std::string keyNameFromVirtualKey(
    int virtualKey)
{
    if (virtualKey >= VK_F1
        && virtualKey <= VK_F12)
    {
        return
            "F"
            + std::to_string(
                virtualKey - VK_F1 + 1);
    }

    return "Unknown";
}