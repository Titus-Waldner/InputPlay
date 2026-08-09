#include "Settings.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <filesystem>
#include <fstream>
#include <limits>
#include <map>
#include <string>

namespace
{
constexpr const char* DefaultSettingsContents =
    "# InputPlay settings\n"
    "# Supported shortcut names: F1 through F12\n"
    "\n"
    "record_start=F9\n"
    "record_pause=F10\n"
    "record_stop=F12\n"
    "\n"
    "play_start=F9\n"
    "play_pause=F10\n"
    "play_cancel=F12\n"
    "\n"
    "default_loops=1\n";

std::string trim(const std::string& value)
{
    const std::size_t first =
        value.find_first_not_of(" \t\r\n");

    if (first == std::string::npos)
    {
        return {};
    }

    const std::size_t last =
        value.find_last_not_of(" \t\r\n");

    return value.substr(
        first,
        last - first + 1);
}

int virtualKeyFromName(const std::string& name)
{
    static const std::map<std::string, int> keys =
    {
        {"F1", VK_F1},
        {"F2", VK_F2},
        {"F3", VK_F3},
        {"F4", VK_F4},
        {"F5", VK_F5},
        {"F6", VK_F6},
        {"F7", VK_F7},
        {"F8", VK_F8},
        {"F9", VK_F9},
        {"F10", VK_F10},
        {"F11", VK_F11},
        {"F12", VK_F12}
    };

    const auto iterator = keys.find(name);

    if (iterator == keys.end())
    {
        return 0;
    }

    return iterator->second;
}

bool parseLoopCount(
    const std::string& value,
    unsigned int& loopCount)
{
    try
    {
        std::size_t parsedLength = 0;

        const unsigned long parsed =
            std::stoul(
                value,
                &parsedLength,
                10);

        if (parsedLength != value.length())
        {
            return false;
        }

        if (parsed == 0)
        {
            return false;
        }

        if (parsed
            > std::numeric_limits<unsigned int>::max())
        {
            return false;
        }

        loopCount =
            static_cast<unsigned int>(parsed);

        return true;
    }
    catch (...)
    {
        return false;
    }
}

std::filesystem::path executableDirectory()
{
    wchar_t executablePath[MAX_PATH]{};

    const DWORD length = GetModuleFileNameW(
        nullptr,
        executablePath,
        MAX_PATH);

    if (length == 0 || length >= MAX_PATH)
    {
        return {};
    }

    return std::filesystem::path(executablePath)
        .parent_path();
}

bool assignShortcut(
    const std::string& value,
    int& target,
    const std::string& settingName,
    std::string& errorMessage)
{
    const int virtualKey =
        virtualKeyFromName(value);

    if (virtualKey == 0)
    {
        errorMessage =
            "Invalid shortcut for "
            + settingName
            + ": "
            + value
            + ".";

        return false;
    }

    target = virtualKey;
    return true;
}

bool requireSetting(
    const std::map<std::string, std::string>& values,
    const std::string& name,
    std::string& value,
    std::string& errorMessage)
{
    const auto iterator = values.find(name);

    if (iterator == values.end())
    {
        errorMessage =
            "Missing required setting: "
            + name
            + ".";

        return false;
    }

    value = iterator->second;
    return true;
}
}

bool loadOrCreateSettings(
    Settings& settings,
    std::string& settingsPath,
    std::string& errorMessage)
{
    const std::filesystem::path directory =
        executableDirectory();

    if (directory.empty())
    {
        errorMessage =
            "Unable to determine the executable directory.";

        return false;
    }

    const std::filesystem::path path =
        directory / "settings.config";

    settingsPath = path.string();

    if (!std::filesystem::exists(path))
    {
        std::ofstream newFile(path);

        if (!newFile)
        {
            errorMessage =
                "Unable to create settings.config.";

            return false;
        }

        newFile << DefaultSettingsContents;

        if (!newFile)
        {
            errorMessage =
                "Unable to write the default settings.";

            return false;
        }
    }

    std::ifstream inputFile(path);

    if (!inputFile)
    {
        errorMessage =
            "Unable to open settings.config.";

        return false;
    }

    std::map<std::string, std::string> values;
    std::string line;

    while (std::getline(inputFile, line))
    {
        line = trim(line);

        if (line.empty() || line[0] == '#')
        {
            continue;
        }

        const std::size_t separator =
            line.find('=');

        if (separator == std::string::npos)
        {
            errorMessage =
                "Invalid settings line: "
                + line;

            return false;
        }

        const std::string name =
            trim(line.substr(0, separator));

        const std::string value =
            trim(line.substr(separator + 1));

        if (name.empty())
        {
            errorMessage =
                "A settings entry has an empty name.";

            return false;
        }

        if (value.empty())
        {
            errorMessage =
                "The setting "
                + name
                + " has an empty value.";

            return false;
        }

        values[name] = value;
    }

    std::string value;

    if (!requireSetting(
            values,
            "record_start",
            value,
            errorMessage)
        || !assignShortcut(
            value,
            settings.recordStartKey,
            "record_start",
            errorMessage))
    {
        return false;
    }

    if (!requireSetting(
            values,
            "record_pause",
            value,
            errorMessage)
        || !assignShortcut(
            value,
            settings.recordPauseKey,
            "record_pause",
            errorMessage))
    {
        return false;
    }

    if (!requireSetting(
            values,
            "record_stop",
            value,
            errorMessage)
        || !assignShortcut(
            value,
            settings.recordStopKey,
            "record_stop",
            errorMessage))
    {
        return false;
    }

    if (!requireSetting(
            values,
            "play_start",
            value,
            errorMessage)
        || !assignShortcut(
            value,
            settings.playStartKey,
            "play_start",
            errorMessage))
    {
        return false;
    }

    if (!requireSetting(
            values,
            "play_pause",
            value,
            errorMessage)
        || !assignShortcut(
            value,
            settings.playPauseKey,
            "play_pause",
            errorMessage))
    {
        return false;
    }

    if (!requireSetting(
            values,
            "play_cancel",
            value,
            errorMessage)
        || !assignShortcut(
            value,
            settings.playCancelKey,
            "play_cancel",
            errorMessage))
    {
        return false;
    }

    if (!requireSetting(
            values,
            "default_loops",
            value,
            errorMessage))
    {
        return false;
    }

    if (!parseLoopCount(
            value,
            settings.defaultLoops))
    {
        errorMessage =
            "default_loops must be a positive whole number.";

        return false;
    }

    errorMessage.clear();
    return true;
}

std::string keyNameFromVirtualKey(int virtualKey)
{
    if (virtualKey >= VK_F1
        && virtualKey <= VK_F12)
    {
        return "F"
            + std::to_string(
                virtualKey - VK_F1 + 1);
    }

    return "Unknown";
}