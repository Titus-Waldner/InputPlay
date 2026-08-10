#pragma once

#include "Settings.h"

#include <string>

bool loadOrCreateSettings(
    Settings& settings,
    std::string& settingsPath,
    std::string& errorMessage);