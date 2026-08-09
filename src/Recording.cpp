#include "Recording.h"

void Recording::addEvent(const InputEvent& event)
{
    events_.push_back(event);
}

void Recording::clear()
{
    events_.clear();
}

std::size_t Recording::eventCount() const
{
    return events_.size();
}

bool Recording::empty() const
{
    return events_.empty();
}

const std::vector<InputEvent>& Recording::events() const
{
    return events_;
}

