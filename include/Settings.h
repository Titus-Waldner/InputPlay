#pragma once

#include <string>

struct Settings
{
    int recordStartKey = 0;
    int recordPauseKey = 0;
    int recordStopKey = 0;

    int playStartKey = 0;
    int playPauseKey = 0;
    int playCancelKey = 0;

    unsigned int defaultLoops = 1;
};

std::string keyNameFromVirtualKey(
    int virtualKey);