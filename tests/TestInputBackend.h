#pragma once

#include "IInputBackend.h"

#include <cstddef>
#include <string>
#include <vector>

class TestInputBackend final
    : public IInputBackend
{
public:
    bool execute(
        const InputEvent& event,
        std::string& errorMessage) override;

    void releaseAll() override;

    void setFailureAfter(
        std::size_t eventCount,
        const std::string& errorMessage);

    void clearFailure();

    [[nodiscard]]
    const std::vector<InputEvent>& executedEvents() const;

    [[nodiscard]]
    std::size_t releaseCount() const;

private:
    std::vector<InputEvent> executedEvents_;

    bool failureEnabled_ = false;
    std::size_t failureAfter_ = 0;

    std::string failureMessage_;

    std::size_t releaseCount_ = 0;
};