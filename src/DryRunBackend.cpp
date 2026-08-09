#include "DryRunBackend.h"

#include <cstddef>
#include <iostream>
bool DryRunBackend::execute(
    const InputEvent& event,
    std::string& errorMessage)
{
    static std::size_t eventCount = 0;
    ++eventCount;

    if (eventCount == 1 || eventCount % 500 == 0)
    {
        std::cout
            << "Dry-run progress: "
            << eventCount
            << " events processed\n";
    }

    errorMessage.clear();
    return true;
}

void DryRunBackend::releaseAll()
{
    // Nothing to release in dry-run mode.
}