#pragma once

#include "Recording.h"
#include "Settings.h"

#include <QMainWindow>
#include <QString>

#include <memory>

class EventListModel;
class EventListView;
class PlaybackWidget;
class PropertyEditor;
class MacroInfoPanel;
class TimelineWidget;
class PlaybackThread;
class EventFilterWidget;
class StatisticsPanel;
class UndoHistoryPanel;
class SearchWidget;
class RecordingWidget;
class GlobalHotkeyManager;
class SystemTrayManager;
class QPushButton;

class QCloseEvent;
class QDragEnterEvent;
class QDropEvent;
class QLabel;
class QMenu;
class QAction;
class QSlider;
class QSplitter;
class QTabWidget;
class QStackedWidget;
class QUndoStack;
class QWidget;

class MainWindow
    : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(
        QWidget* parent = nullptr);

    ~MainWindow() override;

    void loadMacro(
        const QString& filePath);

signals:
    void macroLoaded(
        const QString& filePath);

    void macroModified();

    void eventSelected(
        int index);

public slots:
    void newMacro();
    void openMacro();
    void saveMacro();
    void saveMacroAs();
    void exportEvents();
    void openSettings();
    void showAbout();
    void showShortcuts();

    void addEvent();
    void insertEventBefore();
    void insertEventAfter();
    void deleteSelectedEvents();

    void duplicateEvent(
        int index);

    void moveEventUp(
        int index);

    void moveEventDown(
        int index);

    void copyEvents();
    void pasteEvents();
    void goToEvent();
    void batchTimingOperation();
    void findEvents();
    void showDisplayInfo();

    void toggleColorCodedRows(
        bool enabled);

    void toggleStatisticsPanel(
        bool visible);

    void toggleUndoHistoryPanel(
        bool visible);

    // Center workspace navigation.
    void showEditorWorkspace();
    void showRecordingWorkspace();
	void showListView();
	void showTimelineView();

    void onWorkspaceTabChanged(
        int index);

    // Recording lifecycle.
    void startRecording();
    void stopRecording();

    void onRecordingCompleted(
        bool hasEvents);

    void onRecordingCancelled();

    // Global hotkeys.
    void onRecordStartStopHotkey();
    void onRecordPauseHotkey();
    void onPlaybackStartStopHotkey();
    void onPlaybackPauseHotkey();
    void onEmergencyStop();

    void onFilterChanged();

    void onTimelineZoomChanged(
        int value);

    void onEventSelectionChanged(
        int index);

    void onEventModified(
        int index,
        const InputEvent& oldEvent,
        const InputEvent& newEvent);

    void onMacroModified();

protected:
    void closeEvent(
        QCloseEvent* event) override;

    void dragEnterEvent(
        QDragEnterEvent* event) override;

    void dropEvent(
        QDropEvent* event) override;

private:
    void setupUi();
    void setupMenuBar();
    void setupCentralWidget();
    void setupStatusBar();
    void setupConnections();
    void setupHotkeys();

    void updateWindowTitle();
    void updateActionStates();
    void updateStatusBar();
    void updateRecentFilesMenu();

    void addToRecentFiles(
        const QString& filePath);

    bool confirmDiscardChanges();

    bool saveMacroToFile(
        const QString& filePath);

    void showEventCreationDialog(
        int insertPosition);

    // Data.
    std::unique_ptr<Recording> recording_;
    Settings settings_;
    QString currentFilePath_;
    bool modified_ = false;
	
	// GUI-only playback preferences.
	bool dryRunDefault_ = true;
	bool confirmRealPlayback_ = true;

    // Undo and redo.
    QUndoStack* undoStack_ = nullptr;

    // Core interface components.
    EventListModel* eventModel_ = nullptr;
    EventListView* eventListView_ = nullptr;
    PlaybackWidget* playbackWidget_ = nullptr;
    PropertyEditor* propertyEditor_ = nullptr;
    MacroInfoPanel* infoPanel_ = nullptr;
    TimelineWidget* timelineWidget_ = nullptr;
    PlaybackThread* playbackThread_ = nullptr;
    EventFilterWidget* filterWidget_ = nullptr;
    StatisticsPanel* statisticsPanel_ = nullptr;
    UndoHistoryPanel* undoHistoryPanel_ = nullptr;
    SearchWidget* searchWidget_ = nullptr;
    RecordingWidget* recordingWidget_ = nullptr;
    GlobalHotkeyManager* hotkeyManager_ = nullptr;
    SystemTrayManager* trayManager_ = nullptr;

    // Main application layout.
    QSplitter* mainSplitter_ = nullptr;

    // Center workflow tabs.
    QTabWidget* workspaceTabs_ = nullptr;
    QWidget* editorWorkspace_ = nullptr;
    QWidget* recordingWorkspace_ = nullptr;

    // List and timeline pages inside the editor workspace.
    QStackedWidget* viewStack_ = nullptr;

    // Toolbar.
    QSlider* zoomSlider_ = nullptr;
    QLabel* zoomLabel_ = nullptr;
	QPushButton* listViewButton_ = nullptr;
	QPushButton* timelineViewButton_ = nullptr;

    // Status bar.
    QLabel* statusLabel_ = nullptr;
    QLabel* selectionLabel_ = nullptr;
    QLabel* coordsLabel_ = nullptr;
    QLabel* durationLabel_ = nullptr;

    // File actions.
    QAction* newAction_ = nullptr;
    QAction* openAction_ = nullptr;
    QAction* saveAction_ = nullptr;
    QAction* saveAsAction_ = nullptr;
    QAction* exportAction_ = nullptr;

    QMenu* recentFilesMenu_ = nullptr;
    QAction* clearRecentAction_ = nullptr;

    // Edit actions.
    QAction* undoAction_ = nullptr;
    QAction* redoAction_ = nullptr;
    QAction* copyAction_ = nullptr;
    QAction* pasteAction_ = nullptr;
    QAction* addEventAction_ = nullptr;
    QAction* insertBeforeAction_ = nullptr;
    QAction* insertAfterAction_ = nullptr;
    QAction* duplicateAction_ = nullptr;
    QAction* moveUpAction_ = nullptr;
    QAction* moveDownAction_ = nullptr;
    QAction* deleteAction_ = nullptr;
    QAction* selectAllAction_ = nullptr;
    QAction* goToAction_ = nullptr;
    QAction* batchTimingAction_ = nullptr;

    // View and panel actions.
    QAction* findAction_ = nullptr;
    QAction* displayInfoAction_ = nullptr;
    QAction* colorCodedRowsAction_ = nullptr;
    QAction* statisticsPanelAction_ = nullptr;
    QAction* undoHistoryPanelAction_ = nullptr;
    QAction* recordingPanelAction_ = nullptr;
    QAction* listViewAction_ = nullptr;
    QAction* timelineViewAction_ = nullptr;

    // Macro and settings actions.
    QAction* recordAction_ = nullptr;
    QAction* settingsAction_ = nullptr;

    static constexpr int EditorWorkspaceIndex = 0;
    static constexpr int RecordingWorkspaceIndex = 1;
    static constexpr int MaxRecentFiles = 10;
};