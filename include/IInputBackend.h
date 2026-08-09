#pragma once

#include "InputEvent.h"

#include <string>

class IInputBackend
{
public:
    virtual ~IInputBackend() = default;

    virtual bool execute(
        const InputEvent& event,
        std::string& errorMessage) = 0;

    virtual void releaseAll() = 0;
};