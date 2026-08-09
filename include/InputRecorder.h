#pragma once

#include "Recording.h"
#include "Settings.h"

#include <string>

class InputRecorder
{
public:
    bool record(
        Recording& recording,
        const Settings& settings,
        std::string& errorMessage);
};