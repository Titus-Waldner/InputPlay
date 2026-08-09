#pragma once

#include "IInputBackend.h"

class SendInputBackend final : public IInputBackend
{
public:
    bool execute(
        const InputEvent& event,
        std::string& errorMessage) override;

    void releaseAll() override;
};