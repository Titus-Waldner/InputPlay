#pragma once

enum class InputBackendType
{
    SendInputAbsolute,
    SendInputCorrectedRelative,
    VhfCorrectedRelative,
    VhfAbsolute,
    VhfNativeRelative
};