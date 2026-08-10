#pragma once

#include "Recording.h"
#include "RecordingController.h"
#include "RecordingOptions.h"
#include "RecordingResult.h"

class InputRecorder
{
public:
    RecordingResult record(
        Recording& recording,
        const RecordingOptions& options,
        RecordingController& controller);
};