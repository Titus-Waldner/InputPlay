#pragma once

#include "IInputBackend.h"
#include "PlaybackOptions.h"
#include "Settings.h"

#include <string>

int runPlayback(
    const std::string& filePath,
    IInputBackend& backend,
    const Settings& settings,
    const PlaybackOptions& options);
