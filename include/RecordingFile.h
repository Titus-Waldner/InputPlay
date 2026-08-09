#pragma once

#include "Recording.h"

#include <string>

class RecordingFile
{
public:
    static bool save(
        const Recording& recording,
        const std::string& filePath,
        std::string& errorMessage);

    static bool load(
        const std::string& filePath,
        Recording& recording,
        std::string& errorMessage);
};