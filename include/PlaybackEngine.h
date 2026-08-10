#pragma once

#include "IInputBackend.h"
#include "PlaybackOptions.h"
#include "PlaybackResult.h"
#include "Settings.h"

#include <string>

PlaybackResult runPlayback(
    const std::string& filePath,
    IInputBackend& backend,
    const Settings& settings,
    const PlaybackOptions& options);