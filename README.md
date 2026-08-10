# InputPlay

InputPlay is a Windows command-line mouse and keyboard recorder and playback tool.

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

Before recording starts, pressing the stop key cancels the operation.

While recording is paused:

- Mouse and keyboard events are ignored.
- Paused time is removed from the recorded timeline.
- Resuming inserts a cursor teleport event at the current cursor position.
- The start, pause, and stop control keys are excluded from the recording.

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

Dry-run playback does not send input:

```powershell
.\InputPlay.exe play .\recordings\my-recording.irec
```

Default playback controls:

```text
F9   Start playback
F10  Pause or resume
F12  Cancel
```

## Live Playback

To send real mouse and keyboard input, specify `--send-input`:

```powershell
.\InputPlay.exe play .\recordings\my-recording.irec --send-input
```

Live playback should be tested carefully because it can move the pointer, click items, type text, and use keyboard shortcuts.

## Align Cursor to Recorded Start

Move the cursor to its recorded starting coordinate before each loop:

```powershell
.\InputPlay.exe play .\recordings\my-recording.irec `
    --send-input `
    --align-start
```

The cursor is aligned at the beginning of every loop, preventing accumulated positional drift.

Older recordings may not contain a starting cursor position.

## Start Immediately

Skip the F9 start prompt:

```powershell
.\InputPlay.exe play .\recordings\my-recording.irec `
    --start-immediately
```

This is recommended for PowerShell, Python, scheduled tasks, and other automated workflows.

## Playback Loops

Play a recording three times:

```powershell
.\InputPlay.exe play .\recordings\my-recording.irec --loops 3
```

Loop indefinitely:

```powershell
.\InputPlay.exe play .\recordings\my-recording.irec --loops inf
```

Infinite playback should normally be paired with a timeout or named cancellation session.

## Playback Timeout

Stop playback automatically after a number of seconds:

```powershell
.\InputPlay.exe play .\recordings\my-recording.irec `
    --loops inf `
    --start-immediately `
    --timeout 120
```

A timeout:

- Stops scheduling events
- Releases held keys
- Releases held mouse buttons
- Returns exit code `5`

The timeout applies to the complete playback operation, including loops and paused time.

## Named Playback Sessions

Start a named blocking playback session:

```powershell
.\InputPlay.exe play .\recordings\my-recording.irec `
    --send-input `
    --align-start `
    --start-immediately `
    --loops inf `
    --session report-run
```

Cancel the session from another terminal or process:

```powershell
.\InputPlay.exe cancel report-run
```

The original playback process remains blocked until playback completes, is cancelled, times out, or fails.

Session names may contain:

- Letters
- Numbers
- Hyphens
- Underscores

Session names may contain up to 64 characters.

## Display Compatibility

InputPlay records:

- Monitor count
- Monitor positions
- Monitor resolutions
- Monitor work areas
- Primary-monitor designation
- Virtual-desktop position and size

By default, InputPlay reports a warning when the current display setup differs from the recorded setup, then continues.

Require an exact or compatible display setup:

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

## Settings

InputPlay loads `settings.config` from the same directory as `InputPlay.exe`.

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

## PowerShell Example

Run a blocking playback process:

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

Cancel the playback from another PowerShell process:

```powershell
.\InputPlay.exe cancel powershell-test
```

## Python Example

```python
import subprocess

command = [
    "InputPlay.exe",
    "play",
    r"recordings\example.irec",
    "--send-input",
    "--align-start",
    "--start-immediately",
    "--session",
    "python-test",
    "--timeout",
    "120",
]

result = subprocess.run(command, check=False)

if result.returncode == 0:
    print("Playback completed.")
elif result.returncode == 4:
    print("Playback was cancelled.")
elif result.returncode == 5:
    print("Playback timed out.")
else:
    print(f"Playback failed with exit code {result.returncode}.")
```

Cancel from another Python process:

```python
import subprocess

subprocess.run(
    ["InputPlay.exe", "cancel", "python-test"],
    check=False,
)
```

## Safety Notes

Before live playback:

1. Validate the recording.
2. Check display compatibility.
3. Use a harmless test application first.
4. Consider specifying a timeout.
5. Use a named session for externally cancellable automation.
6. Keep the configured cancel shortcut available.
7. Avoid running untrusted `.irec` files.

InputPlay releases tracked keyboard keys and mouse buttons when playback completes, fails, is cancelled, or times out.

## Troubleshooting

### Recording does not start

Confirm the configured `record_start` key in `settings.config`, then press and release the key after `Recording armed` appears.

### Recording cancels before starting

The configured `record_stop` key was detected before the start key. Run the recording command again and press the configured start key.

### Playback does not send input

Live input requires:

```text
--send-input
```

Without that option, InputPlay runs in safe dry-run mode.

### Cursor alignment fails

The recording may not contain a starting cursor position, or the recorded coordinate may no longer exist in the current virtual desktop.

Inspect the file:

```powershell
.\InputPlay.exe info .\recordings\my-recording.irec
```

### Display compatibility warning

The monitor arrangement, resolution, work area, or primary monitor differs from the recording environment.

Use `info` to compare the recording with the current system.

### Named session cannot be created

Another active InputPlay process may already use the same session name. Choose a different session name or cancel the existing session.

### Function-key text appears in a terminal

InputPlay reads configured function keys without registering exclusive global shortcuts. Depending on the terminal, a function-key sequence may remain queued after InputPlay exits. This does not affect the recording or playback result.

For fully scripted playback, use:

```text
--start-immediately
```

## License and Warranty

InputPlay is provided without warranty. Test recordings carefully before using live playback in important workflows.