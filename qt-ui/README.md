# InputPlay Studio - Qt Interface

A modern, dark-mode Qt interface for InputPlay macro recording and playback.

## Features

- **Load & View Macros**: Open `.irec` files and view all event details
- **Event Editing**: Modify, add, insert, and delete events
- **List View**: Traditional table-based event display with color-coded types
- **Timeline View**: Visual timeline showing events over time
- **Playback Controls**: Play, pause, stop with speed control and loop options
- **Dry-Run Mode**: Safe preview without sending actual input
- **Property Editor**: Full control over every event property
- **Settings Dialog**: Configure hotkeys, playback defaults, and appearance
- **Drag & Drop**: Open files by dropping them onto the window
- **Display Compatibility**: Check if macro will work on your current display setup

## Requirements

- Qt 6.x (Core, Widgets, Gui modules)
- CMake 3.20+
- C++20 compatible compiler (MinGW-w64 or MSVC)
- InputPlayCore library (built from parent directory)

## Building

### Prerequisites

1. Install Qt 6 for Windows
2. Build InputPlayCore first:
   ```bash
   cd /path/to/InputPlay
   mkdir build && cd build
   cmake .. -G Ninja
   cmake --build .
   ```

### Build Qt UI

```bash
cd qt-ui
mkdir build && cd build
cmake .. -G Ninja -DCMAKE_PREFIX_PATH="C:/Qt/6.x.x/mingw_64"
cmake --build .
```

For release build:
```bash
cmake .. -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH="C:/Qt/6.x.x/mingw_64"
cmake --build .
```

### Run

```bash
./InputPlayStudio.exe

# Or open a macro directly:
./InputPlayStudio.exe path/to/macro.irec
```

## Usage

### Loading a Macro

1. Click **Open** or press `Ctrl+O`
2. Select a `.irec` file
3. Events appear in the list view

### Editing Events

- **Select**: Click an event row
- **Edit**: Use the Property Editor panel on the right
- **Insert**: Right-click → Insert Event Before/After
- **Delete**: Select events and press `Delete` or right-click → Delete

### Playback

1. Load a macro
2. Configure options:
   - **Dry Run**: On (safe) or Off (sends real input)
   - **Loops**: 1, 2, 3, 5, 10, custom, or infinite
   - **Speed**: 25% to 400%
3. Click **▶ Play**

### Views

- **List View**: Traditional table with columns (Type, Details, Time, Duration)
- **Timeline View**: Visual representation of events over time

### Keyboard Shortcuts

| Shortcut | Action |
|----------|--------|
| `Ctrl+N` | New macro |
| `Ctrl+O` | Open macro |
| `Ctrl+S` | Save macro |
| `Ctrl+Shift+S` | Save As |
| `Ctrl+Z` | Undo |
| `Ctrl+Y` | Redo |
| `Delete` | Delete selected events |
| `Ctrl+A` | Select all events |
| `Space` | Play/Pause toggle |

## Architecture

```
qt-ui/
├── CMakeLists.txt
├── README.md
├── src/
│   ├── main.cpp              # Application entry point
│   ├── MainWindow.h/cpp      # Main application window
│   ├── EventListModel.h/cpp  # Qt model for event data
│   ├── EventListView.h/cpp   # Table view for events
│   ├── TimelineWidget.h/cpp  # Visual timeline widget
│   ├── PlaybackWidget.h/cpp  # Playback controls
│   ├── PropertyEditor.h/cpp  # Event property editing
│   ├── MacroInfoPanel.h/cpp  # Macro information panel
│   ├── SettingsDialog.h/cpp  # Settings configuration
│   └── DarkStyle.h/cpp       # Dark theme styling
└── resources/
    ├── resources.qrc
    └── icons/                # SVG icons
```

## Theme

The interface uses a modern dark blue theme:

- **Background**: #1a1a2e (primary), #16213e (secondary), #0f3460 (tertiary)
- **Accent**: #00d9ff (cyan), #ff006e (magenta)
- **Text**: #ffffff (primary), #a0a0b0 (secondary)

Event types are color-coded:
- 🔵 Mouse Move (cyan)
- 🔴 Mouse Click (magenta)
- 🟣 Mouse Wheel (purple)
- 🟢 Key Events (green)
- ⚪ Wait Events (gray)
- 🟡 Teleport (yellow)

## License

Same as InputPlay parent project.
