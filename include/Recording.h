#pragma once

#include "DisplayMetadata.h"
#include "InputEvent.h"

#include <cstddef>
#include <vector>

class Recording
{
public:
    void addEvent(const InputEvent& event);
    void clear();

    void setStartingCursorPosition(int x, int y);

    [[nodiscard]] int startingCursorX() const;
    [[nodiscard]] int startingCursorY() const;
    [[nodiscard]] bool hasStartingCursorPosition() const;

    void setDisplayMetadata(
        const DisplayMetadata& metadata);

    [[nodiscard]] const DisplayMetadata&
    displayMetadata() const;

    [[nodiscard]] bool hasDisplayMetadata() const;

    [[nodiscard]] std::size_t eventCount() const;
    [[nodiscard]] bool empty() const;
    [[nodiscard]] const std::vector<InputEvent>& events() const;

private:
    std::vector<InputEvent> events_;

    int startingCursorX_ = 0;
    int startingCursorY_ = 0;
    bool hasStartingCursorPosition_ = false;

    DisplayMetadata displayMetadata_;
    bool hasDisplayMetadata_ = false;
};