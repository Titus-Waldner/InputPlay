#pragma once

#include "Recording.h"

#include <string>

class MouseRecorder
{
public:
    bool record(
        Recording& recording,
        std::string& errorMessage);
};