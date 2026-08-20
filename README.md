# L33T R3PL4Y

**Record. Edit. Replay.**

L33T R3PL4Y is a Windows mouse and keyboard recorder, macro editor, and playback application built with C++20 and Qt 6.

Recordings are saved as editable `.irec` files and can be replayed through Windows input APIs or an optional Virtual HID device.

## Features

- Mouse and keyboard recording
- High-resolution event timing
- Editable event list and timeline
- Continue Recording for appending to an existing recording
- Automatic cursor restoration when continuing a recording
- Pause, resume, stop, and emergency stop
- Finite and infinite playback loops
- Starting-cursor alignment
- Display compatibility validation
- Dry Run mode
- Physical mouse blocking for Windows input modes
- Windows `SendInput` playback
- Optional Virtual HID mouse and keyboard playback
- Command-line interface included in Windows releases

## Input Methods

L33T R3PL4Y provides five playback methods.

### Windows - Exact Position

Replays recorded absolute cursor coordinates through Windows `SendInput`.

This mode provides accurate desktop positioning and prevents accumulated cursor drift. Keyboard input is also submitted through `SendInput`.

### Windows - Corrected Relative

Replays recorded relative mouse movement through Windows `SendInput` while using the recorded cursor path to correct positional drift.

Keyboard input is submitted through `SendInput`.

### Virtual HID - Native Relative

Replays the original recorded mouse deltas through the Virtual HID device without positional feedback correction.

This is the recommended Virtual HID relative mode and most closely resembles input from a conventional relative mouse.

### Virtual HID - Corrected Relative (Experimental)

Replays relative mouse movement through the Virtual HID device with positional feedback correction.

This mode is experimental and may oscillate on some systems. Use **Virtual HID - Native Relative** for stable relative playback.

### Virtual HID - Exact Position

Replays recorded absolute cursor coordinates through the Virtual HID device.

Absolute Virtual HID positioning currently applies to the primary monitor.

All Virtual HID modes submit keyboard events through the Virtual HID keyboard.

## Continue Recording

The Recording workspace includes a **Continue Recording** option.

Continue Recording:

- Works with newly created and loaded `.irec` files
- Moves the cursor to the final recorded mouse position
- Appends newly captured events to the existing recording
- Continues timestamps from the end of the existing recording
- Preserves the original starting position and display metadata
- Leaves the existing recording unchanged if recording is cancelled

## Virtual HID Driver

Virtual HID playback requires the separate **DriverLevelInputSimulator** project:

https://github.com/Titus-Waldner/DriverLevelInputSimulator

Install and verify the Generic Virtual Input Device before selecting a Virtual HID input method.

The current test-signed driver requires:

- 64-bit Windows 10 or later
- Administrator privileges for installation
- Secure Boot disabled
- Windows Test Mode enabled
- A restart after enabling Test Mode

Review the driver repository's installation and safety instructions before installing it.

The Windows input methods do not require the Virtual HID driver.

## Physical Mouse Blocking

**Block physical mouse input** prevents physical mouse activity from interfering with Windows-based playback.

This option is unavailable with Virtual HID playback because the current blocker cannot reliably distinguish the virtual mouse from a physical mouse.

## Hotkeys

Default workspace-dependent hotkeys:

- `F9` — Start or stop recording/playback
- `F10` — Pause or resume recording/playback
- `Ctrl+Shift+Escape` — Emergency Stop

Only one copy of L33T R3PL4Y should run at a time. An older instance minimized to the system tray may retain ownership of the global hotkeys.

## Recording Format

Recordings use the `.irec` format.

Mouse events can store:

- Timestamp
- Absolute X and Y coordinates
- Relative X and Y movement
- Mouse-button state
- Wheel movement

Storing both absolute positions and relative movement allows the same recording to be used with exact-position and relative playback methods.

## Installation

Download the latest Windows x64 ZIP from:

https://github.com/Titus-Waldner/L33T-R3PL4Y/releases

Extract the complete archive, keep all included files together, and run:

```text
L33TR3PL4Y.exe