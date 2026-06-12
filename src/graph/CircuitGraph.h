#pragma once
#include <QPointF>
#include <QSizeF>
#include <vector>
#include <memory>
#include <QString>
#include <QStringList>
#include "NetNode.h"
#include "../domain/Component.h"
#include "../domain/Wire.h"
#include "../domain/Junction.h"

/**
 * Stores all elements and derives a simplified netlist.
 * It also keeps canvas/project metadata needed by the start menu and serializer.
 */
class CircuitGraph {
public:
    CircuitGraph() = default;

    void setCanvasSize(QSizeF s) { m_canvasSize = s; }
    QSizeF canvasSize() const { return m_canvasSize; }

    void addComponent (std::shared_ptr<Component>  c);
    void removeComponent(ComponentID id);

    void addWire    (std::shared_ptr<Wire>     w);
    void removeWire (WireID id);

    void addJunction   (std::shared_ptr<Junction>  j);
    void removeJunction(JunctionID id);
    void autoDetectJunctions();

    void clear();

    const std::vector<std::shared_ptr<Component>>& components() const { return m_components; }
    const std::vector<std::shared_ptr<Wire>>&      wires()      const { return m_wires; }
    const std::vector<std::shared_ptr<Junction>>&  junctions()  const { return m_junctions; }

    std::shared_ptr<Component>  componentById(ComponentID id)   const;
    std::shared_ptr<Wire>       wireById(WireID id)             const;
    std::shared_ptr<Junction>   junctionById(JunctionID id)     const;
    std::shared_ptr<Pin> pinAt(QPointF worldPoint) const;

    void buildNetlist();
    const std::vector<NetNode>& netNodes() const { return m_netNodes; }
    QStringList runDRC() const;

private:
    bool pointAlreadyHasJunction(QPointF p, double tolerance = 1.0) const;
    static bool axisAlignedIntersection(QPointF a1, QPointF a2, QPointF b1, QPointF b2, QPointF* out);

    QSizeF m_canvasSize {1600, 1000};
    std::vector<std::shared_ptr<Component>>  m_components;
    std::vector<std::shared_ptr<Wire>>       m_wires;
    std::vector<std::shared_ptr<Junction>>   m_junctions;
    std::vector<NetNode>                     m_netNodes;
};
