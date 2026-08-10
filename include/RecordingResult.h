#pragma once

#include <cstddef>
#include <cstdint>
#include <string>

enum class RecordingResultCode
{
    Completed,
    Cancelled,
    Failed
};

struct RecordingResult
{
    RecordingResultCode code =
        RecordingResultCode::Failed;

    std::string message;

    std::size_t eventCount = 0;
    std::uint64_t durationMicroseconds = 0;

    [[nodiscard]] bool succeeded() const
    {
        return code
            == RecordingResultCode::Completed;
    }
};