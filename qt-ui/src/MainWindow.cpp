#include "MainWindow.h"
#include "EventListModel.h"
#include "EventListView.h"
#include "PlaybackWidget.h"
#include "PropertyEditor.h"
#include "MacroInfoPanel.h"
#include "TimelineWidget.h"
#include "SettingsDialog.h"
#include "DarkStyle.h"
#include "UndoCommands.h"
#include "EventCreationDialog.h"
#include "PlaybackThread.h"
#include "EventFilterWidget.h"
#include "ShortcutsDialog.h"
#include "GoToEventDialog.h"
#include "BatchTimingDialog.h"
#include "ExportDialog.h"
#include "DisplayInfoDialog.h"
#include "StatisticsPanel.h"
#include "UndoHistoryPanel.h"
#include "SearchWidget.h"
#include "RecordingWidget.h"
#include "RecordingThread.h"
#include "GlobalHotkeyManager.h"
#include "SystemTrayManager.h"

#include "RecordingFile.h"
#include "SettingsFile.h"

#include <QApplication>
#include <QMenuBar>
#include <QMenu>
#include <QToolBar>
#include <QStatusBar>
#include <QSplitter>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QAction>
#include <QActionGroup>
#include <QFileDialog>
#include <QMessageBox>
#include <QCloseEvent>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QFileInfo>
#include <QDir>
#include <QSettings>
#include <QUndoStack>
#include <QSlider>
#include <QClipboard>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QFile>
#include <QTextStream>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , recording_(std::make_unique<Recording>())
    , undoStack_(new QUndoStack(this))
{
    setWindowTitle("InputPlay Studio");
    setMinimumSize(1400, 900);
    resize(1400, 900);
    setAcceptDrops(true);
    
    // Load settings from InputPlay
    std::string settingsPath;
    std::string errorMessage;
    loadOrCreateSettings(settings_, settingsPath, errorMessage);
    
    // Create playback thread
    playbackThread_ = new PlaybackThread(this);
    
    // Create global hotkey manager
    hotkeyManager_ = new GlobalHotkeyManager(this);
    
    // Create system tray manager
    trayManager_ = new SystemTrayManager(this, this);
    
    setupUi();
    setupMenuBar();
    setupToolBar();
    setupCentralWidget();
    setupStatusBar();
    setupConnections();
    
    // Setup global hotkeys
    setupHotkeys();
    
    updateWindowTitle();
    updateActionStates();
    
    // Restore window state
    QSettings qsettings("InputPlay", "Studio");
    restoreGeometry(qsettings.value("geometry").toByteArray());
    restoreState(qsettings.value("windowState").toByteArray());
    
    // Restore view settings
    bool colorCoded = qsettings.value("colorCodedRows", true).toBool();
    colorCodedRowsAction_->setChecked(colorCoded);
    eventModel_->setColorCodedRows(colorCoded);
    
    // Show tray icon if available
    if (trayManager_->isAvailable()) {
        trayManager_->setVisible(qsettings.value("showTrayIcon", true).toBool());
    }
}

MainWindow::~MainWindow() = default;

void MainWindow::setupUi()
{
    // Create the model first
    eventModel_ = new EventListModel(this);
}

void MainWindow::setupMenuBar()
{
    // File Menu
    QMenu* fileMenu = menuBar()->addMenu(tr("&File"));
    
    newAction_ = fileMenu->addAction(tr("&New"), this, &MainWindow::newMacro);
    newAction_->setShortcut(QKeySequence::New);
    newAction_->setStatusTip(tr("Create a new macro"));
    
    openAction_ = fileMenu->addAction(tr("&Open..."), this, &MainWindow::openMacro);
    openAction_->setShortcut(QKeySequence::Open);
    openAction_->setStatusTip(tr("Open an existing macro file"));
    
    // Recent Files submenu
    recentFilesMenu_ = fileMenu->addMenu(tr("Open &Recent"));
    updateRecentFilesMenu();
    
    fileMenu->addSeparator();
    
    saveAction_ = fileMenu->addAction(tr("&Save"), this, &MainWindow::saveMacro);
    saveAction_->setShortcut(QKeySequence::Save);
    saveAction_->setStatusTip(tr("Save the current macro"));
    
    saveAsAction_ = fileMenu->addAction(tr("Save &As..."), this, &MainWindow::saveMacroAs);
    saveAsAction_->setShortcut(QKeySequence::SaveAs);
    saveAsAction_->setStatusTip(tr("Save the macro to a new file"));
    
    fileMenu->addSeparator();
    
    exportAction_ = fileMenu->addAction(tr("&Export Events..."), this, &MainWindow::exportEvents);
    exportAction_->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_E));
    exportAction_->setStatusTip(tr("Export events to CSV/JSON"));
    
    fileMenu->addSeparator();
    
    QAction* exitAction = fileMenu->addAction(tr("E&xit"), this, &QMainWindow::close);
    exitAction->setShortcut(QKeySequence::Quit);
    
    // Edit Menu
    QMenu* editMenu = menuBar()->addMenu(tr("&Edit"));
    
    undoAction_ = undoStack_->createUndoAction(this, tr("&Undo"));
    undoAction_->setShortcut(QKeySequence::Undo);
    editMenu->addAction(undoAction_);
    
    redoAction_ = undoStack_->createRedoAction(this, tr("&Redo"));
    redoAction_->setShortcut(QKeySequence::Redo);
    editMenu->addAction(redoAction_);
    
    editMenu->addSeparator();
    
    copyAction_ = editMenu->addAction(tr("&Copy Events"), this, &MainWindow::copyEvents);
    copyAction_->setShortcut(QKeySequence::Copy);
    copyAction_->setStatusTip(tr("Copy selected events to clipboard"));
    
    pasteAction_ = editMenu->addAction(tr("&Paste Events"), this, &MainWindow::pasteEvents);
    pasteAction_->setShortcut(QKeySequence::Paste);
    pasteAction_->setStatusTip(tr("Paste events from clipboard"));
    
    editMenu->addSeparator();
    
    addEventAction_ = editMenu->addAction(tr("&Add Event..."), this, &MainWindow::addEvent);
    addEventAction_->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_E));
    addEventAction_->setStatusTip(tr("Add a new event at the end"));
    
    insertBeforeAction_ = editMenu->addAction(tr("Insert Event &Before..."), this, &MainWindow::insertEventBefore);
    insertBeforeAction_->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_E));
    insertBeforeAction_->setStatusTip(tr("Insert a new event before selection"));
    
    insertAfterAction_ = editMenu->addAction(tr("Insert Event &After..."), this, &MainWindow::insertEventAfter);
    insertAfterAction_->setStatusTip(tr("Insert a new event after selection"));
    
    editMenu->addSeparator();
    
    duplicateAction_ = editMenu->addAction(tr("D&uplicate"), this, [this]() {
        int index = eventListView_->selectedEventIndex();
        if (index >= 0) duplicateEvent(index);
    });
    duplicateAction_->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_D));
    duplicateAction_->setStatusTip(tr("Duplicate selected event"));
    
    moveUpAction_ = editMenu->addAction(tr("Move &Up"), this, [this]() {
        int index = eventListView_->selectedEventIndex();
        if (index > 0) moveEventUp(index);
    });
    moveUpAction_->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Up));
    moveUpAction_->setStatusTip(tr("Move event up"));
    
    moveDownAction_ = editMenu->addAction(tr("Move Dow&n"), this, [this]() {
        int index = eventListView_->selectedEventIndex();
        if (recording_ && index >= 0 && index < static_cast<int>(recording_->eventCount()) - 1) {
            moveEventDown(index);
        }
    });
    moveDownAction_->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Down));
    moveDownAction_->setStatusTip(tr("Move event down"));
    
    editMenu->addSeparator();
    
    deleteAction_ = editMenu->addAction(tr("&Delete"), this, &MainWindow::deleteSelectedEvents);
    deleteAction_->setShortcut(QKeySequence::Delete);
    deleteAction_->setStatusTip(tr("Delete selected events"));
    
    selectAllAction_ = editMenu->addAction(tr("Select &All"));
    selectAllAction_->setShortcut(QKeySequence::SelectAll);
    
    editMenu->addSeparator();
    
    goToAction_ = editMenu->addAction(tr("&Go to Event..."), this, &MainWindow::goToEvent);
    goToAction_->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_G));
    goToAction_->setStatusTip(tr("Jump to a specific event"));
    
    batchTimingAction_ = editMenu->addAction(tr("&Batch Timing..."), this, &MainWindow::batchTimingOperation);
    batchTimingAction_->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_T));
    batchTimingAction_->setStatusTip(tr("Scale or offset event timing"));
    
    editMenu->addSeparator();
    
    settingsAction_ = editMenu->addAction(tr("&Settings..."), this, &MainWindow::openSettings);
    settingsAction_->setShortcut(QKeySequence::Preferences);
    
    // Macro Menu (Recording/Playback)
    QMenu* macroMenu = menuBar()->addMenu(tr("&Macro"));
    
    recordAction_ = macroMenu->addAction(tr("&Record..."), this, &MainWindow::startRecording);
    recordAction_->setShortcut(QKeySequence(Qt::Key_F9));
    recordAction_->setStatusTip(tr("Start recording a new macro"));
    
    QAction* stopRecordAction = macroMenu->addAction(tr("&Stop Recording"), this, &MainWindow::stopRecording);
    stopRecordAction->setShortcut(QKeySequence(Qt::SHIFT | Qt::Key_F9));
    stopRecordAction->setStatusTip(tr("Stop the current recording"));
    
    macroMenu->addSeparator();
    
    QAction* playAction = macroMenu->addAction(tr("&Play"), this, [this]() {
        playbackWidget_->play();
    });
    playAction->setShortcut(QKeySequence(Qt::Key_F5));
    playAction->setStatusTip(tr("Start playback of the current macro"));
    
    QAction* stopPlayAction = macroMenu->addAction(tr("S&top Playback"), this, [this]() {
        playbackWidget_->stop();
    });
    stopPlayAction->setShortcut(QKeySequence(Qt::SHIFT | Qt::Key_F5));
    stopPlayAction->setStatusTip(tr("Stop playback"));
    
    macroMenu->addSeparator();
    
    recordingPanelAction_ = macroMenu->addAction(tr("Show &Recording Panel"));
    recordingPanelAction_->setCheckable(true);
    recordingPanelAction_->setChecked(false);
    connect(recordingPanelAction_, &QAction::toggled, this, &MainWindow::toggleRecordingPanel);
    
    // View Menu
    QMenu* viewMenu = menuBar()->addMenu(tr("&View"));
    
    QActionGroup* viewGroup = new QActionGroup(this);
    
    listViewAction_ = viewMenu->addAction(tr("&List View"));
    listViewAction_->setCheckable(true);
    listViewAction_->setChecked(true);
    listViewAction_->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_1));
    viewGroup->addAction(listViewAction_);
    
    timelineViewAction_ = viewMenu->addAction(tr("&Timeline View"));
    timelineViewAction_->setCheckable(true);
    timelineViewAction_->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_2));
    viewGroup->addAction(timelineViewAction_);
    
    viewMenu->addSeparator();
    
    QAction* zoomInAction = viewMenu->addAction(tr("Zoom &In"));
    zoomInAction->setShortcut(QKeySequence::ZoomIn);
    connect(zoomInAction, &QAction::triggered, this, [this]() {
        if (zoomSlider_) zoomSlider_->setValue(zoomSlider_->value() + 10);
    });
    
    QAction* zoomOutAction = viewMenu->addAction(tr("Zoom &Out"));
    zoomOutAction->setShortcut(QKeySequence::ZoomOut);
    connect(zoomOutAction, &QAction::triggered, this, [this]() {
        if (zoomSlider_) zoomSlider_->setValue(zoomSlider_->value() - 10);
    });
    
    QAction* zoomResetAction = viewMenu->addAction(tr("&Reset Zoom"));
    zoomResetAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_0));
    connect(zoomResetAction, &QAction::triggered, this, [this]() {
        if (zoomSlider_) zoomSlider_->setValue(100);
    });
    
    viewMenu->addSeparator();
    
    findAction_ = viewMenu->addAction(tr("&Find Events..."), this, &MainWindow::findEvents);
    findAction_->setShortcut(QKeySequence::Find);
    findAction_->setStatusTip(tr("Search for events"));
    
    viewMenu->addSeparator();
    
    // Panel visibility toggles
    statisticsPanelAction_ = viewMenu->addAction(tr("&Statistics Panel"));
    statisticsPanelAction_->setCheckable(true);
    statisticsPanelAction_->setChecked(false);
    connect(statisticsPanelAction_, &QAction::toggled, this, &MainWindow::toggleStatisticsPanel);
    
    undoHistoryPanelAction_ = viewMenu->addAction(tr("&Undo History Panel"));
    undoHistoryPanelAction_->setCheckable(true);
    undoHistoryPanelAction_->setChecked(false);
    connect(undoHistoryPanelAction_, &QAction::toggled, this, &MainWindow::toggleUndoHistoryPanel);
    
    viewMenu->addSeparator();
    
    colorCodedRowsAction_ = viewMenu->addAction(tr("&Color-coded Event Rows"));
    colorCodedRowsAction_->setCheckable(true);
    colorCodedRowsAction_->setChecked(true);
    colorCodedRowsAction_->setStatusTip(tr("Show subtle background colors based on event type"));
    connect(colorCodedRowsAction_, &QAction::toggled, this, &MainWindow::toggleColorCodedRows);
    
    viewMenu->addSeparator();
    
    displayInfoAction_ = viewMenu->addAction(tr("&Display Compatibility..."), this, &MainWindow::showDisplayInfo);
    displayInfoAction_->setStatusTip(tr("View display compatibility details"));
    
    // Help Menu
    QMenu* helpMenu = menuBar()->addMenu(tr("&Help"));
    
    QAction* shortcutsAction = helpMenu->addAction(tr("&Keyboard Shortcuts..."), this, &MainWindow::showShortcuts);
    shortcutsAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Slash));
    
    helpMenu->addSeparator();
    
    QAction* aboutAction = helpMenu->addAction(tr("&About InputPlay Studio"), this, &MainWindow::showAbout);
    
    QAction* aboutQtAction = helpMenu->addAction(tr("About &Qt"), qApp, &QApplication::aboutQt);
}

void MainWindow::setupToolBar()
{
    mainToolBar_ = addToolBar(tr("Main Toolbar"));
    mainToolBar_->setMovable(false);
    mainToolBar_->setIconSize(QSize(20, 20));
    
    // File actions
    QAction* newBtn = mainToolBar_->addAction(tr("New"));
    connect(newBtn, &QAction::triggered, this, &MainWindow::newMacro);
    
    QAction* openBtn = mainToolBar_->addAction(tr("Open"));
    connect(openBtn, &QAction::triggered, this, &MainWindow::openMacro);
    
    QAction* saveBtn = mainToolBar_->addAction(tr("Save"));
    connect(saveBtn, &QAction::triggered, this, &MainWindow::saveMacro);
    
    mainToolBar_->addSeparator();
    
    // Recording action
    QAction* recordBtn = mainToolBar_->addAction(tr("⏺ Record"));
    recordBtn->setToolTip(tr("Start recording a new macro (F9)"));
    connect(recordBtn, &QAction::triggered, this, &MainWindow::startRecording);
    
    mainToolBar_->addSeparator();
    
    // View toggle
    QAction* listBtn = mainToolBar_->addAction(tr("List"));
    listBtn->setCheckable(true);
    listBtn->setChecked(true);
    connect(listBtn, &QAction::triggered, this, [this]() {
        viewStack_->setCurrentIndex(0);
        listViewAction_->setChecked(true);
    });
    
    QAction* timelineBtn = mainToolBar_->addAction(tr("Timeline"));
    timelineBtn->setCheckable(true);
    connect(timelineBtn, &QAction::triggered, this, [this]() {
        viewStack_->setCurrentIndex(1);
        timelineViewAction_->setChecked(true);
    });
    
    QActionGroup* viewBtnGroup = new QActionGroup(this);
    viewBtnGroup->addAction(listBtn);
    viewBtnGroup->addAction(timelineBtn);
    
    mainToolBar_->addSeparator();
    
    // Zoom controls
    QLabel* zoomIconLabel = new QLabel(tr("Zoom:"));
    mainToolBar_->addWidget(zoomIconLabel);
    
    zoomSlider_ = new QSlider(Qt::Horizontal);
    zoomSlider_->setRange(10, 400);
    zoomSlider_->setValue(100);
    zoomSlider_->setFixedWidth(100);
    zoomSlider_->setToolTip(tr("Timeline zoom (10% - 400%)"));
    connect(zoomSlider_, &QSlider::valueChanged, this, &MainWindow::onTimelineZoomChanged);
    mainToolBar_->addWidget(zoomSlider_);
    
    zoomLabel_ = new QLabel(tr("100%"));
    zoomLabel_->setMinimumWidth(40);
    mainToolBar_->addWidget(zoomLabel_);
    
    mainToolBar_->addSeparator();
    
    // Settings
    QAction* settingsBtn = mainToolBar_->addAction(tr("Settings"));
    connect(settingsBtn, &QAction::triggered, this, &MainWindow::openSettings);
}

void MainWindow::setupCentralWidget()
{
    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    
    QHBoxLayout* mainLayout = new QHBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    
    // Main horizontal splitter
	mainSplitter_ =
		new QSplitter(
			Qt::Horizontal,
			centralWidget);

	mainSplitter_->setChildrenCollapsible(
		false);

	mainLayout->addWidget(
		mainSplitter_);
    
    // Left Panel: Macro Info, Recording, and Statistics (stacked vertically)
    QWidget* leftContainer = new QWidget(this);
    QVBoxLayout* leftLayout = new QVBoxLayout(leftContainer);
    leftLayout->setContentsMargins(0, 0, 0, 0);
    leftLayout->setSpacing(0);
    
    infoPanel_ = new MacroInfoPanel(this);
    leftLayout->addWidget(infoPanel_, 1);
    
    // Recording Widget (hidden by default)
    recordingWidget_ = new RecordingWidget(this);
    recordingWidget_->setVisible(false);
    leftLayout->addWidget(recordingWidget_, 1);
    
    // Statistics Panel (hidden by default)
    statisticsPanel_ = new StatisticsPanel(this);
    statisticsPanel_->setVisible(false);
    leftLayout->addWidget(statisticsPanel_, 1);
    
    leftContainer->setMinimumWidth(250);
    leftContainer->setMaximumWidth(400);
    mainSplitter_->addWidget(leftContainer);
    
    // Center Panel: Event Views (List and Timeline)
    QWidget* centerContainer =
		new QWidget(this);

	centerContainer->setMinimumWidth(
		820);

	QVBoxLayout* centerLayout =
		new QVBoxLayout(
			centerContainer);
    centerLayout->setContentsMargins(0, 0, 0, 0);
    centerLayout->setSpacing(0);
    
    // Playback Widget at top
    playbackWidget_ = new PlaybackWidget(this);
    centerLayout->addWidget(playbackWidget_);
    
    // Search widget (hidden by default)
    searchWidget_ = new SearchWidget(this);
    searchWidget_->setVisible(false);
    centerLayout->addWidget(searchWidget_);
    
    // Filter widget
    filterWidget_ = new EventFilterWidget(this);
    filterWidget_->setContentsMargins(8, 4, 8, 4);
    centerLayout->addWidget(filterWidget_);
    
    // View Stack (List/Timeline)
    viewStack_ = new QStackedWidget(this);
    
    eventListView_ = new EventListView(this);
    eventListView_->setModel(eventModel_);
    viewStack_->addWidget(eventListView_);
    
    timelineWidget_ = new TimelineWidget(this);
    viewStack_->addWidget(timelineWidget_);
    
    centerLayout->addWidget(viewStack_, 1);
    
    mainSplitter_->addWidget(centerContainer);
    
    // Right Panel: Property Editor and Undo History (stacked vertically)
    QWidget* rightContainer = new QWidget(this);
    QVBoxLayout* rightLayout = new QVBoxLayout(rightContainer);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->setSpacing(0);
    
    propertyEditor_ = new PropertyEditor(this);
    rightLayout->addWidget(propertyEditor_, 2);
    
    // Undo History Panel (hidden by default)
    undoHistoryPanel_ = new UndoHistoryPanel(this);
    undoHistoryPanel_->setUndoStack(undoStack_);
    undoHistoryPanel_->setVisible(false);
    rightLayout->addWidget(undoHistoryPanel_, 1);
    
    rightContainer->setMinimumWidth(280);
    rightContainer->setMaximumWidth(450);
    mainSplitter_->addWidget(rightContainer);
    
    // Set splitter sizes
    // Prevent individual panels from being collapsed.
	mainSplitter_->setCollapsible(
		0,
		false);

	mainSplitter_->setCollapsible(
		1,
		false);

	mainSplitter_->setCollapsible(
		2,
		false);

	// Give the center workspace most of the available width.
	mainSplitter_->setSizes(
		{
			260,
			850,
			290
		});

	mainSplitter_->setStretchFactor(
		0,
		0);

	mainSplitter_->setStretchFactor(
		1,
		1);

	mainSplitter_->setStretchFactor(
		2,
		0);
}

void MainWindow::setupStatusBar()
{
    statusLabel_ = new QLabel(tr("Ready"));
    statusBar()->addWidget(statusLabel_, 1);
    
    durationLabel_ = new QLabel(tr(""));
    statusBar()->addPermanentWidget(durationLabel_);
    
    selectionLabel_ = new QLabel(tr("No selection"));
    statusBar()->addPermanentWidget(selectionLabel_);
    
    coordsLabel_ = new QLabel(tr(""));
    statusBar()->addPermanentWidget(coordsLabel_);
}

void MainWindow::setupConnections()
{
    // View actions
    connect(listViewAction_, &QAction::triggered, this, [this]() {
        viewStack_->setCurrentIndex(0);
    });
    
    connect(timelineViewAction_, &QAction::triggered, this, [this]() {
        viewStack_->setCurrentIndex(1);
    });
    
    // Filter widget
    connect(filterWidget_, &EventFilterWidget::filterChanged, this, &MainWindow::onFilterChanged);
    
    // Event selection
    connect(eventListView_, &EventListView::eventSelected, this, &MainWindow::onEventSelectionChanged);
    
    // EventListView context menu actions
    connect(eventListView_, &EventListView::duplicateRequested, this, &MainWindow::duplicateEvent);
    connect(eventListView_, &EventListView::moveUpRequested, this, &MainWindow::moveEventUp);
    connect(eventListView_, &EventListView::moveDownRequested, this, &MainWindow::moveEventDown);
    connect(eventListView_, &EventListView::deleteEventsRequested, this, [this](const QList<int>& indices) {
        if (!indices.isEmpty()) {
            undoStack_->push(new RemoveEventsCommand(eventModel_, indices));
        }
    });
    connect(eventListView_, &EventListView::addEventRequested, this, [this](int afterIndex) {
        if (afterIndex < 0) {
            addEvent();
        } else {
            showEventCreationDialog(afterIndex + 1);
        }
    });
    
    // Property changes - now with old and new event for undo
    connect(propertyEditor_, &PropertyEditor::eventModified, this, &MainWindow::onEventModified);
    
    // Select all
    connect(selectAllAction_, &QAction::triggered, eventListView_, &EventListView::selectAll);
    
    // Model changes trigger modified state
    connect(eventModel_, &EventListModel::dataChanged, this, &MainWindow::onMacroModified);
    connect(eventModel_, &EventListModel::rowsInserted, this, &MainWindow::onMacroModified);
    connect(eventModel_, &EventListModel::rowsRemoved, this, &MainWindow::onMacroModified);
    
    // Undo stack clean state
    connect(undoStack_, &QUndoStack::cleanChanged, this, [this](bool clean) {
        modified_ = !clean;
        updateWindowTitle();
    });
    
    // Playback thread connections
    connect(playbackThread_, &PlaybackThread::playbackStarted, this, [this]() {
        statusLabel_->setText(tr("Playing..."));
        playbackWidget_->setPlaybackActive(true);
    });
    
    connect(playbackThread_, &PlaybackThread::playbackStopped, this, [this]() {
        statusLabel_->setText(tr("Stopped"));
        playbackWidget_->setPlaybackActive(false);
        timelineWidget_->clearPlayhead();
    });
    
    connect(playbackThread_, &PlaybackThread::playbackCompleted, this, [this]() {
        statusLabel_->setText(tr("Playback completed"));
        playbackWidget_->setPlaybackActive(false);
        timelineWidget_->clearPlayhead();
    });
    
    connect(playbackThread_, &PlaybackThread::playbackPaused, this, [this]() {
        statusLabel_->setText(tr("Paused"));
    });
    
    connect(playbackThread_, &PlaybackThread::playbackResumed, this, [this]() {
        statusLabel_->setText(tr("Playing..."));
    });
    
    connect(playbackThread_, &PlaybackThread::progressChanged, this, [this](const PlaybackProgress& progress) {
        playbackWidget_->updateProgress(static_cast<int>(progress.completedEvents), static_cast<int>(progress.totalEvents));
    });
    connect(playbackThread_, &PlaybackThread::eventExecuted, timelineWidget_, &TimelineWidget::setPlayheadIndex);
    
    connect(playbackThread_, &PlaybackThread::error, this, [this](const QString& message) {
        QMessageBox::warning(this, tr("Playback Error"), message);
        statusLabel_->setText(tr("Playback error"));
        playbackWidget_->setPlaybackActive(false);
        timelineWidget_->clearPlayhead();
    });
    
    // Playback widget to thread
    connect(playbackWidget_, &PlaybackWidget::playRequested, this, [this]() {
        if (recording_ && !recording_->empty()) {
            playbackThread_->setRecording(recording_.get());
            playbackThread_->setSpeed(playbackWidget_->speed());
            playbackThread_->setDryRun(playbackWidget_->isDryRun());
            playbackThread_->setLooping(playbackWidget_->isLooping());
            playbackThread_->start();
        }
    });
    
    connect(playbackWidget_, &PlaybackWidget::pauseRequested, playbackThread_, &PlaybackThread::pause);
    connect(playbackWidget_, &PlaybackWidget::resumeRequested, playbackThread_, &PlaybackThread::resume);
    connect(playbackWidget_, &PlaybackWidget::stopRequested, playbackThread_, &PlaybackThread::stop);
    
    connect(playbackWidget_, &PlaybackWidget::speedChanged, playbackThread_, &PlaybackThread::setSpeed);
    
    // Info panel file operations
    connect(infoPanel_, &MacroInfoPanel::newRequested, this, &MainWindow::newMacro);
    connect(infoPanel_, &MacroInfoPanel::openRequested, this, &MainWindow::openMacro);
    
    // Search widget connections
    connect(searchWidget_, &SearchWidget::navigateToResult, this, [this](int index) {
        eventListView_->selectEvent(index);
        eventListView_->scrollTo(eventModel_->index(index, 0));
    });
    
    // Statistics panel filter button
    connect(statisticsPanel_, &StatisticsPanel::filterByType, this, [this](EventType type) {
        if (filterWidget_) {
            filterWidget_->setTypeFilter(type);
        }
    });
}

void MainWindow::loadMacro(const QString& filePath)
{
    if (!confirmDiscardChanges()) {
        return;
    }
    
    Recording newRecording;
    std::string errorMessage;
    
    if (!RecordingFile::load(filePath.toStdString(), newRecording, errorMessage)) {
        QMessageBox::critical(this, tr("Load Error"),
            tr("Failed to load macro:\n%1").arg(QString::fromStdString(errorMessage)));
        return;
    }
    
    *recording_ = std::move(newRecording);
    currentFilePath_ = filePath;
    modified_ = false;
    
    undoStack_->clear();
    undoStack_->setClean();
    
    eventModel_->setRecording(recording_.get());
    infoPanel_->setRecording(recording_.get(), filePath);
    timelineWidget_->setRecording(recording_.get());
    playbackWidget_->setRecording(recording_.get());
    propertyEditor_->clear();
    searchWidget_->setRecording(recording_.get());
    statisticsPanel_->setRecording(recording_.get());
    
    updateWindowTitle();
    updateActionStates();
    updateStatusBar();
    
    addToRecentFiles(filePath);
    statusLabel_->setText(tr("Loaded: %1").arg(QFileInfo(filePath).fileName()));
    
    emit macroLoaded(filePath);
}

void MainWindow::newMacro()
{
    if (!confirmDiscardChanges()) {
        return;
    }
    
    recording_ = std::make_unique<Recording>();
    currentFilePath_.clear();
    modified_ = false;
    
    undoStack_->clear();
    undoStack_->setClean();
    
    eventModel_->setRecording(recording_.get());
    infoPanel_->setRecording(recording_.get(), "");
    timelineWidget_->setRecording(recording_.get());
    playbackWidget_->setRecording(recording_.get());
    propertyEditor_->clear();
    searchWidget_->setRecording(recording_.get());
    statisticsPanel_->setRecording(recording_.get());
    
    updateWindowTitle();
    updateActionStates();
    
    statusLabel_->setText(tr("New macro created"));
}

void MainWindow::openMacro()
{
    QString filePath = QFileDialog::getOpenFileName(
        this,
        tr("Open Macro"),
        QDir::currentPath(),
        tr("InputPlay Recordings (*.irec);;All Files (*)")
    );
    
    if (!filePath.isEmpty()) {
        loadMacro(filePath);
    }
}

void MainWindow::saveMacro()
{
    if (currentFilePath_.isEmpty()) {
        saveMacroAs();
        return;
    }
    
    if (saveMacroToFile(currentFilePath_)) {
        modified_ = false;
        updateWindowTitle();
        statusLabel_->setText(tr("Saved: %1").arg(QFileInfo(currentFilePath_).fileName()));
    }
}

void MainWindow::saveMacroAs()
{
    QString filePath = QFileDialog::getSaveFileName(
        this,
        tr("Save Macro As"),
        currentFilePath_.isEmpty() ? QDir::currentPath() : currentFilePath_,
        tr("InputPlay Recordings (*.irec);;All Files (*)")
    );
    
    if (!filePath.isEmpty()) {
        if (!filePath.endsWith(".irec", Qt::CaseInsensitive)) {
            filePath += ".irec";
        }
        
        if (saveMacroToFile(filePath)) {
            currentFilePath_ = filePath;
            modified_ = false;
            updateWindowTitle();
            infoPanel_->setRecording(recording_.get(), filePath);
            statusLabel_->setText(tr("Saved: %1").arg(QFileInfo(filePath).fileName()));
        }
    }
}

bool MainWindow::saveMacroToFile(const QString& filePath)
{
    std::string errorMessage;
    
    if (!RecordingFile::save(*recording_, filePath.toStdString(), errorMessage)) {
        QMessageBox::critical(this, tr("Save Error"),
            tr("Failed to save macro:\n%1").arg(QString::fromStdString(errorMessage)));
        return false;
    }
    
    undoStack_->setClean();
    return true;
}

void MainWindow::openSettings()
{
    SettingsDialog dialog(settings_, this);
    
    // Save current theme in case user cancels
    ThemeType originalTheme = DarkStyle::currentTheme();
    AccentColor originalAccent = DarkStyle::currentAccent();
    
    // Live preview of theme changes
    connect(&dialog, &SettingsDialog::appearanceChanged, this, [](ThemeType theme, AccentColor accent) {
        DarkStyle::apply(qApp, theme, accent);
    });
    
    if (dialog.exec() == QDialog::Accepted) {
        settings_ = dialog.settings();
        
        // Save appearance settings
        QSettings qsettings("InputPlay", "Studio");
        qsettings.setValue("theme", static_cast<int>(dialog.selectedTheme()));
        qsettings.setValue("accentColor", static_cast<int>(dialog.selectedAccent()));
        
        statusLabel_->setText(tr("Settings updated"));
    } else {
        // User cancelled - restore original theme
        if (DarkStyle::currentTheme() != originalTheme || DarkStyle::currentAccent() != originalAccent) {
            DarkStyle::apply(qApp, originalTheme, originalAccent);
        }
    }
}

void MainWindow::showAbout()
{
    QMessageBox::about(this, tr("About InputPlay Studio"),
        tr("<h3>InputPlay Studio</h3>"
           "<p>Version 1.0.0</p>"
           "<p>A powerful macro recording and playback tool for Windows.</p>"
           "<p>Built with Qt and InputPlayCore.</p>"));
}

void MainWindow::showShortcuts()
{
    ShortcutsDialog dialog(this);
    dialog.exec();
}

void MainWindow::onFilterChanged()
{
    // For now, we'll just update visibility of rows based on filter
    // A more sophisticated implementation would use a QSortFilterProxyModel
    if (!recording_ || !eventModel_ || !filterWidget_) return;
    
    int visibleCount = 0;
    int totalCount = static_cast<int>(recording_->eventCount());
    
    for (int i = 0; i < totalCount; ++i) {
        const InputEvent& event = recording_->events()[i];
        bool visible = filterWidget_->matchesFilter(event, i);
        eventListView_->setRowHidden(i, !visible);
        if (visible) ++visibleCount;
    }
    
    if (visibleCount < totalCount) {
        statusLabel_->setText(tr("Showing %1 of %2 events").arg(visibleCount).arg(totalCount));
    } else {
        updateStatusBar();
    }
}

void MainWindow::onTimelineZoomChanged(int value)
{
    if (zoomLabel_) {
        zoomLabel_->setText(tr("%1%").arg(value));
    }
    if (timelineWidget_) {
        timelineWidget_->setZoom(value / 100.0);
    }
}

void MainWindow::onEventSelectionChanged(int index)
{
    if (index >= 0 && recording_ && index < static_cast<int>(recording_->eventCount())) {
        const InputEvent& event = recording_->events()[index];
        propertyEditor_->setEvent(&event, index);
        selectionLabel_->setText(tr("Event %1 of %2").arg(index + 1).arg(recording_->eventCount()));
        
        if (event.type == EventType::MouseMove || event.type == EventType::MouseButtonDown ||
            event.type == EventType::MouseButtonUp || event.type == EventType::MouseTeleport) {
            coordsLabel_->setText(tr("(%1, %2)").arg(event.mouseX).arg(event.mouseY));
        } else {
            coordsLabel_->clear();
        }
    } else {
        propertyEditor_->clear();
        selectionLabel_->setText(tr("No selection"));
        coordsLabel_->clear();
    }
    
    emit eventSelected(index);
}

void MainWindow::onEventModified(int index, const InputEvent& oldEvent, const InputEvent& newEvent)
{
    if (index >= 0 && eventModel_) {
        undoStack_->push(new ModifyEventCommand(eventModel_, index, oldEvent, newEvent));
    }
}

void MainWindow::onMacroModified()
{
    if (!modified_) {
        modified_ = true;
        updateWindowTitle();
    }
    infoPanel_->refresh();
}

void MainWindow::updateWindowTitle()
{
    QString title = "InputPlay Studio";
    
    if (!currentFilePath_.isEmpty()) {
        title = QFileInfo(currentFilePath_).fileName() + " - " + title;
    } else if (recording_ && !recording_->empty()) {
        title = "Untitled - " + title;
    }
    
    if (modified_) {
        title = "* " + title;
    }
    
    setWindowTitle(title);
}

void MainWindow::updateActionStates()
{
    bool hasRecording = recording_ && !recording_->empty();
    bool hasFile = !currentFilePath_.isEmpty();
    bool hasSelection = eventListView_ && !eventListView_->selectedEventIndices().isEmpty();
    
    saveAction_->setEnabled(hasRecording || hasFile);
    saveAsAction_->setEnabled(hasRecording);
    exportAction_->setEnabled(hasRecording);
    deleteAction_->setEnabled(hasRecording && hasSelection);
    selectAllAction_->setEnabled(hasRecording);
    copyAction_->setEnabled(hasSelection);
    goToAction_->setEnabled(hasRecording);
    batchTimingAction_->setEnabled(hasRecording);
    findAction_->setEnabled(hasRecording);
    displayInfoAction_->setEnabled(recording_ != nullptr);
}

void MainWindow::updateStatusBar()
{
    if (recording_ && !recording_->empty()) {
        statusLabel_->setText(tr("Events: %1").arg(recording_->eventCount()));
        
        // Calculate and show duration
        const auto& events = recording_->events();
        std::uint64_t totalDuration = events.back().timestampMicroseconds;
        
        if (totalDuration < 1000000) {
            durationLabel_->setText(tr("Duration: %1 ms").arg(totalDuration / 1000.0, 0, 'f', 1));
        } else {
            double seconds = totalDuration / 1000000.0;
            if (seconds < 60) {
                durationLabel_->setText(tr("Duration: %1 s").arg(seconds, 0, 'f', 2));
            } else {
                int mins = static_cast<int>(seconds) / 60;
                double secs = seconds - (mins * 60);
                durationLabel_->setText(tr("Duration: %1m %2s").arg(mins).arg(secs, 0, 'f', 1));
            }
        }
    } else {
        statusLabel_->setText(tr("Ready"));
        durationLabel_->clear();
    }
}

bool MainWindow::confirmDiscardChanges()
{
    if (!modified_) {
        return true;
    }
    
    QMessageBox::StandardButton result = QMessageBox::question(
        this,
        tr("Unsaved Changes"),
        tr("The current macro has unsaved changes. Do you want to save them?"),
        QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
        QMessageBox::Save
    );
    
    if (result == QMessageBox::Cancel) {
        return false;
    }
    
    if (result == QMessageBox::Save) {
        saveMacro();
        return !modified_; // Return true only if save succeeded
    }
    
    return true;
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    if (confirmDiscardChanges()) {
        // Save window state
        QSettings settings("InputPlay", "Studio");
        settings.setValue("geometry", saveGeometry());
        settings.setValue("windowState", saveState());
        settings.setValue("colorCodedRows", colorCodedRowsAction_->isChecked());
        
        event->accept();
    } else {
        event->ignore();
    }
}

void MainWindow::dragEnterEvent(QDragEnterEvent* event)
{
    if (event->mimeData()->hasUrls()) {
        for (const QUrl& url : event->mimeData()->urls()) {
            if (url.isLocalFile() && url.toLocalFile().endsWith(".irec", Qt::CaseInsensitive)) {
                event->acceptProposedAction();
                return;
            }
        }
    }
}

void MainWindow::dropEvent(QDropEvent* event)
{
    for (const QUrl& url : event->mimeData()->urls()) {
        if (url.isLocalFile() && url.toLocalFile().endsWith(".irec", Qt::CaseInsensitive)) {
            loadMacro(url.toLocalFile());
            break;
        }
    }
}

void MainWindow::addEvent()
{
    if (!recording_) return;
    
    int insertPosition = static_cast<int>(recording_->eventCount());
    showEventCreationDialog(insertPosition);
}

void MainWindow::insertEventBefore()
{
    if (!recording_) return;
    
    QModelIndexList selection = eventListView_->selectionModel()->selectedRows();
    int insertPosition = selection.isEmpty() ? 0 : selection.first().row();
    showEventCreationDialog(insertPosition);
}

void MainWindow::insertEventAfter()
{
    if (!recording_) return;
    
    QModelIndexList selection = eventListView_->selectionModel()->selectedRows();
    int insertPosition = selection.isEmpty() 
        ? static_cast<int>(recording_->eventCount())
        : selection.last().row() + 1;
    showEventCreationDialog(insertPosition);
}

void MainWindow::deleteSelectedEvents()
{
    if (!recording_ || !eventModel_) return;
    
    QModelIndexList selection = eventListView_->selectionModel()->selectedRows();
    if (selection.isEmpty()) return;
    
    // Collect row indices for the command
    QList<int> rows;
    for (const QModelIndex& idx : selection) {
        rows.append(idx.row());
    }
    
    if (rows.isEmpty()) return;
    
    // Use batch removal command
    undoStack_->push(new RemoveEventsCommand(eventModel_, rows));
}

void MainWindow::showEventCreationDialog(int insertPosition)
{
    EventCreationDialog dialog(this);
    
    if (dialog.exec() == QDialog::Accepted) {
        InputEvent event = dialog.createdEvent();
        
        if (insertPosition >= static_cast<int>(recording_->eventCount())) {
            // Add at end
            undoStack_->push(new AddEventCommand(eventModel_, event));
        } else {
            // Insert at position
            undoStack_->push(new InsertEventCommand(eventModel_, insertPosition, event));
        }
    }
}

void MainWindow::duplicateEvent(int index)
{
    if (!recording_ || index < 0 || index >= static_cast<int>(recording_->eventCount())) {
        return;
    }
    
    InputEvent event = recording_->events()[index];
    // Insert after the selected event
    undoStack_->push(new InsertEventCommand(eventModel_, index + 1, event));
    
    // Select the new event
    eventListView_->selectEvent(index + 1);
}

void MainWindow::moveEventUp(int index)
{
    if (!recording_ || index <= 0 || index >= static_cast<int>(recording_->eventCount())) {
        return;
    }
    
    undoStack_->push(new MoveEventCommand(eventModel_, index, index - 1));
    
    // Keep selection on the moved event
    eventListView_->selectEvent(index - 1);
}

void MainWindow::moveEventDown(int index)
{
    if (!recording_ || index < 0 || index >= static_cast<int>(recording_->eventCount()) - 1) {
        return;
    }
    
    undoStack_->push(new MoveEventCommand(eventModel_, index, index + 1));
    
    // Keep selection on the moved event
    eventListView_->selectEvent(index + 1);
}

void MainWindow::updateRecentFilesMenu()
{
    recentFilesMenu_->clear();
    
    QSettings settings("InputPlay", "Studio");
    QStringList recentFiles = settings.value("recentFiles").toStringList();
    
    if (recentFiles.isEmpty()) {
        QAction* emptyAction = recentFilesMenu_->addAction(tr("(No recent files)"));
        emptyAction->setEnabled(false);
    } else {
        for (int i = 0; i < recentFiles.size() && i < MaxRecentFiles; ++i) {
            QString filePath = recentFiles[i];
            QFileInfo fileInfo(filePath);
            
            QString text = QString("&%1  %2").arg(i + 1).arg(fileInfo.fileName());
            QAction* action = recentFilesMenu_->addAction(text);
            action->setData(filePath);
            action->setToolTip(filePath);
            
            connect(action, &QAction::triggered, this, [this, filePath]() {
                loadMacro(filePath);
            });
        }
        
        recentFilesMenu_->addSeparator();
        clearRecentAction_ = recentFilesMenu_->addAction(tr("Clear Recent Files"));
        connect(clearRecentAction_, &QAction::triggered, this, [this]() {
            QSettings settings("InputPlay", "Studio");
            settings.setValue("recentFiles", QStringList());
            updateRecentFilesMenu();
        });
    }
}

void MainWindow::addToRecentFiles(const QString& filePath)
{
    QSettings settings("InputPlay", "Studio");
    QStringList recentFiles = settings.value("recentFiles").toStringList();
    
    // Remove if already exists
    recentFiles.removeAll(filePath);
    
    // Add to front
    recentFiles.prepend(filePath);
    
    // Limit to max
    while (recentFiles.size() > MaxRecentFiles) {
        recentFiles.removeLast();
    }
    
    settings.setValue("recentFiles", recentFiles);
    updateRecentFilesMenu();
}

void MainWindow::copyEvents()
{
    if (!recording_) return;
    
    QList<int> indices = eventListView_->selectedEventIndices();
    if (indices.isEmpty()) return;
    
    // Serialize selected events to JSON
    QJsonArray eventsArray;
    for (int index : indices) {
        if (index >= 0 && index < static_cast<int>(recording_->eventCount())) {
            const InputEvent& event = recording_->events()[index];
            
            QJsonObject eventObj;
            eventObj["type"] = static_cast<int>(event.type);
            eventObj["timestamp"] = static_cast<qint64>(event.timestampMicroseconds);
            eventObj["mouseX"] = event.mouseX;
            eventObj["mouseY"] = event.mouseY;
            eventObj["mouseDeltaX"] = event.mouseDeltaX;
            eventObj["mouseDeltaY"] = event.mouseDeltaY;
            eventObj["mouseButton"] = event.mouseButton;
            eventObj["mouseWheelDelta"] = event.mouseWheelDelta;
            eventObj["keyCode"] = static_cast<int>(event.keyCode);
            eventObj["waitMicroseconds"] = static_cast<qint64>(event.waitMicroseconds);
            
            eventsArray.append(eventObj);
        }
    }
    
    QJsonDocument doc(eventsArray);
    QClipboard* clipboard = QApplication::clipboard();
    clipboard->setText(doc.toJson(QJsonDocument::Compact));
    
    statusLabel_->setText(tr("Copied %1 event(s)").arg(indices.size()));
}

void MainWindow::pasteEvents()
{
    if (!recording_ || !eventModel_) return;
    
    QClipboard* clipboard = QApplication::clipboard();
    QString text = clipboard->text();
    
    if (text.isEmpty()) return;
    
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(text.toUtf8(), &error);
    
    if (error.error != QJsonParseError::NoError || !doc.isArray()) {
        statusLabel_->setText(tr("Clipboard does not contain valid event data"));
        return;
    }
    
    QJsonArray eventsArray = doc.array();
    if (eventsArray.isEmpty()) return;
    
    // Determine insert position
    int insertPosition = eventListView_->selectedEventIndex();
    if (insertPosition < 0) {
        insertPosition = static_cast<int>(recording_->eventCount());
    } else {
        insertPosition++; // Insert after selection
    }
    
    // Calculate timestamp offset to maintain relative timing
    std::uint64_t baseTimestamp = 0;
    if (insertPosition > 0 && insertPosition <= static_cast<int>(recording_->eventCount())) {
        baseTimestamp = recording_->events()[insertPosition - 1].timestampMicroseconds + 1000; // +1ms
    }
    
    std::uint64_t firstPastedTimestamp = 0;
    bool firstEvent = true;
    
    // Parse and insert events
    std::vector<InputEvent> eventsToInsert;
    for (const QJsonValue& val : eventsArray) {
        QJsonObject obj = val.toObject();
        
        InputEvent event;
        event.type = static_cast<EventType>(obj["type"].toInt());
        std::uint64_t originalTimestamp = static_cast<std::uint64_t>(obj["timestamp"].toVariant().toLongLong());
        
        if (firstEvent) {
            firstPastedTimestamp = originalTimestamp;
            firstEvent = false;
        }
        
        // Adjust timestamp relative to insert position
        event.timestampMicroseconds = baseTimestamp + (originalTimestamp - firstPastedTimestamp);
        
        event.mouseX = obj["mouseX"].toInt();
        event.mouseY = obj["mouseY"].toInt();
        event.mouseDeltaX = obj["mouseDeltaX"].toInt();
        event.mouseDeltaY = obj["mouseDeltaY"].toInt();
        event.mouseButton = obj["mouseButton"].toInt();
        event.mouseWheelDelta = obj["mouseWheelDelta"].toInt();
        event.keyCode = static_cast<std::uint32_t>(obj["keyCode"].toInt());
        event.waitMicroseconds = static_cast<std::uint64_t>(obj["waitMicroseconds"].toVariant().toLongLong());
        
        eventsToInsert.push_back(event);
    }
    
    // Use undo stack for batch insert
    undoStack_->beginMacro(tr("Paste %1 Event(s)").arg(eventsToInsert.size()));
    for (size_t i = 0; i < eventsToInsert.size(); ++i) {
        undoStack_->push(new InsertEventCommand(eventModel_, insertPosition + static_cast<int>(i), eventsToInsert[i]));
    }
    undoStack_->endMacro();
    
    // Select pasted events
    eventListView_->clearSelection();
    eventListView_->selectEvent(insertPosition);
    
    statusLabel_->setText(tr("Pasted %1 event(s)").arg(eventsToInsert.size()));
}

void MainWindow::goToEvent()
{
    if (!recording_ || recording_->empty()) return;
    
    int currentEvent = eventListView_->selectedEventIndex() + 1;
    if (currentEvent < 1) currentEvent = 1;
    
    GoToEventDialog dialog(static_cast<int>(recording_->eventCount()), currentEvent, this);
    
    if (dialog.exec() == QDialog::Accepted) {
        int eventNum = dialog.selectedEvent();
        eventListView_->selectEvent(eventNum - 1); // Convert to 0-based
        eventListView_->scrollTo(eventModel_->index(eventNum - 1, 0));
    }
}

void MainWindow::batchTimingOperation()
{
    if (!recording_ || recording_->empty()) return;
    
    BatchTimingDialog dialog(this);
    
    if (dialog.exec() != QDialog::Accepted) return;
    
    QList<int> indices;
    if (dialog.applyToSelection()) {
        indices = eventListView_->selectedEventIndices();
        if (indices.isEmpty()) {
            QMessageBox::information(this, tr("No Selection"), 
                tr("Please select events to apply the timing operation to."));
            return;
        }
    } else {
        // All events
        for (int i = 0; i < static_cast<int>(recording_->eventCount()); ++i) {
            indices.append(i);
        }
    }
    
    // Build old and new event lists for undo
    std::vector<std::pair<int, InputEvent>> oldEvents;
    std::vector<std::pair<int, InputEvent>> newEvents;
    
    const auto& events = recording_->events();
    
    for (int idx : indices) {
        if (idx < 0 || idx >= static_cast<int>(events.size())) continue;
        
        InputEvent oldEvent = events[idx];
        InputEvent newEvent = oldEvent;
        
        // Calculate delay from previous event
        std::uint64_t prevTimestamp = (idx > 0) ? events[idx - 1].timestampMicroseconds : 0;
        std::uint64_t delay = oldEvent.timestampMicroseconds - prevTimestamp;
        
        switch (dialog.operation()) {
            case BatchTimingDialog::Operation::Scale: {
                double factor = dialog.scaleFactor();
                std::uint64_t newDelay = static_cast<std::uint64_t>(delay * factor);
                newEvent.timestampMicroseconds = prevTimestamp + newDelay;
                break;
            }
            case BatchTimingDialog::Operation::Offset: {
                int offsetUs = dialog.offsetMs() * 1000;
                std::int64_t newDelay = static_cast<std::int64_t>(delay) + offsetUs;
                if (newDelay < 0) newDelay = 0;
                newEvent.timestampMicroseconds = prevTimestamp + static_cast<std::uint64_t>(newDelay);
                break;
            }
            case BatchTimingDialog::Operation::SetMinDelay: {
                std::uint64_t minDelayUs = static_cast<std::uint64_t>(dialog.minDelayMs()) * 1000;
                if (delay < minDelayUs) {
                    newEvent.timestampMicroseconds = prevTimestamp + minDelayUs;
                }
                break;
            }
        }
        
        oldEvents.emplace_back(idx, oldEvent);
        newEvents.emplace_back(idx, newEvent);
    }
    
    if (!newEvents.empty()) {
        QString description;
        switch (dialog.operation()) {
            case BatchTimingDialog::Operation::Scale:
                description = tr("Scale Timing x%1").arg(dialog.scaleFactor());
                break;
            case BatchTimingDialog::Operation::Offset:
                description = tr("Offset Timing %1ms").arg(dialog.offsetMs());
                break;
            case BatchTimingDialog::Operation::SetMinDelay:
                description = tr("Set Min Delay %1ms").arg(dialog.minDelayMs());
                break;
        }
        
        undoStack_->push(new BatchModifyCommand(eventModel_, description, oldEvents, newEvents));
        statusLabel_->setText(tr("Applied timing operation to %1 event(s)").arg(newEvents.size()));
    }
}

void MainWindow::exportEvents()
{
    if (!recording_ || recording_->empty()) {
        QMessageBox::information(this, tr("No Events"), tr("There are no events to export."));
        return;
    }
    
    ExportDialog dialog(this);
    
    if (dialog.exec() != QDialog::Accepted) return;
    
    QString filePath = dialog.filePath();
    if (filePath.isEmpty()) {
        QMessageBox::warning(this, tr("No File"), tr("Please specify an output file."));
        return;
    }
    
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, tr("Export Error"), 
            tr("Could not open file for writing:\n%1").arg(file.errorString()));
        return;
    }
    
    QTextStream stream(&file);
    
    // Get events to export
    QList<int> indices;
    if (dialog.exportSelection()) {
        indices = eventListView_->selectedEventIndices();
    } else {
        for (int i = 0; i < static_cast<int>(recording_->eventCount()); ++i) {
            indices.append(i);
        }
    }
    
    const auto& events = recording_->events();
    
    switch (dialog.format()) {
        case ExportDialog::Format::CSV:
        case ExportDialog::Format::TSV: {
            QString sep = (dialog.format() == ExportDialog::Format::CSV) ? "," : "\t";
            
            if (dialog.includeHeaders()) {
                stream << "Index" << sep << "Type" << sep << "Timestamp_us" << sep 
                       << "X" << sep << "Y" << sep << "DeltaX" << sep << "DeltaY" << sep
                       << "Button" << sep << "WheelDelta" << sep << "KeyCode" << sep << "WaitUs\n";
            }
            
            for (int idx : indices) {
                const InputEvent& e = events[idx];
                QString typeName;
                switch (e.type) {
                    case EventType::MouseMove: typeName = "MouseMove"; break;
                    case EventType::MouseButtonDown: typeName = "MouseButtonDown"; break;
                    case EventType::MouseButtonUp: typeName = "MouseButtonUp"; break;
                    case EventType::MouseWheel: typeName = "MouseWheel"; break;
                    case EventType::KeyDown: typeName = "KeyDown"; break;
                    case EventType::KeyUp: typeName = "KeyUp"; break;
                    case EventType::Wait: typeName = "Wait"; break;
                    case EventType::MouseTeleport: typeName = "MouseTeleport"; break;
                }
                
                stream << (idx + 1) << sep << typeName << sep << e.timestampMicroseconds << sep
                       << e.mouseX << sep << e.mouseY << sep << e.mouseDeltaX << sep << e.mouseDeltaY << sep
                       << e.mouseButton << sep << e.mouseWheelDelta << sep << e.keyCode << sep << e.waitMicroseconds << "\n";
            }
            break;
        }
        case ExportDialog::Format::JSON: {
            QJsonArray eventsArray;
            for (int idx : indices) {
                const InputEvent& e = events[idx];
                QJsonObject obj;
                obj["index"] = idx + 1;
                obj["type"] = static_cast<int>(e.type);
                obj["timestamp"] = static_cast<qint64>(e.timestampMicroseconds);
                obj["x"] = e.mouseX;
                obj["y"] = e.mouseY;
                obj["deltaX"] = e.mouseDeltaX;
                obj["deltaY"] = e.mouseDeltaY;
                obj["button"] = e.mouseButton;
                obj["wheelDelta"] = e.mouseWheelDelta;
                obj["keyCode"] = static_cast<int>(e.keyCode);
                obj["waitUs"] = static_cast<qint64>(e.waitMicroseconds);
                eventsArray.append(obj);
            }
            
            QJsonDocument doc(eventsArray);
            stream << doc.toJson(QJsonDocument::Indented);
            break;
        }
    }
    
    file.close();
    statusLabel_->setText(tr("Exported %1 event(s) to %2").arg(indices.size()).arg(QFileInfo(filePath).fileName()));
}

void MainWindow::findEvents()
{
    if (searchWidget_) {
        searchWidget_->setVisible(true);
        searchWidget_->setRecording(recording_.get());
        searchWidget_->focusSearch();
    }
}

void MainWindow::showDisplayInfo()
{
    DisplayInfoDialog dialog(recording_.get(), this);
    dialog.exec();
}

void MainWindow::toggleColorCodedRows(bool enabled)
{
    if (eventModel_) {
        eventModel_->setColorCodedRows(enabled);
    }
}

void MainWindow::toggleStatisticsPanel(bool visible)
{
    if (statisticsPanel_) {
        statisticsPanel_->setVisible(visible);
        if (visible) {
            statisticsPanel_->setRecording(recording_.get());
        }
    }
}

void MainWindow::toggleUndoHistoryPanel(bool visible)
{
    if (undoHistoryPanel_) {
        undoHistoryPanel_->setVisible(visible);
    }
}

void MainWindow::toggleRecordingPanel(bool visible)
{
    if (recordingWidget_) {
        recordingWidget_->setVisible(visible);
    }
}

void MainWindow::startRecording()
{
    // Show recording panel if hidden
    if (recordingWidget_ && !recordingWidget_->isVisible()) {
        recordingWidget_->setVisible(true);
        recordingPanelAction_->setChecked(true);
    }
    
    // Check if we should confirm discarding changes
    if (!confirmDiscardChanges()) {
        return;
    }
    
    // Start recording
    if (recordingWidget_) {
        recordingWidget_->startRecording();
    }
}

void MainWindow::stopRecording()
{
    if (recordingWidget_ && recordingWidget_->isRecording()) {
        recordingWidget_->stopRecording();
    }
}

void MainWindow::onRecordingCompleted(bool hasEvents)
{
    if (!hasEvents) {
        QMessageBox::information(this, tr("Recording"),
            tr("No events were recorded."));
        return;
    }
    
    // Take the recording from the recording widget
    if (recordingWidget_ && recordingWidget_->recordingThread()) {
        auto newRecording = recordingWidget_->recordingThread()->takeRecording();
        if (newRecording && newRecording->eventCount() > 0) {
            *recording_ = std::move(*newRecording);
            currentFilePath_.clear();
            modified_ = true;
            
            undoStack_->clear();
            
            eventModel_->setRecording(recording_.get());
            infoPanel_->setRecording(recording_.get(), "");
            timelineWidget_->setRecording(recording_.get());
            playbackWidget_->setRecording(recording_.get());
            propertyEditor_->clear();
            searchWidget_->setRecording(recording_.get());
            statisticsPanel_->setRecording(recording_.get());
            
            updateWindowTitle();
            updateActionStates();
            
            statusLabel_->setText(tr("Recorded %1 events").arg(recording_->eventCount()));
            
            // Show tray notification
            if (trayManager_) {
                trayManager_->showMessage(tr("Recording Complete"),
                    tr("Recorded %1 events").arg(recording_->eventCount()));
                trayManager_->setIdleState();
            }
        }
    }
}

void MainWindow::setupHotkeys()
{
    // Connect hotkey signals
    connect(hotkeyManager_, &GlobalHotkeyManager::recordStartStopTriggered,
            this, &MainWindow::onRecordStartStopHotkey);
    connect(hotkeyManager_, &GlobalHotkeyManager::recordPauseTriggered,
            this, &MainWindow::onRecordPauseHotkey);
    connect(hotkeyManager_, &GlobalHotkeyManager::playbackStartStopTriggered,
            this, &MainWindow::onPlaybackStartStopHotkey);
    connect(hotkeyManager_, &GlobalHotkeyManager::playbackPauseTriggered,
            this, &MainWindow::onPlaybackPauseHotkey);
    connect(hotkeyManager_, &GlobalHotkeyManager::emergencyStopTriggered,
            this, &MainWindow::onEmergencyStop);
    
    // Connect tray manager signals
    connect(trayManager_, &SystemTrayManager::showWindowRequested, this, [this]() {
        show();
        raise();
        activateWindow();
    });
    connect(trayManager_, &SystemTrayManager::exitRequested, this, &MainWindow::close);
    connect(trayManager_, &SystemTrayManager::quickRecordRequested, this, &MainWindow::startRecording);
    connect(trayManager_, &SystemTrayManager::quickPlayRequested, this, [this]() {
        playbackWidget_->play();
    });
    
    // Register default hotkeys (can be customized in settings)
    QSettings settings("InputPlay", "Studio");
    bool hotkeysEnabled = settings.value("hotkeysEnabled", true).toBool();
    
    if (hotkeysEnabled) {
#ifdef _WIN32
        hotkeyManager_->registerHotkey(GlobalHotkeyManager::RecordStartStop, VK_F9);
        hotkeyManager_->registerHotkey(GlobalHotkeyManager::PlaybackStartStop, VK_F10);
        hotkeyManager_->registerHotkey(GlobalHotkeyManager::PlaybackPause, VK_F11);
        hotkeyManager_->registerHotkey(GlobalHotkeyManager::EmergencyStop, VK_ESCAPE, Qt::ControlModifier | Qt::ShiftModifier);
#endif
        hotkeyManager_->setEnabled(true);
    }
    
    // Connect recording widget signals
    if (recordingWidget_) {
        connect(recordingWidget_, &RecordingWidget::recordingStarted, this, [this]() {
            if (trayManager_) {
                trayManager_->setRecordingState(true, false);
                trayManager_->showMessage(tr("Recording Started"),
                    tr("Recording input events..."));
            }
            updateActionStates();
        });
        
        connect(recordingWidget_, &RecordingWidget::recordingCompleted,
                this, &MainWindow::onRecordingCompleted);
        
        connect(recordingWidget_, &RecordingWidget::recordingCancelled, this, [this]() {
            if (trayManager_) {
                trayManager_->setIdleState();
            }
            statusLabel_->setText(tr("Recording cancelled"));
        });
        
        connect(recordingWidget_, &RecordingWidget::statusChanged,
                statusLabel_, &QLabel::setText);
    }
}

void MainWindow::onRecordStartStopHotkey()
{
    if (recordingWidget_ && recordingWidget_->isRecording()) {
        stopRecording();
    } else {
        startRecording();
    }
}

void MainWindow::onRecordPauseHotkey()
{
    if (recordingWidget_ && recordingWidget_->isRecording()) {
        recordingWidget_->togglePause();
        if (trayManager_) {
            trayManager_->setRecordingState(true, recordingWidget_->isPaused());
        }
    }
}

void MainWindow::onPlaybackStartStopHotkey()
{
    if (playbackWidget_->isPlaying()) {
        playbackWidget_->stop();
        if (trayManager_) {
            trayManager_->setIdleState();
        }
    } else if (recording_ && recording_->eventCount() > 0) {
        playbackWidget_->play();
        if (trayManager_) {
            trayManager_->setPlaybackState(true, false);
        }
    }
}

void MainWindow::onPlaybackPauseHotkey()
{
    if (playbackWidget_->isPlaying()) {
        if (playbackWidget_->isPaused()) {
            playbackWidget_->resume();
            if (trayManager_) {
                trayManager_->setPlaybackState(true, false);
            }
        } else {
            playbackWidget_->pause();
            if (trayManager_) {
                trayManager_->setPlaybackState(true, true);
            }
        }
    }
}

void MainWindow::onEmergencyStop()
{
    // Stop everything immediately
    if (recordingWidget_ && recordingWidget_->isRecording()) {
        recordingWidget_->cancelRecording();
    }
    if (playbackWidget_->isPlaying()) {
        playbackWidget_->stop();
    }
    
    if (trayManager_) {
        trayManager_->setIdleState();
        trayManager_->showMessage(tr("Emergency Stop"),
            tr("All operations stopped."), 1);
    }
    
    statusLabel_->setText(tr("Emergency stop - all operations cancelled"));
}
