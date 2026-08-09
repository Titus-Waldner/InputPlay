#pragma once

#include "InputEvent.h"

#include <cstddef>
#include <vector>

class Recording
{
public:
    void addEvent(const InputEvent& event);
    void clear();

    [[nodiscard]] std::size_t eventCount() const;
    [[nodiscard]] bool empty() const;
    [[nodiscard]] const std::vector<InputEvent>& events() const;

private:
    std::vector<InputEvent> events_;
};
