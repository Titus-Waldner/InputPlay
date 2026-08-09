#include "DisplayMetadata.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <algorithm>
#include <cstddef>
#include <sstream>

namespace
{
BOOL CALLBACK monitorEnumerationCallback(
    HMONITOR monitorHandle,
    HDC,
    LPRECT,
    LPARAM userData)
{
    auto* metadata =
        reinterpret_cast<DisplayMetadata*>(userData);

    MONITORINFOEXA monitorInfo{};
    monitorInfo.cbSize = sizeof(monitorInfo);

    if (!GetMonitorInfoA(
            monitorHandle,
            &monitorInfo))
    {
        return FALSE;
    }

    MonitorMetadata monitor;
    monitor.deviceName = monitorInfo.szDevice;

    monitor.left = monitorInfo.rcMonitor.left;
    monitor.top = monitorInfo.rcMonitor.top;
    monitor.right = monitorInfo.rcMonitor.right;
    monitor.bottom = monitorInfo.rcMonitor.bottom;

    monitor.workLeft = monitorInfo.rcWork.left;
    monitor.workTop = monitorInfo.rcWork.top;
    monitor.workRight = monitorInfo.rcWork.right;
    monitor.workBottom = monitorInfo.rcWork.bottom;

    monitor.primary =
        (monitorInfo.dwFlags & MONITORINFOF_PRIMARY) != 0;

    metadata->monitors.push_back(monitor);

    return TRUE;
}

bool monitorGeometryMatches(
    const MonitorMetadata& first,
    const MonitorMetadata& second)
{
    return
        first.left == second.left
        && first.top == second.top
        && first.right == second.right
        && first.bottom == second.bottom
        && first.primary == second.primary;
}

bool monitorWorkAreaMatches(
    const MonitorMetadata& first,
    const MonitorMetadata& second)
{
    return
        first.workLeft == second.workLeft
        && first.workTop == second.workTop
        && first.workRight == second.workRight
        && first.workBottom == second.workBottom;
}

bool monitorSortOrder(
    const MonitorMetadata& first,
    const MonitorMetadata& second)
{
    if (first.left != second.left)
    {
        return first.left < second.left;
    }

    if (first.top != second.top)
    {
        return first.top < second.top;
    }

    return first.deviceName < second.deviceName;
}

void sortMonitors(DisplayMetadata& metadata)
{
    std::sort(
        metadata.monitors.begin(),
        metadata.monitors.end(),
        monitorSortOrder);
}
}

bool captureDisplayMetadata(
    DisplayMetadata& metadata,
    std::string& errorMessage)
{
    metadata = DisplayMetadata{};

    metadata.virtualDesktopLeft =
        GetSystemMetrics(SM_XVIRTUALSCREEN);

    metadata.virtualDesktopTop =
        GetSystemMetrics(SM_YVIRTUALSCREEN);

    metadata.virtualDesktopWidth =
        GetSystemMetrics(SM_CXVIRTUALSCREEN);

    metadata.virtualDesktopHeight =
        GetSystemMetrics(SM_CYVIRTUALSCREEN);

    if (!EnumDisplayMonitors(
            nullptr,
            nullptr,
            monitorEnumerationCallback,
            reinterpret_cast<LPARAM>(&metadata)))
    {
        errorMessage =
            "Windows was unable to enumerate the monitors.";

        return false;
    }

    if (metadata.monitors.empty())
    {
        errorMessage =
            "Windows did not report any active monitors.";

        return false;
    }

    sortMonitors(metadata);

    errorMessage.clear();
    return true;
}

DisplayCompatibility compareDisplayMetadata(
    const DisplayMetadata& recorded,
    const DisplayMetadata& current,
    std::string& compatibilityMessage)
{
    if (recorded.monitors.empty())
    {
        compatibilityMessage =
            "The recording does not contain display metadata.";

        return DisplayCompatibility::Unknown;
    }

    if (recorded.monitors.size()
        != current.monitors.size())
    {
        std::ostringstream message;

        message
            << "Monitor count differs. Recorded: "
            << recorded.monitors.size()
            << ", current: "
            << current.monitors.size()
            << ".";

        compatibilityMessage = message.str();

        return DisplayCompatibility::Incompatible;
    }

    if (recorded.virtualDesktopLeft
            != current.virtualDesktopLeft
        || recorded.virtualDesktopTop
            != current.virtualDesktopTop
        || recorded.virtualDesktopWidth
            != current.virtualDesktopWidth
        || recorded.virtualDesktopHeight
            != current.virtualDesktopHeight)
    {
        std::ostringstream message;

        message
            << "Virtual desktop geometry differs. Recorded: "
            << recorded.virtualDesktopLeft
            << ","
            << recorded.virtualDesktopTop
            << " "
            << recorded.virtualDesktopWidth
            << "x"
            << recorded.virtualDesktopHeight
            << "; current: "
            << current.virtualDesktopLeft
            << ","
            << current.virtualDesktopTop
            << " "
            << current.virtualDesktopWidth
            << "x"
            << current.virtualDesktopHeight
            << ".";

        compatibilityMessage = message.str();

        return DisplayCompatibility::Incompatible;
    }

    bool workAreaDifference = false;
    bool deviceNameDifference = false;

    for (std::size_t index = 0;
         index < recorded.monitors.size();
         ++index)
    {
        const MonitorMetadata& recordedMonitor =
            recorded.monitors[index];

        const MonitorMetadata& currentMonitor =
            current.monitors[index];

        if (!monitorGeometryMatches(
                recordedMonitor,
                currentMonitor))
        {
            std::ostringstream message;

            message
                << "Monitor geometry differs at monitor "
                << index + 1
                << ".";

            compatibilityMessage = message.str();

            return DisplayCompatibility::Incompatible;
        }

        if (!monitorWorkAreaMatches(
                recordedMonitor,
                currentMonitor))
        {
            workAreaDifference = true;
        }

        if (recordedMonitor.deviceName
            != currentMonitor.deviceName)
        {
            deviceNameDifference = true;
        }
    }

    if (workAreaDifference || deviceNameDifference)
    {
        compatibilityMessage =
            "Monitor geometry matches, but monitor names "
            "or work areas differ.";

        return DisplayCompatibility::CompatibleWithWarnings;
    }

    compatibilityMessage =
        "Current display configuration exactly matches "
        "the recording.";

    return DisplayCompatibility::Exact;
}