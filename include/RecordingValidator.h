#pragma once

#include "Recording.h"

#include <cstddef>
#include <string>
#include <vector>

struct ValidationResult
{
    std::vector<std::string> errors;
    std::vector<std::string> warnings;

    [[nodiscard]] bool valid() const
    {
        return errors.empty();
    }

    [[nodiscard]] std::size_t errorCount() const
    {
        return errors.size();
    }

    [[nodiscard]] std::size_t warningCount() const
    {
        return warnings.size();
    }
};

ValidationResult validateRecording(
    const Recording& recording);