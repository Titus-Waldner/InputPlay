#include "Recording.h"

void Recording::addEvent(const InputEvent& event)
{
    events_.push_back(event);
}

void Recording::clear()
{
    events_.clear();

    startingCursorX_ = 0;
    startingCursorY_ = 0;
    hasStartingCursorPosition_ = false;

    displayMetadata_ = DisplayMetadata{};
    hasDisplayMetadata_ = false;
}

void Recording::setStartingCursorPosition(int x, int y)
{
    startingCursorX_ = x;
    startingCursorY_ = y;
    hasStartingCursorPosition_ = true;
}

int Recording::startingCursorX() const
{
    return startingCursorX_;
}

int Recording::startingCursorY() const
{
    return startingCursorY_;
}

bool Recording::hasStartingCursorPosition() const
{
    return hasStartingCursorPosition_;
}
void Recording::setDisplayMetadata(
    const DisplayMetadata& metadata)
{
    displayMetadata_ = metadata;
    hasDisplayMetadata_ = true;
}

const DisplayMetadata& Recording::displayMetadata() const
{
    return displayMetadata_;
}

bool Recording::hasDisplayMetadata() const
{
    return hasDisplayMetadata_;
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