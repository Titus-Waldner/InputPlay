#pragma once

class PhysicalMouseBlocker
{
public:
    PhysicalMouseBlocker() = delete;

    static bool enable();

    static void disable();

    static bool isEnabled();
};