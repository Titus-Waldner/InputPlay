#pragma once

#include "IInputBackend.h"

#include <unordered_set>

class SendInputBackend final : public IInputBackend
{
public:
    bool execute(
        const InputEvent& event,
        std::string& errorMessage) override;

    void releaseAll() override;

private:
    bool leftButtonHeld_ = false;
    bool rightButtonHeld_ = false;
    bool middleButtonHeld_ = false;

    std::unordered_set<unsigned int> heldKeys_;
};