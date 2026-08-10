# InputPlay

InputPlay is a Windows command-line mouse and keyboard recording and playback tool.

InputPlay records:

- Relative mouse movement
- Mouse button presses and releases
- Vertical mouse-wheel input
- Keyboard key presses and releases
- Event timing
- Cursor position when recording begins
- Cursor teleport events after recording resumes
- Monitor arrangement and virtual-desktop geometry

Recordings are stored as readable `.irec` files.

## Package Contents

```text
InputPlay/
├── InputPlay.exe
├── settings.config
├── README.md
└── recordings/
    └── example.irec
```

InputPlay is statically linked and does not require MSYS2, MinGW, or additional runtime DLLs.

## Quick Start

Open PowerShell, Command Prompt, or Windows Terminal in the InputPlay folder.

Display help:

```powershell
.\InputPlay.exe --help
```

Inspect the example recording:

```powershell
.\InputPlay.exe info .\recordings\example.irec
```

Validate the example recording:

```powershell
.\InputPlay.exe validate .\recordings\example.irec
```

Perform a safe dry run:

```powershell
.\InputPlay.exe play .\recordings\example.irec
```

A dry run processes event timing without sending mouse or keyboard input.

## Commands

```text
record <file>
play <file> [options]
pause <session>
resume <session>
cancel <session>
info <file>
validate <file>
test-model
exit-codes
```

Display the current command list:

```powershell
.\InputPlay.exe --help
```

## Recording

Start a new recording:

```powershell
.\InputPlay.exe record .\recordings\my-recording.irec
```

Default recording controls:

```text
F9   Start recording
F10  Pause or resume recording
F12  Stop and save
```

Before recording starts, pressing the stop key cancels the operation and returns exit code `4`.

While recording is paused:

- Mouse and keyboard events are ignored.
- Paused time is removed from the recorded timeline.
- Resuming inserts a cursor teleport event at the current cursor position.
- The start, pause, and stop control keys are excluded from the recording.

InputPlay can capture mouse and keyboard input together. The reusable core API also supports mouse-only and keyboard-only recording modes.

## Recording Information

Display a recording summary:

```powershell
.\InputPlay.exe info .\recordings\my-recording.irec
```

The summary includes:

- Event count
- Recording duration
- Starting cursor position
- Monitor count
- Virtual desktop dimensions
- Individual monitor positions and resolutions
- Mouse movement event count
- Mouse teleport event count
- Mouse button event count
- Mouse wheel event count
- Keyboard event count
- Wait event count
- Current display compatibility

## Recording Validation

Validate a recording:

```powershell
.\InputPlay.exe validate .\recordings\my-recording.irec
```

Validation checks include:

- Timestamp ordering
- Keyboard key-down and key-up balance
- Mouse button-down and button-up balance
- Valid monitor geometry
- Valid starting cursor position
- Valid mouse teleport coordinates
- Unsupported mouse buttons
- Empty or zero-distance events
- Keys or mouse buttons still held at the end

Warnings do not make a recording invalid. Structural errors return exit code `7`.

## Dry-Run Playback

Dry-run playback processes event timing without sending real input:

```powershell
.\InputPlay.exe play .\recordings\my-recording.irec
```

Default playback controls:

```text
F9   Start playback
F10  Pause or resume
F12  Cancel
```

Dry-run mode is the default. Live input requires the explicit `--send-input` option.

## Live Playback

To send real mouse and keyboard input, specify `--send-input`:

```powershell
.\InputPlay.exe play .\recordings\my-recording.irec --send-input
```

Live playback can:

- Move the pointer
- Click mouse buttons
- Scroll the mouse wheel
- Type keyboard input
- Trigger keyboard shortcuts

Test recordings in dry-run mode before enabling live playback.

## Align Cursor to Recorded Start

Move the cursor to its recorded starting coordinate before each loop:

```powershell
.\InputPlay.exe play .\recordings\my-recording.irec `
    --send-input `
    --align-start
```

The cursor is aligned at the beginning of every loop, helping prevent accumulated positional drift.

Older recordings may not contain a starting cursor position.

## Start Immediately

Skip the F9 start prompt:

```powershell
.\InputPlay.exe play .\recordings\my-recording.irec `
    --start-immediately
```

This option is recommended for:

- PowerShell scripts
- Python programs
- Scheduled tasks
- Continuous integration
- Other automated workflows

## Playback Loops

Play a recording three times:

```powershell
.\InputPlay.exe play .\recordings\my-recording.irec `
    --loops 3
```

Loop indefinitely:

```powershell
.\InputPlay.exe play .\recordings\my-recording.irec `
    --loops inf
```

Infinite playback should normally be paired with a timeout or named cancellation session.

## Playback Timeout

Stop playback automatically after a specified number of seconds:

```powershell
.\InputPlay.exe play .\recordings\my-recording.irec `
    --loops inf `
    --start-immediately `
    --timeout 120
```

A timeout:

- Stops scheduling events
- Releases held keyboard keys
- Releases held mouse buttons
- Returns exit code `5`

The timeout applies to the complete playback operation, including all loops and time spent paused.

## Named Playback Sessions

Named sessions allow another InputPlay process, PowerShell script, or Python program to control a running playback operation.

Start a named blocking playback session:

```powershell
.\InputPlay.exe play .\recordings\my-recording.irec `
    --send-input `
    --align-start `
    --start-immediately `
    --loops inf `
    --session report-run
```

The original process remains blocked while playback runs.

### Pause a Named Session

From another terminal or process:

```powershell
.\InputPlay.exe pause report-run
```

The playback process pauses at its current logical position.

### Resume a Named Session

```powershell
.\InputPlay.exe resume report-run
```

Playback resumes without rushing through events whose timestamps passed while playback was paused.

### Cancel a Named Session

```powershell
.\InputPlay.exe cancel report-run
```

The original playback process:

- Stops scheduling events
- Releases held keyboard keys
- Releases held mouse buttons
- Returns exit code `4`

The original process remains blocked until playback completes, is cancelled, times out, or fails.

Session names may contain:

- Letters
- Numbers
- Hyphens
- Underscores

Session names may contain up to 64 characters.

Only one active playback session may use a particular name at a time.

## Display Compatibility

InputPlay records:

- Monitor count
- Monitor positions
- Monitor resolutions
- Monitor work areas
- Primary-monitor designation
- Virtual-desktop position and size

By default, InputPlay reports a warning when the current display setup differs from the recorded setup, then continues.

Require a compatible display setup:

```powershell
.\InputPlay.exe play .\recordings\my-recording.irec `
    --strict-display
```

Strict mode returns exit code `6` when compatibility cannot be established.

Skip display validation:

```powershell
.\InputPlay.exe play .\recordings\my-recording.irec `
    --ignore-display
```

The following options cannot be used together:

```text
--strict-display
--ignore-display
```

## Full Playback Example

```powershell
.\InputPlay.exe play .\recordings\my-recording.irec `
    --send-input `
    --align-start `
    --start-immediately `
    --loops inf `
    --session example-session `
    --timeout 120 `
    --strict-display
```

Another process can control that session:

```powershell
.\InputPlay.exe pause example-session
.\InputPlay.exe resume example-session
.\InputPlay.exe cancel example-session
```

## Settings

InputPlay loads `settings.config` from the same directory as `InputPlay.exe`.

If the file does not exist, InputPlay creates it automatically.

Default settings:

```ini
# InputPlay settings
# Supported shortcut names: F1 through F12

record_start=F9
record_pause=F10
record_stop=F12

play_start=F9
play_pause=F10
play_cancel=F12

default_loops=1
```

Supported shortcut values are `F1` through `F12`.

Restart InputPlay after modifying the settings file. Settings are loaded each time an InputPlay process starts.

The control keys are read through Windows keyboard state rather than reserved as exclusive system-wide hotkeys.

## Exit Codes

Display the exit-code list:

```powershell
.\InputPlay.exe exit-codes
```

Exit-code contract:

```text
0  Success
1  General failure
2  Invalid command-line arguments
3  Recording load or format failure
4  Operation cancelled
5  Operation timed out
6  Display incompatibility
7  Recording validation failure
```

These values are stable and intended for scripting integrations.

## PowerShell Examples

### Blocking Playback

```powershell
$arguments = @(
    "play"
    ".\recordings\example.irec"
    "--send-input"
    "--align-start"
    "--start-immediately"
    "--session"
    "powershell-test"
    "--timeout"
    "120"
)

$process = Start-Process `
    -FilePath ".\InputPlay.exe" `
    -ArgumentList $arguments `
    -PassThru `
    -Wait

Write-Host "InputPlay exit code: $($process.ExitCode)"
```

### Start Playback Without Waiting

```powershell
$arguments = @(
    "play"
    ".\recordings\example.irec"
    "--start-immediately"
    "--loops"
    "inf"
    "--session"
    "powershell-test"
    "--timeout"
    "120"
)

$process = Start-Process `
    -FilePath ".\InputPlay.exe" `
    -ArgumentList $arguments `
    -PassThru

Write-Host "InputPlay process ID: $($process.Id)"
```

### Control the Session

```powershell
.\InputPlay.exe pause powershell-test
.\InputPlay.exe resume powershell-test
.\InputPlay.exe cancel powershell-test
```

### Wait and Read the Exit Code

```powershell
$process.WaitForExit()

Write-Host "InputPlay exit code: $($process.ExitCode)"
```

## Python Examples

### Blocking Playback

```python
import subprocess
from pathlib import Path

inputplay = Path("InputPlay.exe")
recording = Path("recordings") / "example.irec"

command = [
    str(inputplay),
    "play",
    str(recording),
    "--send-input",
    "--align-start",
    "--start-immediately",
    "--session",
    "python-test",
    "--timeout",
    "120",
]

result = subprocess.run(
    command,
    check=False,
)

if result.returncode == 0:
    print("Playback completed.")
elif result.returncode == 4:
    print("Playback was cancelled.")
elif result.returncode == 5:
    print("Playback timed out.")
else:
    print(
        f"Playback failed with exit code "
        f"{result.returncode}."
    )
```

### Start Playback Asynchronously

```python
import subprocess
from pathlib import Path

inputplay = Path("InputPlay.exe")
recording = Path("recordings") / "example.irec"

process = subprocess.Popen(
    [
        str(inputplay),
        "play",
        str(recording),
        "--start-immediately",
        "--loops",
        "inf",
        "--session",
        "python-test",
        "--timeout",
        "120",
    ]
)

print(f"InputPlay process ID: {process.pid}")
```

### Pause, Resume, and Cancel

```python
import subprocess

subprocess.run(
    ["InputPlay.exe", "pause", "python-test"],
    check=False,
)

subprocess.run(
    ["InputPlay.exe", "resume", "python-test"],
    check=False,
)

subprocess.run(
    ["InputPlay.exe", "cancel", "python-test"],
    check=False,
)
```

### Wait for Completion

```python
exit_code = process.wait()

print(f"InputPlay exit code: {exit_code}")
```

## Architecture

InputPlay separates its reusable core from the command-line interface.

```text
InputPlayCore
├── Recording model and file format
├── Recording validation
├── Recording summaries
├── Input recording engine
├── Playback engine
├── Thread-safe recording controller
├── Thread-safe playback controller
├── Structured recording options
├── Structured playback options
├── Structured recording results
├── Structured playback results
├── Structured progress callbacks
├── Display compatibility
├── Named session controls
├── Settings model and persistence
└── Windows input backends

InputPlay CLI
├── Command-line parsing
├── Terminal presentation
├── Runtime option construction
├── Settings-file loading
└── Process exit-code mapping
```

The recording and playback engines do not write directly to the terminal.

Instead, the core reports structured states and progress information through callbacks. This allows the same core to support:

- The InputPlay command-line interface
- PowerShell and Python automation
- Automated tests
- A future Qt graphical interface

Terminal formatting and process exit codes remain responsibilities of the CLI front end.

## Building from Source

### Requirements

- Windows
- MSYS2 UCRT64
- GCC with C++20 support
- CMake
- Ninja

Configure a development build:

```bash
cmake -S . -B build \
    -G Ninja
```

Build:

```bash
cmake --build build
```

Run the CLI:

```bash
./build/InputPlay.exe --help
```

### Release Build

Configure an optimized Release build:

```bash
cmake -S . -B build-release \
    -G Ninja \
    -DCMAKE_BUILD_TYPE=Release
```

Build:

```bash
cmake --build build-release
```

The MinGW runtime libraries are statically linked into the Release executable.

## Automated Tests

Configure and build the project:

```bash
cmake -S . -B build \
    -G Ninja
cmake --build build
```

Run the automated tests:

```bash
cd build
ctest --output-on-failure
```

Alternatively, run the test executable directly:

```bash
./build/InputPlayCoreTests.exe
```

The test suite covers:

- Recording summaries
- Recording validation
- Test backend execution
- Input cleanup
- Playback event order
- Multiple playback loops
- Structured progress states
- Simulated backend failures
- Timeout behavior
- Pre-start cancellation
- In-process cancellation
- Cross-thread cancellation
- Cross-thread pause and resume
- Pause-duration timing compensation

The automated tests use a test backend and do not send real mouse or keyboard input.

## Safety Notes

Before live playback:

1. Validate the recording.
2. Check display compatibility.
3. Use a harmless test application first.
4. Consider specifying a timeout.
5. Use a named session for externally controllable automation.
6. Keep the configured cancel shortcut available.
7. Avoid running untrusted `.irec` files.

InputPlay releases tracked keyboard keys and mouse buttons when playback:

- Completes
- Fails
- Is cancelled
- Times out

## Troubleshooting

### Recording Does Not Start

Confirm the configured `record_start` key in `settings.config`.

Press and release the configured key after this message appears:

```text
Recording armed
```

### Recording Cancels Before Starting

The configured `record_stop` key was detected before the start key.

Run the recording command again and press the configured start key. Deliberate pre-start cancellation returns exit code `4`.

### Playback Does Not Send Input

Live input requires:

```text
--send-input
```

Without that option, InputPlay runs in safe dry-run mode.

### Cursor Alignment Fails

The recording may not contain a starting cursor position, or the recorded coordinate may no longer exist in the current virtual desktop.

Inspect the file:

```powershell
.\InputPlay.exe info .\recordings\my-recording.irec
```

### Display Compatibility Warning

The monitor arrangement, resolution, work area, or primary monitor differs from the recording environment.

Use `info` to compare the recording with the current system:

```powershell
.\InputPlay.exe info .\recordings\my-recording.irec
```

### Named Session Cannot Be Created

Another active InputPlay process may already use the same session name.

Choose a different session name or cancel the existing session:

```powershell
.\InputPlay.exe cancel existing-session
```

### Pause or Resume Cannot Find a Session

The requested session may have already completed, timed out, failed, or been cancelled.

Confirm that the playback process is still active and that the session name matches exactly.

### Function-Key Text Appears in a Terminal

InputPlay reads configured function keys without registering exclusive global shortcuts.

Depending on the terminal, a function-key sequence may remain queued after InputPlay exits. This does not affect the recording or playback result.

For fully scripted playback, use:

```text
--start-immediately
```

### Settings File Is Recreated

InputPlay creates a default `settings.config` next to the executable when the file is missing.

To restore custom settings, edit the newly created file or replace it with a backup.

## Version Compatibility

InputPlay continues to load supported earlier `.irec` recording versions.

Newer recordings may contain additional metadata, including:

- Starting cursor position
- Monitor geometry
- Cursor teleport events

Use the `info` and `validate` commands to inspect older recordings before live playback.

## License and Warranty

InputPlay is provided without warranty.

Test recordings carefully before using live playback in important workflows.