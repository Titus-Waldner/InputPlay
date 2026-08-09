#include "DryRunBackend.h"

#include <iostream>

namespace
{
const char* eventTypeName(EventType type)
{
    switch (type)
    {
        case EventType::MouseMove:
            return "MouseMove";

        case EventType::MouseButtonDown:
            return "MouseButtonDown";

        case EventType::MouseButtonUp:
            return "MouseButtonUp";

        case EventType::MouseWheel:
            return "MouseWheel";

        case EventType::KeyDown:
            return "KeyDown";

        case EventType::KeyUp:
            return "KeyUp";

        default:
            return "Unknown";
    }
}
}

bool DryRunBackend::execute(
    const InputEvent& event,
    std::string& errorMessage)
{
    std::cout
        << "Playing "
        << eventTypeName(event.type)
        << " at "
        << event.timestampMicroseconds
        << " microseconds\n";

    errorMessage.clear();
    return true;
}

void DryRunBackend::releaseAll()
{
    std::cout << "Dry-run backend released all inputs\n";
}