#pragma once

#include "IInputBackend.h"
#include "InputBackendType.h"

#include <array>
#include <unordered_set>

class VhfInputBackend final
    : public IInputBackend
{
public:
    explicit VhfInputBackend(
        InputBackendType backendType);

    ~VhfInputBackend() override;

    VhfInputBackend(
        const VhfInputBackend&) = delete;

    VhfInputBackend& operator=(
        const VhfInputBackend&) = delete;

    bool open(
        std::string& errorMessage);

    bool execute(
        const InputEvent& event,
        std::string& errorMessage) override;

    void releaseAll() override;

private:
    bool driverIsOpen() const;

    void closeDriver();

    bool submitMouseCommand(
        int movementX,
        int movementY,
        int verticalWheel,
        int horizontalWheel,
        std::string& errorMessage);

	bool submitAbsolutePosition(
		int screenX,
		int screenY,
		std::string& errorMessage);

    bool sendMouseMovement(
        const InputEvent& event,
        std::string& errorMessage);
		
	bool submitRelativeMovement(
		int movementX,
		int movementY,
		std::string& errorMessage);

    bool prepareMousePosition(
        const InputEvent& event,
        std::string& errorMessage);

	bool updateMouseButton(
		int mouseButton,
		bool pressed,
		int screenX,
		int screenY,
		std::string& errorMessage);

    bool submitKeyboardState(
        std::string& errorMessage);

    bool updateKeyboardState(
        unsigned int packedScanCode,
        bool pressed,
        std::string& errorMessage);

    bool translateScanCode(
        unsigned int packedScanCode,
        unsigned char& usage,
        unsigned char& modifierMask) const;

    void* deviceHandle_ =
        nullptr;

    InputBackendType backendType_ =
        InputBackendType::VhfCorrectedRelative;

    unsigned char mouseButtons_ =
        0;

    unsigned char keyboardModifiers_ =
        0;

    std::array<unsigned char, 6> keyboardUsages_ =
        {};

    std::unordered_set<unsigned int>
        heldScanCodes_;

    int previousRecordedX_ =
        0;

    int previousRecordedY_ =
        0;

    bool hasPreviousRecordedPosition_ =
        false;
};