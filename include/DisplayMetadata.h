#pragma once

#include <string>
#include <vector>

struct MonitorMetadata
{
    std::string deviceName;

    int left = 0;
    int top = 0;
    int right = 0;
    int bottom = 0;

    int workLeft = 0;
    int workTop = 0;
    int workRight = 0;
    int workBottom = 0;

    bool primary = false;
};

struct DisplayMetadata
{
    int virtualDesktopLeft = 0;
    int virtualDesktopTop = 0;
    int virtualDesktopWidth = 0;
    int virtualDesktopHeight = 0;

    std::vector<MonitorMetadata> monitors;
};

enum class DisplayCompatibility
{
    Exact,
    CompatibleWithWarnings,
    Incompatible,
    Unknown
};

bool captureDisplayMetadata(
    DisplayMetadata& metadata,
    std::string& errorMessage);

DisplayCompatibility compareDisplayMetadata(
    const DisplayMetadata& recorded,
    const DisplayMetadata& current,
    std::string& compatibilityMessage);