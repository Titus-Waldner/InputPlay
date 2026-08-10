#pragma once

#include "Recording.h"
#include "RecordingOptions.h"
#include "RecordingResult.h"

#include <string>

class InputRecorder
{
public:
    RecordingResult record(
        Recording& recording,
        const RecordingOptions& options);
};