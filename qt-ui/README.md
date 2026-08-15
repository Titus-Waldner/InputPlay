# L33T R3PL4Y

**Record. Edit. Replay.**

L33T R3PL4Y is a Windows application for recording, editing, and
replaying mouse and keyboard input macros.

The application is powered by InputPlayCore and uses the portable
`.irec` macro format.

## Features

- Dedicated Playback & Editing and Recording workspaces
- Mouse and keyboard input recording
- Recording countdown, pause, resume, stop, and cancellation
- List and timeline event views
- Event creation, editing, deletion, duplication, and reordering
- Undo and redo history
- Event search and filtering
- Adjustable playback speed
- Finite, custom, and infinite playback loops
- Dry Run mode for safe playback testing
- Confirmation before real input playback
- Align Start cursor positioning
- Display compatibility information
- System tray integration
- Global active-workspace hotkeys
- Dark, gray, and light themes with selectable accent colors

## Active Workspace Hotkeys

The global hotkeys control whichever workspace is currently open.

- **F9** — Start or stop
- **F10** — Pause or resume
- **Ctrl+Shift+Escape** — Emergency Stop

Workspace switching is blocked while recording or playback is active,
keeping the hotkey behavior predictable.

## Playback Safety

### Dry Run

Dry Run simulates playback without sending real mouse or keyboard
input to Windows.

### Real Playback Confirmation

L33T R3PL4Y can display a confirmation before real input playback
begins.

### Align Start

Align Start moves the cursor to the position captured at the beginning
of the recording before replaying the first event.

## Macro Files

L33T R3PL4Y uses the `.irec` macro format.

Macros can be:

- Created through input recording
- Opened and saved from the File menu
- Edited event by event
- Dragged onto the application window
- Exported as CSV, TSV, or JSON

## Building

### Requirements

- Windows 10 or Windows 11
- Qt 6
- CMake 3.20 or newer
- Ninja
- C++20-compatible compiler
- InputPlayCore built in the parent project directory

The current development environment uses MSYS2 UCRT64 with MinGW-w64.

### Build InputPlayCore

From the repository root:

```bash
cmake -S . -B build -G Ninja
cmake --build build