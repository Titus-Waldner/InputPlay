#include "TestInputBackend.h"

bool TestInputBackend::execute(
    const InputEvent& event,
    std::string& errorMessage)
{
    if (failureEnabled_
        && executedEvents_.size()
            >= failureAfter_)
    {
        errorMessage =
            failureMessage_;

        return false;
    }

    executedEvents_.push_back(
        event);

    errorMessage.clear();
    return true;
}

void TestInputBackend::releaseAll()
{
    ++releaseCount_;
}

void TestInputBackend::setFailureAfter(
    std::size_t eventCount,
    const std::string& errorMessage)
{
    failureEnabled_ = true;
    failureAfter_ = eventCount;
    failureMessage_ = errorMessage;
}

void TestInputBackend::clearFailure()
{
    failureEnabled_ = false;
    failureAfter_ = 0;
    failureMessage_.clear();
}

const std::vector<InputEvent>&
TestInputBackend::executedEvents() const
{
    return executedEvents_;
}

std::size_t TestInputBackend::releaseCount() const
{
    return releaseCount_;
}