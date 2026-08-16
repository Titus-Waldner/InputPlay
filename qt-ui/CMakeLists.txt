cmake_minimum_required(VERSION 3.20)

project(
    InputPlayStudio
    VERSION 1.0.1
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Find Qt6 packages
find_package(Qt6 REQUIRED COMPONENTS
    Core
    Widgets
    Gui
)

# Include InputPlayCore from parent directory
set(INPUTPLAY_CORE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/..")
add_library(InputPlayCore STATIC IMPORTED)
set_target_properties(InputPlayCore PROPERTIES
    IMPORTED_LOCATION "${INPUTPLAY_CORE_DIR}/build/libInputPlayCore.a"
    INTERFACE_INCLUDE_DIRECTORIES "${INPUTPLAY_CORE_DIR}/include"
)

# Source files
set(SOURCES
    src/main.cpp
    src/MainWindow.cpp
    src/EventListModel.cpp
    src/EventListView.cpp
    src/PlaybackWidget.cpp
    src/PropertyEditor.cpp
    src/SettingsDialog.cpp
    src/MacroInfoPanel.cpp
    src/TimelineWidget.cpp
    src/DarkStyle.cpp
    src/UndoCommands.cpp
    src/EventCreationDialog.cpp
    src/PlaybackThread.cpp
    src/EventFilterWidget.cpp
    src/ShortcutsDialog.cpp
    src/GoToEventDialog.cpp
    src/BatchTimingDialog.cpp
    src/ExportDialog.cpp
    src/DisplayInfoDialog.cpp
    src/StatisticsPanel.cpp
    src/UndoHistoryPanel.cpp
    src/SearchWidget.cpp
    src/RecordingThread.cpp
    src/RecordingWidget.cpp
    src/GlobalHotkeyManager.cpp
    src/SystemTrayManager.cpp
	src/PhysicalMouseBlocker.cpp
)

set(HEADERS
    src/MainWindow.h
    src/EventListModel.h
    src/EventListView.h
    src/PlaybackWidget.h
    src/PropertyEditor.h
    src/SettingsDialog.h
    src/MacroInfoPanel.h
    src/TimelineWidget.h
    src/DarkStyle.h
    src/UndoCommands.h
    src/EventCreationDialog.h
    src/PlaybackThread.h
    src/EventFilterWidget.h
    src/ShortcutsDialog.h
    src/GoToEventDialog.h
    src/BatchTimingDialog.h
    src/ExportDialog.h
    src/DisplayInfoDialog.h
    src/StatisticsPanel.h
    src/UndoHistoryPanel.h
    src/SearchWidget.h
    src/RecordingThread.h
    src/RecordingWidget.h
    src/GlobalHotkeyManager.h
    src/SystemTrayManager.h
	src/PhysicalMouseBlocker.h
)

set(RESOURCES
    resources/resources.qrc
)

if(WIN32)
    list(APPEND RESOURCES
        resources/L33TR3PL4Y.rc
    )
endif()

# Create executable
add_executable(InputPlayStudio
    ${SOURCES}
    ${HEADERS}
    ${RESOURCES}
)

set_target_properties(
    InputPlayStudio
    PROPERTIES
        OUTPUT_NAME "L33TR3PL4Y"
)

target_include_directories(InputPlayStudio PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${INPUTPLAY_CORE_DIR}/include
)

target_link_libraries(InputPlayStudio PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Gui
    InputPlayCore
)

# Windows-specific settings
if(WIN32)
    target_link_libraries(InputPlayStudio PRIVATE shcore)
    set_target_properties(InputPlayStudio PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
endif()
