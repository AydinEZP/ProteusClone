#pragma once
#include <QPainter>
#include <QPaintEvent>
#include <QKeyEvent>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QWidget>
#include <QPointF>
#include <QTransform>
#include <memory>
#include <vector>
#include <unordered_map>
#include <optional>
#include "../graph/CircuitGraph.h"
#include "../commands/CommandManager.h"
#include "../persistence/ProjectSerializer.h"

class CanvasWidget : public QWidget {
    Q_OBJECT
public:
    explicit CanvasWidget(CircuitGraph* graph, CommandManager* cmdMgr,
                          QWidget* parent = nullptr);

    void enterPlacementMode(const QString& componentType);
    void enterSelectionMode();
    void enterWireMode();
    void exportImage(const QString& filePath);
    void repaintCanvas() { update(); }

    // Called by MainWindow after each simulation tick to update wire colors
    void setNetValues(const std::unordered_map<WireID, int>& netValues);
    // Called on simulation stop to clear wire colors
    void clearNetValues();

signals:
    void componentSelected(std::shared_ptr<Component> comp);
    void mouseWorldPosChanged(QPointF worldPos);
    // Emitted when user double-clicks a component (for Properties popup)
    void componentDoubleClicked(std::shared_ptr<Component> comp);
    // Emitted when user interacts with Switch, PushButton, or Keypad
    void componentInteracted(std::shared_ptr<Component> comp);

protected:
    void paintEvent(QPaintEvent*) override;
    void mousePressEvent(QMouseEvent*) override;
    void mouseDoubleClickEvent(QMouseEvent*) override;
    void mouseMoveEvent(QMouseEvent*) override;
    void mouseReleaseEvent(QMouseEvent*) override;
    void wheelEvent(QWheelEvent*) override;
    void keyPressEvent(QKeyEvent*) override;

private:
    QPointF screenToWorld(QPointF screen) const;
    QPointF worldToScreen(QPointF world)  const;
    QPointF snapToGrid(QPointF world)     const;

    void drawGrid(QPainter& p);
    void drawCanvasBounds(QPainter& p);
    void drawComponents(QPainter& p);
    void drawWires(QPainter& p);
    void drawJunctions(QPainter& p);
    void drawWireInProgress(QPainter& p);
    void drawSelectionRect(QPainter& p);
    void drawGhostComponent(QPainter& p);

    void selectAt(QPointF worldPos);
    std::shared_ptr<Component> componentAt(QPointF worldPos) const;
    std::shared_ptr<Wire> wireAt(QPointF worldPos) const;
    bool completeWireTo(const std::shared_ptr<Pin>& endPin);
    std::vector<QPointF> currentWirePreviewPath(QPointF end) const;
    QPointF lastWireAnchor() const;
    void addExplicitJunctionAt(QPointF worldPos);
    void rerouteAllWires();
    void deselectAll();
    void deleteSelected();

    std::shared_ptr<Component> createComponent(const QString& type);

    // Wire color from simulation net value (-1=float, 0=LOW, 1=HIGH)
    QColor wireColorFromNetValue(int netVal, bool selected) const;

    CircuitGraph*   m_graph;
    CommandManager* m_cmdMgr;

    QPointF m_panOffset   {0, 0};
    double  m_zoom        {1.0};
    static constexpr double GridSize = 20.0;

    enum class CanvasMode { Select, Place, Wire };
    CanvasMode m_mode {CanvasMode::Select};

    QString m_placeType;
    QPointF m_ghostPos;

    std::shared_ptr<Component> m_selectedComp;
    std::vector<std::shared_ptr<Component>> m_selectedComponents;
    std::shared_ptr<Wire>      m_selectedWire;
    bool                       m_draggingSelect {false};
    QPointF                    m_selectStart;
    QPointF                    m_selectEnd;

    bool    m_draggingComp {false};
    QPointF m_dragStartWorld;
    QPointF m_compOriginalPos;
    bool    m_didDrag {false};  // tracks if actual movement happened

    bool    m_panning   {false};
    QPointF m_panStart;

    std::shared_ptr<Pin> m_wireStartPin;
    QPointF              m_wireCurrentPos;
    std::vector<QPointF> m_wireWaypoints;
    QPointF              m_lastMouseWorld {0, 0};

    // net value per wire id: -1=float, 0=LOW, 1=HIGH
    std::unordered_map<WireID, int> m_netValues;
    bool m_simActive {false};
};
