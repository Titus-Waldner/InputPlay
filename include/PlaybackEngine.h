#pragma once

#include "IInputBackend.h"
#include "PlaybackController.h"
#include "PlaybackOptions.h"
#include "PlaybackProgress.h"
#include "PlaybackResult.h"
#include "Settings.h"

#include <string>

PlaybackResult runPlayback(
    const std::string& filePath,
    IInputBackend& backend,
    const Settings& settings,
    const PlaybackOptions& options,
    PlaybackController& controller,
    const PlaybackCallbacks& callbacks);