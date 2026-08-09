#pragma once

#include "IInputBackend.h"

class DryRunBackend final : public IInputBackend
{
public:
    bool execute(
        const InputEvent& event,
        std::string& errorMessage) override;

    void releaseAll() override;
};