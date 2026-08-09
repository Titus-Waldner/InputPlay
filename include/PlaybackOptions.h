#pragma once

#include <string>

struct PlaybackOptions
{
    bool alignStart = false;

    bool strictDisplay = false;
    bool ignoreDisplay = false;

    bool startImmediately = false;

    unsigned int loopCount = 1;
    bool infiniteLoops = false;

    std::string sessionName;

    bool timeoutEnabled = false;
    unsigned int timeoutSeconds = 0;
};