#pragma once

#include "IInputBackend.h"
#include "InputBackendType.h"

#include <unordered_set>

class SendInputBackend final
    : public IInputBackend
{
public:
    SendInputBackend();

    explicit SendInputBackend(
        InputBackendType backendType);

    bool execute(
        const InputEvent& event,
        std::string& errorMessage) override;

    void releaseAll() override;

private:
    bool sendCorrectedRelativeMove(
        const InputEvent& event,
        std::string& errorMessage);

    bool correctToPosition(
        int destinationX,
        int destinationY,
        std::string& errorMessage);

    bool prepareMousePosition(
        const InputEvent& event,
        std::string& errorMessage);

    InputBackendType backendType_ =
        InputBackendType::SendInputAbsolute;

    bool leftButtonHeld_ =
        false;

    bool rightButtonHeld_ =
        false;

    bool middleButtonHeld_ =
        false;

    int previousRecordedX_ =
        0;

    int previousRecordedY_ =
        0;

    bool hasPreviousRecordedPosition_ =
        false;

    std::unordered_set<unsigned int> heldKeys_;
};