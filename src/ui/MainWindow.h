#pragma once
#include <QMenu>
#include <QAction>
#include <QMainWindow>
#include <QLabel>
#include <QPalette>
#include <unordered_map>
#include <memory>
#include "../graph/CircuitGraph.h"
#include "../commands/CommandManager.h"
#include "../simulation/SimulationEngine.h"
#include "../domain/Wire.h"

class CanvasWidget;
class LibraryPanel;
class PropertiesPanel;
class LogPanel;
class OscilloscopePanel;
class QDockWidget;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    void newProject();
    void openProject();
    void saveProject();
    void saveProjectAs();
    void exportImage();

    void runSimulation();
    void pauseSimulation();
    void stopSimulation();

    void onComponentSelected(std::shared_ptr<Component> comp);
    void onComponentDoubleClicked(std::shared_ptr<Component> comp);
    void onComponentInteracted(std::shared_ptr<Component> comp);
    void onPropertyChanged(ComponentID id, const QString& key, const QString& value);
    void onMouseWorldPos(QPointF pos);
    void onSimulationTick(QStringList warnings,
                          std::unordered_map<WireID,int> netValues);
    void onSimulationStateChanged(SimState state);

    void undo();
    void redo();
    void runDRC();
    void stepSimulation();
    void showStartDialog();
    void showUserGuide();
    void showAboutDialog();

private:
    void setupMenus();
    void setupToolBar();
    void setupDocks();
    void applyTheme(const QString& mode, bool saveSetting = true);
    bool systemPrefersDark() const;
    void updateRecentProjectsMenu();
    void addRecentProject(const QString& path);

    CircuitGraph     m_graph;
    CommandManager   m_cmdMgr;

    CanvasWidget*     m_canvas    {nullptr};
    LibraryPanel*     m_library   {nullptr};
    PropertiesPanel*  m_props     {nullptr};
    LogPanel*         m_logPanel  {nullptr};
    OscilloscopePanel* m_scopePanel{nullptr};
    QDockWidget*        m_scopeDock {nullptr};
    QLabel*           m_coordLabel{nullptr};

    std::unique_ptr<SimulationEngine> m_simEngine;

    QString  m_currentFile;
    QMenu*   m_recentMenu {nullptr};

    QAction* m_undoAction {nullptr};
    QAction* m_redoAction {nullptr};

    QPalette m_systemPalette;
    QString  m_themeMode {"system"};
    QAction* m_themeSystemAction {nullptr};
    QAction* m_themeLightAction  {nullptr};
    QAction* m_themeDarkAction   {nullptr};
};
