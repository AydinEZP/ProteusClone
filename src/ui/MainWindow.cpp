#include "MainWindow.h"
#include "CanvasWidget.h"
#include "LibraryPanel.h"
#include "PropertiesPanel.h"
#include "LogPanel.h"
#include "OscilloscopePanel.h"
#include "HelpDialog.h"
#include <unordered_map>
#include <functional>
#include "NewProjectDialog.h"
#include "ProjectStartDialog.h"
#include "../persistence/ProjectSerializer.h"
#include <QFileInfo>
#include <QDir>

#include <QMenuBar>
#include <QToolBar>
#include <QDockWidget>
#include <QStatusBar>
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>
#include <QLabel>
#include <QAction>
#include <QActionGroup>
#include <QApplication>
#include <QGuiApplication>
#include <QPalette>
#include <QStyleHints>
#include <QIcon>
#include "IconProvider.h"
#include "../domain/components/Oscilloscope.h"
#include <QTimer>
#include <QDialog>
#include <QSizeF>
#include <QSize>

#include <QKeySequence>
#include <QMenu>
#include <QtGlobal>

namespace {
QIcon pcIcon(const QString& name)
{
    return IconProvider::icon(name);
}

QPalette lightThemePalette()
{
    QPalette p;
    p.setColor(QPalette::Window, QColor(240, 240, 240));
    p.setColor(QPalette::WindowText, QColor(32, 32, 32));
    p.setColor(QPalette::Base, Qt::white);
    p.setColor(QPalette::AlternateBase, QColor(247, 247, 247));
    p.setColor(QPalette::ToolTipBase, QColor(255, 255, 220));
    p.setColor(QPalette::ToolTipText, Qt::black);
    p.setColor(QPalette::Text, QColor(32, 32, 32));
    p.setColor(QPalette::Button, QColor(240, 240, 240));
    p.setColor(QPalette::ButtonText, QColor(32, 32, 32));
    p.setColor(QPalette::BrightText, Qt::red);
    p.setColor(QPalette::Link, QColor(35, 100, 200));
    p.setColor(QPalette::Highlight, QColor(61, 126, 255));
    p.setColor(QPalette::HighlightedText, Qt::white);
    p.setColor(QPalette::Disabled, QPalette::Text, QColor(135, 135, 135));
    p.setColor(QPalette::Disabled, QPalette::WindowText, QColor(135, 135, 135));
    p.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(135, 135, 135));
    return p;
}

QPalette darkThemePalette()
{
    QPalette p;
    p.setColor(QPalette::Window, QColor(37, 39, 42));
    p.setColor(QPalette::WindowText, QColor(242, 242, 242));
    p.setColor(QPalette::Base, QColor(27, 29, 32));
    p.setColor(QPalette::AlternateBase, QColor(46, 49, 54));
    p.setColor(QPalette::ToolTipBase, QColor(46, 49, 54));
    p.setColor(QPalette::ToolTipText, QColor(245, 245, 245));
    p.setColor(QPalette::Text, QColor(242, 242, 242));
    p.setColor(QPalette::Button, QColor(50, 53, 58));
    p.setColor(QPalette::ButtonText, QColor(242, 242, 242));
    p.setColor(QPalette::BrightText, QColor(255, 96, 96));
    p.setColor(QPalette::Link, QColor(106, 169, 255));
    p.setColor(QPalette::Highlight, QColor(61, 126, 255));
    p.setColor(QPalette::HighlightedText, Qt::white);
    p.setColor(QPalette::Disabled, QPalette::Text, QColor(125, 125, 125));
    p.setColor(QPalette::Disabled, QPalette::WindowText, QColor(125, 125, 125));
    p.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(125, 125, 125));
    return p;
}

QString applicationThemeStyleSheet(bool dark)
{
    // Explicit widget text colors are used in addition to QPalette because some
    // native platform styles do not repaint existing text reliably after a
    // runtime palette change. Keeping the colors here guarantees that menu,
    // dock, dialog, label and editor text all switch immediately.
    const QString windowText = dark ? "#F2F2F2" : "#202020";
    const QString baseText   = dark ? "#F2F2F2" : "#202020";
    const QString disabled   = dark ? "#7D7D7D" : "#878787";
    const QString base       = dark ? "#1B1D20" : "#FFFFFF";
    const QString button     = dark ? "#32353A" : "#F0F0F0";
    const QString border     = dark ? "#5A5D63" : "#B8B8B8";

    return QString(R"QSS(
QWidget { color: %1; }
QLabel, QGroupBox, QDockWidget, QStatusBar, QMenuBar, QMenu, QToolTip { color: %1; }
QLineEdit, QTextEdit, QPlainTextEdit, QTextBrowser, QListView, QListWidget,
QTreeView, QTreeWidget, QTableView, QTableWidget, QSpinBox, QDoubleSpinBox,
QComboBox { color: %2; background-color: %4; }
QPushButton, QToolButton { color: %1; background-color: %5; }
QMenuBar::item, QMenu::item { color: %1; }
QMenuBar::item:disabled, QMenu::item:disabled, QWidget:disabled { color: %3; }
QGroupBox { border-color: %6; }
)QSS").arg(windowText)
          .arg(baseText)
          .arg(disabled)
          .arg(base)
          .arg(button)
          .arg(border);
}
}
MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    m_systemPalette = qApp->palette();

    setWindowTitle("ProteusClone – OOP Circuit Simulator");
    setWindowIcon(pcIcon("app"));

    // Simulation engine (holds pointer to graph)
    m_simEngine = std::make_unique<SimulationEngine>(&m_graph);

    // Create central canvas
    m_canvas = new CanvasWidget(&m_graph, &m_cmdMgr, this);
    setCentralWidget(m_canvas);

    setupMenus();
    setupToolBar();
    setupDocks();

    // Status bar coord label
    m_coordLabel = new QLabel("(0, 0)", this);
    statusBar()->addPermanentWidget(m_coordLabel);
    statusBar()->showMessage("Ready");

    const QString savedTheme = QSettings().value("appearance/theme", "system").toString();
    applyTheme(savedTheme, false);

#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
    connect(QGuiApplication::styleHints(), &QStyleHints::colorSchemeChanged,
            this, [this](Qt::ColorScheme) {
        if (m_themeMode == "system") applyTheme("system", false);
    });
#endif

    // Connections
    connect(m_canvas, &CanvasWidget::componentSelected,
            this, &MainWindow::onComponentSelected);
    connect(m_canvas, &CanvasWidget::mouseWorldPosChanged,
            this, &MainWindow::onMouseWorldPos);
    // Double-click → show properties panel and focus it
    connect(m_canvas, &CanvasWidget::componentDoubleClicked,
            this, &MainWindow::onComponentDoubleClicked);
    // Click on Switch/PushButton → refresh canvas (state already toggled)
    connect(m_canvas, &CanvasWidget::componentInteracted,
            this, &MainWindow::onComponentInteracted);
    connect(m_simEngine.get(), &SimulationEngine::tickDone,
            this, &MainWindow::onSimulationTick);
    connect(m_simEngine.get(), &SimulationEngine::stateChanged,
            this, &MainWindow::onSimulationStateChanged);
    connect(m_props, &PropertiesPanel::propertyChanged,
            this, &MainWindow::onPropertyChanged);

    resize(1280, 800);
    QTimer::singleShot(0, this, &MainWindow::showStartDialog);
}

MainWindow::~MainWindow() = default;

// ── Menu / Toolbar setup ───────────────────────────────────────────────────────

// Helper: create a QAction with icon, text, shortcut and connect it — Qt 6.11 compatible
static QAction* makeAction(const QIcon& icon, const QString& text,
                            const QKeySequence& shortcut,
                            QObject* receiver, std::function<void()> slot,
                            QObject* parent)
{
    auto* act = new QAction(icon, text, parent);
    if (!shortcut.isEmpty())
        act->setShortcut(shortcut);
    QObject::connect(act, &QAction::triggered, receiver, slot);
    return act;
}

void MainWindow::setupMenus()
{
    auto fileMenu = menuBar()->addMenu("&File");

    fileMenu->addAction(makeAction(pcIcon("new"),  "&New",          QKeySequence::New,
                                   this, [this]{ newProject(); },   this));
    fileMenu->addAction(makeAction(pcIcon("open"), "&Open...",       QKeySequence::Open,
                                   this, [this]{ openProject(); },  this));
    fileMenu->addAction(makeAction(pcIcon("save"), "&Save",          QKeySequence::Save,
                                   this, [this]{ saveProject(); },  this));
    fileMenu->addAction(makeAction(pcIcon("save_as"), "Save &As...", QKeySequence(),
                                   this, [this]{ saveProjectAs(); },this));
    fileMenu->addAction(makeAction(pcIcon("export"), "Export &Image...", QKeySequence(),
                                   this, [this]{ exportImage(); },  this));
    fileMenu->addSeparator();
    m_recentMenu = fileMenu->addMenu(pcIcon("open"), "Recent Projects");
    updateRecentProjectsMenu();
    fileMenu->addSeparator();

    auto* exitAct = new QAction("E&xit", this);
    exitAct->setShortcut(QKeySequence::Quit);
    connect(exitAct, &QAction::triggered, this, &QWidget::close);
    fileMenu->addAction(exitAct);

    auto editMenu = menuBar()->addMenu("&Edit");
    m_undoAction = makeAction(pcIcon("undo"), "&Undo", QKeySequence::Undo,
                               this, [this]{ undo(); }, this);
    m_redoAction = makeAction(pcIcon("redo"), "&Redo", QKeySequence::Redo,
                               this, [this]{ redo(); }, this);
    editMenu->addAction(m_undoAction);
    editMenu->addAction(m_redoAction);

    auto viewMenu = menuBar()->addMenu("&View");
    auto themeMenu = viewMenu->addMenu("&Theme");
    auto* themeGroup = new QActionGroup(this);
    themeGroup->setExclusive(true);

    m_themeSystemAction = themeMenu->addAction("System (Automatic)");
    m_themeLightAction  = themeMenu->addAction("Light");
    m_themeDarkAction   = themeMenu->addAction("Dark");
    for (QAction* action : {m_themeSystemAction, m_themeLightAction, m_themeDarkAction}) {
        action->setCheckable(true);
        themeGroup->addAction(action);
    }
    connect(m_themeSystemAction, &QAction::triggered, this, [this]{ applyTheme("system"); });
    connect(m_themeLightAction,  &QAction::triggered, this, [this]{ applyTheme("light"); });
    connect(m_themeDarkAction,   &QAction::triggered, this, [this]{ applyTheme("dark"); });

    auto simMenu = menuBar()->addMenu("&Simulation");
    simMenu->addAction(makeAction(pcIcon("run"),   "Run",   QKeySequence(Qt::Key_F5),
                                   this, [this]{ runSimulation(); },   this));
    simMenu->addAction(makeAction(pcIcon("pause"), "Pause", QKeySequence(Qt::Key_F6),
                                   this, [this]{ pauseSimulation(); }, this));
    simMenu->addAction(makeAction(pcIcon("stop"),  "Stop",  QKeySequence(Qt::Key_F7),
                                   this, [this]{ stopSimulation(); },  this));
    simMenu->addAction(makeAction(pcIcon("step"),  "Step",  QKeySequence(Qt::Key_F8),
                                   this, [this]{ stepSimulation(); },  this));
    simMenu->addSeparator();
    simMenu->addAction(makeAction(pcIcon("drc"), "Run &DRC", QKeySequence(),
                                   this, [this]{ runDRC(); }, this));

    auto helpMenu = menuBar()->addMenu("&Help");
    helpMenu->addAction(makeAction(pcIcon("help"), "&User Guide", QKeySequence(Qt::Key_F1),
                                   this, [this]{ showUserGuide(); }, this));
    helpMenu->addSeparator();
    helpMenu->addAction(makeAction(pcIcon("app"), "&About ProteusClone", QKeySequence(),
                                   this, [this]{ showAboutDialog(); }, this));
}

bool MainWindow::systemPrefersDark() const
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
    const Qt::ColorScheme scheme = QGuiApplication::styleHints()->colorScheme();
    if (scheme == Qt::ColorScheme::Dark) return true;
    if (scheme == Qt::ColorScheme::Light) return false;
#endif
    return m_systemPalette.color(QPalette::Window).lightness() < 128;
}

void MainWindow::applyTheme(const QString& mode, bool saveSetting)
{
    QString normalized = mode.trimmed().toLower();
    if (normalized != "system" && normalized != "light" && normalized != "dark")
        normalized = "system";

    m_themeMode = normalized;
    const bool useDark = (normalized == "dark") ||
                         (normalized == "system" && systemPrefersDark());

    qApp->setPalette(useDark ? darkThemePalette() : lightThemePalette());
    qApp->setStyleSheet(applicationThemeStyleSheet(useDark));

    // Force custom-painted widgets and any cached editor viewport to repaint now.
    for (QWidget* widget : qApp->allWidgets()) {
        if (widget) widget->update();
    }

    if (m_themeSystemAction) m_themeSystemAction->setChecked(normalized == "system");
    if (m_themeLightAction)  m_themeLightAction->setChecked(normalized == "light");
    if (m_themeDarkAction)   m_themeDarkAction->setChecked(normalized == "dark");

    if (saveSetting)
        QSettings().setValue("appearance/theme", normalized);

    if (m_canvas) m_canvas->repaintCanvas();
    if (statusBar()) {
        const QString shown = normalized == "system"
            ? QString("System theme (%1)").arg(useDark ? "Dark" : "Light")
            : QString("%1 theme").arg(useDark ? "Dark" : "Light");
        statusBar()->showMessage(shown, 2000);
    }
}

void MainWindow::setupToolBar()
{
    auto* tb = addToolBar("Main");
    tb->setIconSize(QSize(22, 22));

    // Toolbar uses QAction::triggered → lambda, no deprecated overloads
    auto tbAct = [&](const QString& iconName, const QString& tip,
                      std::function<void()> slot) -> QAction* {
        auto* a = new QAction(pcIcon(iconName), tip, tb);
        a->setToolTip(tip);
        connect(a, &QAction::triggered, this, slot);
        tb->addAction(a);
        return a;
    };
    auto tbActObj = [&](const QString& iconName, const QString& tip,
                         QObject* recv, std::function<void()> slot) {
        auto* a = new QAction(pcIcon(iconName), tip, tb);
        connect(a, &QAction::triggered, recv, slot);
        tb->addAction(a);
    };

    tbAct("new",  "New Project",    [this]{ newProject(); });
    tbAct("open", "Open Project",   [this]{ openProject(); });
    tbAct("save", "Save Project",   [this]{ saveProject(); });
    tb->addSeparator();
    tbAct("undo", "Undo (Ctrl+Z)",  [this]{ undo(); });
    tbAct("redo", "Redo (Ctrl+Y)",  [this]{ redo(); });
    tb->addSeparator();
    tbAct("run",   "Run (F5)",      [this]{ runSimulation(); });
    tbAct("pause", "Pause (F6)",    [this]{ pauseSimulation(); });
    tbAct("stop",  "Stop (F7)",     [this]{ stopSimulation(); });
    tbAct("step",  "Step (F8)",     [this]{ stepSimulation(); });
    tb->addSeparator();
    tbAct("drc",  "Run DRC",        [this]{ runDRC(); });
    tbActObj("wire", "Draw Wire (W)", m_canvas, [this]{ m_canvas->enterWireMode(); });
}

void MainWindow::setupDocks()
{
    // Library (left)
    m_library = new LibraryPanel(this);
    auto libDock = new QDockWidget("Library", this);
    libDock->setWidget(m_library);
    libDock->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    addDockWidget(Qt::LeftDockWidgetArea, libDock);

    connect(m_library, &LibraryPanel::componentSelected,
            m_canvas, &CanvasWidget::enterPlacementMode);

    // Properties (right)
    m_props = new PropertiesPanel(this);
    auto propDock = new QDockWidget("Properties", this);
    propDock->setWidget(m_props);
    propDock->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    addDockWidget(Qt::RightDockWidgetArea, propDock);

    // Log (bottom)
    m_logPanel = new LogPanel(this);
    auto logDock = new QDockWidget("Log / DRC", this);
    logDock->setWidget(m_logPanel);
    logDock->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    addDockWidget(Qt::BottomDockWidgetArea, logDock);

    // Dedicated oscilloscope panel (bottom). The oscilloscope component remains on the canvas,
    // but this dock provides the full-size plot and Time/Div / Volt/Div controls required by
    // the project specification.
    m_scopePanel = new OscilloscopePanel(this);
    m_scopeDock = new QDockWidget("Oscilloscope", this);
    m_scopeDock->setObjectName("OscilloscopeDock");
    m_scopeDock->setWidget(m_scopePanel);
    m_scopeDock->setFeatures(QDockWidget::DockWidgetMovable
                           | QDockWidget::DockWidgetFloatable
                           | QDockWidget::DockWidgetClosable);
    addDockWidget(Qt::BottomDockWidgetArea, m_scopeDock);
    tabifyDockWidget(logDock, m_scopeDock);

    // The scope panel is intentionally closed by default. Selecting an
    // Oscilloscope component on the schematic reopens and raises this dock.
    m_scopeDock->hide();
}

// ── File actions ───────────────────────────────────────────────────────────────

void MainWindow::newProject()
{
    NewProjectDialog dlg(this);
    if (dlg.exec() != QDialog::Accepted) return;
    m_graph.clear();
    m_graph.setCanvasSize(QSizeF(dlg.canvasWidth(), dlg.canvasHeight()));
    m_currentFile.clear();
    m_logPanel->clear();
    setWindowTitle(QString("ProteusClone – %1 [%2x%3]").arg(dlg.projectName()).arg(dlg.canvasWidth()).arg(dlg.canvasHeight()));
    m_canvas->repaintCanvas();
}

void MainWindow::openProject()
{
    QString path = QFileDialog::getOpenFileName(this, "Open Project", {},
                                                 "ProteusClone Files (*.pcj);;All Files (*)");
    if (path.isEmpty()) return;
    QString err = ProjectSerializer::load(m_graph, path);
    if (!err.isEmpty()) {
        QMessageBox::critical(this, "Load Error", err);
        return;
    }
    m_currentFile = path;
    addRecentProject(path);
    setWindowTitle(QString("ProteusClone – %1").arg(path));
    m_canvas->repaintCanvas();
}

void MainWindow::saveProject()
{
    if (m_currentFile.isEmpty()) { saveProjectAs(); return; }
    QString err = ProjectSerializer::save(m_graph, m_currentFile);
    if (!err.isEmpty()) QMessageBox::critical(this, "Save Error", err);
    else statusBar()->showMessage("Saved.", 2000);
}

void MainWindow::saveProjectAs()
{
    QString path = QFileDialog::getSaveFileName(this, "Save Project As", {},
                                                 "ProteusClone Files (*.pcj);;All Files (*)");
    if (path.isEmpty()) return;
    if (!path.endsWith(".pcj")) path += ".pcj";
    if (QFileInfo::exists(path) && path != m_currentFile) {
        auto reply = QMessageBox::question(this, "Overwrite Project?",
                                           "A project with this name already exists. Overwrite it?",
                                           QMessageBox::Yes | QMessageBox::Cancel,
                                           QMessageBox::Cancel);
        if (reply != QMessageBox::Yes) return;
    }
    m_currentFile = path;
    addRecentProject(path);
    saveProject();
    setWindowTitle(QString("ProteusClone – %1").arg(path));
}

void MainWindow::exportImage()
{
    QString path = QFileDialog::getSaveFileName(this, "Export Image", {},
                                                 "PNG Images (*.png)");
    if (path.isEmpty()) return;
    m_canvas->exportImage(path);
    statusBar()->showMessage("Image exported.", 2000);
}

// ── Simulation ─────────────────────────────────────────────────────────────────

void MainWindow::runSimulation()
{
    m_logPanel->clear();
    m_simEngine->start();
}

void MainWindow::pauseSimulation()  { m_simEngine->pause(); }
void MainWindow::stopSimulation()   { m_simEngine->stop(); }
void MainWindow::stepSimulation()   { m_simEngine->step(); }

void MainWindow::onSimulationTick(QStringList warnings,
                                   std::unordered_map<WireID,int> netValues)
{
    if (!warnings.isEmpty())
        m_logPanel->appendMessages(warnings);
    // Push wire net values so canvas draws animated wire colors
    m_canvas->setNetValues(netValues);
    if (m_scopePanel) m_scopePanel->refresh();
    m_canvas->repaintCanvas();
}

void MainWindow::onSimulationStateChanged(SimState state)
{
    QString s;
    switch (state) {
        case SimState::Running: s = "Simulation RUNNING";  break;
        case SimState::Paused:  s = "Simulation PAUSED";   break;
        case SimState::Stopped:
            s = "Simulation STOPPED";
            // Clear wire colors when simulation stops
            m_canvas->clearNetValues();
            if (m_scopePanel) m_scopePanel->refresh();
            m_canvas->repaintCanvas();
            break;
    }
    statusBar()->showMessage(s, 3000);
}

void MainWindow::onComponentDoubleClicked(std::shared_ptr<Component> comp)
{
    // Show the component in PropertiesPanel and ensure the dock is visible
    m_props->showComponent(comp);
    if (auto scope = std::dynamic_pointer_cast<Oscilloscope>(comp)) {
        m_scopePanel->setScope(scope);
        if (m_scopeDock) {
            m_scopeDock->show();
            m_scopeDock->raise();
        }
    }
    // Raise the properties dock if it's tabbed/hidden
    if (auto* dock = qobject_cast<QDockWidget*>(m_props->parentWidget()))
        dock->raise();
}

void MainWindow::onComponentInteracted(std::shared_ptr<Component> /*comp*/)
{
    // Just repaint so Switch/PushButton visual state updates immediately
    m_canvas->repaintCanvas();
}

// ── Edit ───────────────────────────────────────────────────────────────────────

void MainWindow::undo()
{
    m_cmdMgr.undo();
    m_canvas->repaintCanvas();
}

void MainWindow::redo()
{
    m_cmdMgr.redo();
    m_canvas->repaintCanvas();
}

void MainWindow::runDRC()
{
    m_graph.buildNetlist();
    QStringList warnings = m_graph.runDRC();
    m_logPanel->clear();
    if (warnings.isEmpty())
        m_logPanel->appendMessages({"DRC passed: no issues found."});
    else
        m_logPanel->appendMessages(warnings);
}

// ── Signals ────────────────────────────────────────────────────────────────────

void MainWindow::onComponentSelected(std::shared_ptr<Component> comp)
{
    m_props->showComponent(comp);
    if (auto scope = std::dynamic_pointer_cast<Oscilloscope>(comp)) {
        m_scopePanel->setScope(scope);
        if (m_scopeDock) {
            m_scopeDock->show();
            m_scopeDock->raise();
        }
    }
}

void MainWindow::onPropertyChanged(ComponentID id, const QString& key, const QString& value)
{
    auto comp = m_graph.componentById(id);
    if (comp) {
        comp->setProperty(key, value);
        m_canvas->repaintCanvas();
    }
}

void MainWindow::onMouseWorldPos(QPointF pos)
{
    m_coordLabel->setText(QString("(%1, %2)")
                          .arg(qRound(pos.x()))
                          .arg(qRound(pos.y())));
}


void MainWindow::showStartDialog()
{
    ProjectStartDialog dlg(this);
    if (dlg.exec() != QDialog::Accepted) return;
    switch (dlg.choice()) {
        case ProjectStartDialog::Choice::NewProject:
            newProject();
            break;
        case ProjectStartDialog::Choice::OpenProject:
            openProject();
            break;
        case ProjectStartDialog::Choice::RecentProject: {
            QString path = dlg.recentPath();
            QString err = ProjectSerializer::load(m_graph, path);
            if (!err.isEmpty()) QMessageBox::critical(this, "Load Error", err);
            else { m_currentFile = path; addRecentProject(path); setWindowTitle(QString("ProteusClone – %1").arg(path)); m_canvas->repaintCanvas(); }
            break;
        }
        case ProjectStartDialog::Choice::None:
            break;
    }
}


// ── Help ──────────────────────────────────────────────────────────────────────

void MainWindow::showUserGuide()
{
    HelpDialog dlg(this);
    dlg.exec();
}

void MainWindow::showAboutDialog()
{
    QMessageBox::about(this, "About ProteusClone",
        "<h3>ProteusClone</h3>"
        "<p>Educational Proteus-like circuit design and simulation application.</p>"
        "<p>Features include schematic editing, 90-degree wiring, junctions, "
        "analog/digital simulation, measurement instruments, oscilloscope, "
        "project save/load, and OOP component architecture.</p>"
        "<p>Use <b>Help &gt; User Guide</b> or press <b>F1</b> for full usage instructions.</p>");
}

// ── Recent projects ────────────────────────────────────────────────────────────

void MainWindow::updateRecentProjectsMenu()
{
    if (!m_recentMenu) return;
    m_recentMenu->clear();
    QSettings s;
    QStringList recent = s.value("recentProjects").toStringList();
    for (auto& path : recent) {
        auto act = m_recentMenu->addAction(path, this, [this, path](){
            QString err = ProjectSerializer::load(m_graph, path);
            if (!err.isEmpty()) QMessageBox::critical(this, "Load Error", err);
            else { m_currentFile = path; m_canvas->repaintCanvas(); }
        });
        Q_UNUSED(act);
    }
    if (recent.isEmpty())
        m_recentMenu->addAction("(none)")->setEnabled(false);
}

void MainWindow::addRecentProject(const QString& path)
{
    QSettings s;
    QStringList recent = s.value("recentProjects").toStringList();
    recent.removeAll(path);
    recent.prepend(path);
    while (recent.size() > 5) recent.removeLast();
    s.setValue("recentProjects", recent);
    updateRecentProjectsMenu();
}
