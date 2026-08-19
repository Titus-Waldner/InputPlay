# L33T R3PL4Y

**Record. Edit. Replay.**

L33T R3PL4Y is a Windows input recorder, macro editor, and playback application built with C++20 and Qt 6.

The application records mouse and keyboard activity into editable `.irec` files and replays the events using their original timing.

## Features

- Mouse and keyboard recording
- High-resolution event timestamps
- Editable event list and timeline
- Playback progress and timeline highlighting
- Pause, resume, stop, and emergency-stop controls
- Finite and infinite looping
- Starting-cursor alignment
- Display-configuration validation
- Dry Run mode
- Physical mouse blocking for Windows input modes
- Windows SendInput playback
- Optional Virtual HID playback
- Original InputPlay command-line interface included in releases

## Input Methods

L33T R3PL4Y provides four playback methods.

### Windows - Exact Position

Replays recorded absolute cursor coordinates through Windows `SendInput`.

This mode provides the highest desktop-position accuracy and prevents accumulated cursor drift.

Keyboard events are also submitted through `SendInput`.

### Windows - Corrected Relative

Replays recorded relative mouse movement through Windows `SendInput`.

The application compares the live cursor position with the recorded path and corrects positional drift during playback.

Keyboard events are submitted through `SendInput`.

### Virtual HID - Corrected Relative

Replays relative mouse reports through the Generic Virtual Input Device.

The application checks the live cursor position and corrects accumulated positional drift while continuing to submit relative HID movement.

Keyboard events are submitted through the Virtual HID keyboard.

### Virtual HID - Exact Position

Replays recorded absolute cursor coordinates through the Generic Virtual Input Device.

Absolute Virtual HID coordinates currently apply to the primary monitor.

Keyboard events are submitted through the Virtual HID keyboard.

## Virtual HID Driver

The Virtual HID input methods require the separate **DriverLevelInputSimulator** project:

https://github.com/Titus-Waldner/DriverLevelInputSimulator

Install and verify the Generic Virtual Input Device before selecting either Virtual HID method.

The current Virtual HID driver is a development and test build. Review the driver repository’s installation instructions and safety notes before installation.

The current test-signed driver requires:

- 64-bit Windows 10 or later
- Administrator privileges for installation
- Secure Boot disabled
- Windows Test Mode enabled
- A restart after enabling Test Mode

The normal Windows input methods do not require the Virtual HID driver.

## Physical Mouse Blocking

The **Block physical mouse input** option prevents physical mouse activity from interfering with Windows-based playback.

Physical mouse blocking is automatically unavailable when a Virtual HID method is selected because the current low-level blocker cannot reliably distinguish the virtual mouse from a physical mouse.

F9 and Emergency Stop remain available during Windows protected playback.

## Hotkeys

The default workspace-dependent hotkeys are:

- `F9` — Start or stop recording/playback
- `F10` — Pause or resume recording/playback
- `Ctrl+Shift+Escape` — Emergency Stop

Only one copy of L33T R3PL4Y should be running at a time. An older instance, including one minimized to the system tray, may retain ownership of the global hotkeys.

## Recording Format

Recordings use the `.irec` format.

Each mouse-movement event stores:

- Event timestamp
- Absolute X and Y coordinates
- Relative X and Y movement
- Mouse-button state
- Wheel information

Storing both absolute positions and relative movement allows the same recording to be replayed through exact-position and corrected-relative input methods.

## Installation

Download the latest Windows x64 ZIP from the GitHub Releases page:

https://github.com/Titus-Waldner/L33T-R3PL4Y/releases

Extract the complete archive and keep all included files together.

Run:

```text
L33TR3PL4Y.exe