#include "CanvasWidget.h"
#include <QPainter>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QKeyEvent>
#include <QPixmap>
#include <QFileDialog>
#include <QLineF>
#include <algorithm>
#include <QtGlobal>

#include "../domain/components/Resistor.h"
#include "../domain/components/Capacitor.h"
#include "../domain/components/Inductor.h"
#include "../domain/components/DCVoltageSource.h"
#include "../domain/components/Battery.h"
#include "../domain/components/Ground.h"
#include "../domain/components/ClockGenerator.h"
#include "../domain/components/Switch.h"
#include "../domain/components/PushButton.h"
#include "../domain/components/LED.h"
#include "../domain/components/SevenSegment.h"
#include "../domain/components/AndGate.h"
#include "../domain/components/OrGate.h"
#include "../domain/components/NotGate.h"
#include "../domain/components/XorGate.h"
#include "../domain/components/NandGate.h"
#include "../domain/components/DFlipFlop.h"
#include "../domain/components/SimpleADC.h"
#include "../domain/components/SimpleDAC.h"
#include "../domain/components/LCD16x2.h"
#include "../domain/components/Keypad.h"
#include "../domain/components/VoltageProbe.h"
#include "../domain/components/Voltmeter.h"
#include "../domain/components/Ammeter.h"
#include "../domain/components/Oscilloscope.h"
#include "../domain/components/Microcontroller.h"
#include "../domain/components/Potentiometer.h"
#include "../domain/components/ExternalMemory.h"
#include "../commands/AddComponentCommand.h"
#include "../commands/MoveComponentCommand.h"
#include "../commands/DeleteComponentCommand.h"
#include "../commands/AddWireCommand.h"
#include "../commands/DeleteWireCommand.h"

CanvasWidget::CanvasWidget(CircuitGraph* graph, CommandManager* cmdMgr, QWidget* parent)
    : QWidget(parent), m_graph(graph), m_cmdMgr(cmdMgr)
{
    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);
    setMinimumSize(400, 300);
    setStyleSheet("background-color: #f8f8f8;");
}

// ── Simulation wire color API ─────────────────────────────────────────────────

void CanvasWidget::setNetValues(const std::unordered_map<WireID, int>& netValues)
{
    m_netValues  = netValues;
    m_simActive  = true;
    update();
}

void CanvasWidget::clearNetValues()
{
    m_netValues.clear();
    m_simActive = false;
    update();
}

QColor CanvasWidget::wireColorFromNetValue(int netVal, bool selected) const
{
    if (selected)  return Qt::cyan;
    if (!m_simActive) return Qt::darkGreen;
    switch (netVal) {
        case 1:  return QColor(220, 30, 30);   // HIGH → red
        case 0:  return QColor(30, 60, 220);   // LOW  → blue
        default: return QColor(140,140,140);   // float → grey
    }
}

// ── Coordinate conversion ─────────────────────────────────────────────────────

QPointF CanvasWidget::screenToWorld(QPointF screen) const
{
    return (screen - m_panOffset) / m_zoom;
}

QPointF CanvasWidget::worldToScreen(QPointF world) const
{
    return world * m_zoom + m_panOffset;
}

QPointF CanvasWidget::snapToGrid(QPointF world) const
{
    double g = GridSize;
    return QPointF(qRound(world.x() / g) * g,
                   qRound(world.y() / g) * g);
}

// ── Modes ─────────────────────────────────────────────────────────────────────

void CanvasWidget::enterPlacementMode(const QString& componentType)
{
    m_mode      = CanvasMode::Place;
    m_placeType = componentType;
    m_wireStartPin.reset();   // always clear wire state on mode change
    m_wireWaypoints.clear();
    deselectAll();
    setCursor(Qt::CrossCursor);
    update();
}

void CanvasWidget::enterSelectionMode()
{
    m_mode = CanvasMode::Select;
    m_wireStartPin.reset();   // BUG FIX: clear dangling wire start
    m_wireWaypoints.clear();
    setCursor(Qt::ArrowCursor);
    update();
}

void CanvasWidget::enterWireMode()
{
    deselectAll();
    m_mode = CanvasMode::Wire;
    m_wireStartPin.reset();
    m_wireWaypoints.clear();
    setCursor(Qt::CrossCursor);
    update();
}

// ── Drawing ───────────────────────────────────────────────────────────────────

void CanvasWidget::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.translate(m_panOffset);
    painter.scale(m_zoom, m_zoom);

    drawGrid(painter);
    drawCanvasBounds(painter);
    drawWires(painter);
    drawJunctions(painter);
    drawComponents(painter);
    drawWireInProgress(painter);
    drawSelectionRect(painter);
    drawGhostComponent(painter);
}

void CanvasWidget::drawGrid(QPainter& p)
{
    QRectF visible = QRectF(screenToWorld({0,0}),
                            screenToWorld({(double)width(),(double)height()}));

    double step = GridSize;
    int x0 = static_cast<int>(visible.left()  / step) - 1;
    int x1 = static_cast<int>(visible.right() / step) + 1;
    int y0 = static_cast<int>(visible.top()   / step) - 1;
    int y1 = static_cast<int>(visible.bottom()/ step) + 1;

    p.setPen(QPen(QColor(210,210,210), 0.5));
    for (int ix = x0; ix <= x1; ++ix)
        p.drawLine(QPointF(ix*step, y0*step), QPointF(ix*step, y1*step));
    for (int iy = y0; iy <= y1; ++iy)
        p.drawLine(QPointF(x0*step, iy*step), QPointF(x1*step, iy*step));
}

void CanvasWidget::drawCanvasBounds(QPainter& p)
{
    QSizeF s = m_graph->canvasSize();
    QRectF r(0, 0, s.width(), s.height());
    p.save();
    p.setPen(QPen(QColor(90,90,90), 0, Qt::DashLine));
    p.setBrush(Qt::NoBrush);
    p.drawRect(r);
    p.setPen(QPen(QColor(150,0,0), 0));
    p.drawLine(QPointF(-10,0), QPointF(10,0));
    p.drawLine(QPointF(0,-10), QPointF(0,10));
    p.restore();
}

void CanvasWidget::drawComponents(QPainter& p)
{
    for (auto& comp : m_graph->components()) {
        bool sel = (comp == m_selectedComp) ||
                   (std::find(m_selectedComponents.begin(),
                              m_selectedComponents.end(), comp)
                    != m_selectedComponents.end());
        comp->draw(p, sel);
    }
}

void CanvasWidget::drawWires(QPainter& p)
{
    for (auto& wire : m_graph->wires()) {
        // Look up net value for this wire
        int netVal = -1;
        if (m_simActive) {
            auto it = m_netValues.find(wire->id());
            if (it != m_netValues.end()) netVal = it->second;
        }

        QColor col = wireColorFromNetValue(netVal, wire->selected());
        p.setPen(QPen(col, 2));
        auto& path = wire->path();
        for (size_t i = 1; i < path.size(); ++i)
            p.drawLine(path[i-1], path[i]);
    }
}

void CanvasWidget::drawJunctions(QPainter& p)
{
    for (auto& j : m_graph->junctions())
        j->draw(p);
}

void CanvasWidget::drawWireInProgress(QPainter& p)
{
    if (m_mode != CanvasMode::Wire || !m_wireStartPin) return;

    const auto path = currentWirePreviewPath(m_wireCurrentPos);
    if (path.size() < 2) return;

    p.save();
    p.setPen(QPen(Qt::blue, 2, Qt::DashLine));
    for (size_t i = 1; i < path.size(); ++i)
        p.drawLine(path[i - 1], path[i]);

    // Draw small circles on user-defined intermediate bends so the route is visible.
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(50, 120, 255, 130));
    for (const auto& wp : m_wireWaypoints)
        p.drawEllipse(wp, 3.5, 3.5);
    p.restore();
}

void CanvasWidget::drawSelectionRect(QPainter& p)
{
    if (!m_draggingSelect) return;
    QRectF r(m_selectStart, m_selectEnd);
    r = r.normalized();
    p.setPen(QPen(Qt::blue, 1, Qt::DashLine));
    p.setBrush(QColor(0,0,255,20));
    p.drawRect(r);
}

void CanvasWidget::drawGhostComponent(QPainter& p)
{
    if (m_mode != CanvasMode::Place) return;
    auto ghost = createComponent(m_placeType);
    if (!ghost) return;
    ghost->moveTo(snapToGrid(m_ghostPos));
    p.setOpacity(0.5);
    ghost->draw(p, false);
    p.setOpacity(1.0);
}

// ── Mouse events ──────────────────────────────────────────────────────────────

void CanvasWidget::mousePressEvent(QMouseEvent* e)
{
    setFocus();
    const QPointF rawWorld = screenToWorld(e->position());
    const QPointF gridWorld = snapToGrid(rawWorld);

    if (e->button() == Qt::MiddleButton || e->button() == Qt::RightButton) {
        m_panning  = true;
        m_panStart = e->position() - m_panOffset;
        return;
    }

    if (e->button() != Qt::LeftButton)
        return;

    // ── Placement mode: place ONE component then return to Select mode ────────
    if (m_mode == CanvasMode::Place) {
        auto comp = createComponent(m_placeType);
        if (comp) {
            comp->moveTo(gridWorld);
            m_cmdMgr->execute(std::make_unique<AddComponentCommand>(m_graph, comp));
            deselectAll();
            m_selectedComp = comp;
            m_selectedComponents.push_back(comp);
            emit componentSelected(comp);
        }
        m_mode = CanvasMode::Select;
        setCursor(Qt::ArrowCursor);
        update();
        return;
    }

    // ── Wire mode supports multi-bend routing.
    // First click on a pin starts the wire. Each empty click adds one 90-degree
    // bend point. Clicking a second pin completes the route.
    if (m_mode == CanvasMode::Wire) {
        auto pin = m_graph->pinAt(rawWorld);
        if (!pin) pin = m_graph->pinAt(gridWorld);

        if (!m_wireStartPin) {
            if (pin) {
                m_wireStartPin   = pin;
                m_wireWaypoints.clear();
                m_wireCurrentPos = pin->worldPos();
                update();
            }
            return;
        }

        if (pin) {
            completeWireTo(pin);
            update();
            return;
        }

        // Add another manual bend. Users can click as many bend points as they need.
        const QPointF anchor = lastWireAnchor();
        if (QLineF(anchor, gridWorld).length() > 1.0) {
            m_wireWaypoints.push_back(gridWorld);
            m_wireCurrentPos = gridWorld;
        }
        update();
        return;
    }

    // ── Selection mode ────────────────────────────────────────────────────────
    if (auto comp = componentAt(rawWorld)) {
        deselectAll();
        m_selectedComp = comp;
        m_selectedComponents.push_back(comp);
        emit componentSelected(comp);

        // The matrix keypad is directly interactive on the canvas. Clicking a
        // visual key closes that row/column contact until mouse release. Hold
        // Ctrl while dragging to move the keypad instead of pressing a key.
        if (auto* keypad = dynamic_cast<Keypad*>(comp.get())) {
            if (!(e->modifiers() & Qt::ControlModifier) && keypad->pressAtWorldPos(rawWorld)) {
                emit componentInteracted(comp);
                update();
                return;
            }
        }

        // PushButton is momentary: normal mouse press drives OUT HIGH and
        // mouse release drives it LOW. Hold Ctrl while dragging to reposition it.
        if (auto* btn = dynamic_cast<PushButton*>(comp.get())) {
            if (!(e->modifiers() & Qt::ControlModifier)) {
                btn->setPressed(true);
                emit componentInteracted(comp);
                update();
                return;
            }
        }

        m_draggingComp    = true;
        m_didDrag         = false;
        m_dragStartWorld  = gridWorld;
        m_compOriginalPos = comp->position();
        update();
        return;
    }

    if (auto wire = wireAt(rawWorld)) {
        deselectAll();
        wire->setSelected(true);
        m_selectedWire = wire;
        update();
        return;
    }

    deselectAll();
    emit componentSelected(nullptr);
    m_draggingSelect = true;
    m_selectStart    = gridWorld;
    m_selectEnd      = gridWorld;
    update();
}

void CanvasWidget::mouseDoubleClickEvent(QMouseEvent* e)
{
    if (e->button() != Qt::LeftButton) return;
    const QPointF rawWorld = screenToWorld(e->position());

    if (auto comp = componentAt(rawWorld)) {
        emit componentDoubleClicked(comp);
        return;
    }
}

void CanvasWidget::mouseMoveEvent(QMouseEvent* e)
{
    const QPointF rawWorld = screenToWorld(e->position());
    const QPointF gridWorld = snapToGrid(rawWorld);
    m_lastMouseWorld = rawWorld;
    emit mouseWorldPosChanged(gridWorld);

    if (m_panning) {
        m_panOffset = e->position() - m_panStart;
        update();
        return;
    }

    m_ghostPos = rawWorld;

    if (m_draggingComp && m_selectedComp) {
        const QPointF delta = gridWorld - m_dragStartWorld;
        if (!delta.isNull()) m_didDrag = true;
        m_selectedComp->moveTo(snapToGrid(m_compOriginalPos + delta));
        rerouteAllWires();
        update();
        return;
    }

    if (m_draggingSelect) {
        m_selectEnd = gridWorld;
        update();
        return;
    }

    if (m_mode == CanvasMode::Wire && m_wireStartPin) {
        auto pin = m_graph->pinAt(rawWorld);
        m_wireCurrentPos = pin ? pin->worldPos() : gridWorld;
        update();
    }

    // Highlight pins under cursor. Raw world coordinates are used so snapping
    // does not make the hover area jump away from the visible pin.
    for (auto& comp : m_graph->components())
        for (auto& pin : comp->pins())
            pin->setHighlighted(false);

    auto pin = m_graph->pinAt(rawWorld);
    if (pin) pin->setHighlighted(true);

    update();
}

void CanvasWidget::mouseReleaseEvent(QMouseEvent* e)
{
    const QPointF rawWorld = screenToWorld(e->position());

    if (e->button() == Qt::MiddleButton || e->button() == Qt::RightButton) {
        m_panning = false;
        return;
    }

    if (e->button() != Qt::LeftButton)
        return;

    // Complete click-drag-release wire connection if released on another pin.
    if (m_mode == CanvasMode::Wire && m_wireStartPin) {
        auto pin = m_graph->pinAt(rawWorld);
        if (pin) {
            completeWireTo(pin);
            update();
            return;
        }
    }

    if (m_selectedComp) {
        if (auto* keypad = dynamic_cast<Keypad*>(m_selectedComp.get())) {
            if (keypad->pressedKey() != "none") {
                keypad->releaseKey();
                emit componentInteracted(m_selectedComp);
                update();
            }
        }
        if (auto* btn = dynamic_cast<PushButton*>(m_selectedComp.get())) {
            if (btn->pressed()) {
                btn->setPressed(false);
                emit componentInteracted(m_selectedComp);
                update();
            }
        }
    }

    if (m_draggingComp && m_selectedComp) {
        const bool didDrag = m_didDrag;
        if (didDrag) {
            const QPointF newPos = m_selectedComp->position();
            const QPointF oldPos = m_compOriginalPos;
            if (newPos != oldPos) {
                m_cmdMgr->execute(std::make_unique<MoveComponentCommand>(
                    m_graph, m_selectedComp->id(), oldPos, newPos));
            }
        } else if (auto* sw = dynamic_cast<Switch*>(m_selectedComp.get())) {
            // A normal click toggles the persistent switch state. A real drag still moves it.
            sw->toggle();
            emit componentInteracted(m_selectedComp);
        }
        m_draggingComp = false;
        m_didDrag      = false;
        rerouteAllWires();
        update();
        return;
    }

    if (m_draggingSelect) {
        m_draggingSelect = false;
        m_selectedComponents.clear();
        QRectF selRect(m_selectStart, m_selectEnd);
        selRect = selRect.normalized();
        for (auto& comp : m_graph->components()) {
            if (selRect.intersects(comp->boundingBox()))
                m_selectedComponents.push_back(comp);
        }
        if (!m_selectedComponents.empty()) {
            m_selectedComp = m_selectedComponents.front();
            emit componentSelected(m_selectedComp);
        } else {
            m_selectedComp.reset();
            emit componentSelected(nullptr);
        }
        update();
        return;
    }
}

void CanvasWidget::wheelEvent(QWheelEvent* e)
{
    QPointF before = screenToWorld(e->position());
    double factor  = e->angleDelta().y() > 0 ? 1.15 : 1.0/1.15;
    m_zoom = std::clamp(m_zoom * factor, 0.1, 10.0);
    QPointF after = screenToWorld(e->position());
    m_panOffset += (after - before) * m_zoom;
    update();
}

void CanvasWidget::keyPressEvent(QKeyEvent* e)
{
    if (e->key() == Qt::Key_R && m_selectedComp) {
        m_selectedComp->rotate90();
        rerouteAllWires();
        update();
        return;
    }
    if (e->key() == Qt::Key_H && m_selectedComp) {
        m_selectedComp->mirrorHorizontal();
        rerouteAllWires();
        update();
        return;
    }
    if (e->key() == Qt::Key_V && m_selectedComp) {
        m_selectedComp->mirrorVertical();
        rerouteAllWires();
        update();
        return;
    }
    if (e->key() == Qt::Key_W) {
        enterWireMode();
        return;
    }
    if (e->key() == Qt::Key_J) {
        // Explicit junction only: crossing wires do NOT connect unless the user adds this dot.
        addExplicitJunctionAt(snapToGrid(m_lastMouseWorld));
        update();
        return;
    }
    if (e->key() == Qt::Key_Escape) {
        if (m_mode == CanvasMode::Wire && m_wireStartPin) {
            m_wireStartPin.reset();
            m_wireWaypoints.clear();
            update();
        } else {
            enterSelectionMode();
            deselectAll();
            emit componentSelected(nullptr);
            update();
        }
        return;
    }
    if (e->key() == Qt::Key_Delete) {
        deleteSelected();
        update();
        return;
    }
}

// ── Helpers ───────────────────────────────────────────────────────────────────

std::shared_ptr<Component> CanvasWidget::componentAt(QPointF worldPos) const
{
    // Check in reverse drawing order so top-most component is selected first.
    const auto& comps = m_graph->components();
    for (auto it = comps.rbegin(); it != comps.rend(); ++it) {
        const auto& comp = *it;
        if (comp && comp->hitTest(worldPos))
            return comp;
    }
    return nullptr;
}

std::shared_ptr<Wire> CanvasWidget::wireAt(QPointF worldPos) const
{
    const auto& wires = m_graph->wires();
    for (auto it = wires.rbegin(); it != wires.rend(); ++it) {
        const auto& wire = *it;
        if (wire && wire->hitTest(worldPos, 7.0 / std::max(0.2, m_zoom)))
            return wire;
    }
    return nullptr;
}

bool CanvasWidget::completeWireTo(const std::shared_ptr<Pin>& endPin)
{
    if (!m_wireStartPin || !endPin) return false;
    if (endPin == m_wireStartPin) {
        // Same pin: cancel only the preview, do not create a zero-length wire.
        m_wireStartPin.reset();
        m_wireWaypoints.clear();
        return false;
    }

    // Avoid duplicate wires in both directions.
    for (const auto& existing : m_graph->wires()) {
        if (!existing) continue;
        const bool sameDirection = existing->startPin() == m_wireStartPin && existing->endPin() == endPin;
        const bool reverseDirection = existing->startPin() == endPin && existing->endPin() == m_wireStartPin;
        if (sameDirection || reverseDirection) {
            m_wireStartPin.reset();
            m_wireWaypoints.clear();
            return false;
        }
    }

    auto wire = std::make_shared<Wire>(m_wireStartPin, endPin);
    const auto customPath = Wire::makeOrthogonalPath(m_wireStartPin->worldPos(),
                                                     m_wireWaypoints,
                                                     endPin->worldPos());
    if (customPath.size() < 2) {
        m_wireStartPin.reset();
        m_wireWaypoints.clear();
        return false;
    }

    wire->setPath(customPath);
    wire->setManualRoute(m_wireWaypoints.size() > 0 || customPath.size() > 3);
    m_cmdMgr->execute(std::make_unique<AddWireCommand>(m_graph, wire));

    m_wireStartPin.reset();
    m_wireWaypoints.clear();
    rerouteAllWires();
    return true;
}

std::vector<QPointF> CanvasWidget::currentWirePreviewPath(QPointF end) const
{
    if (!m_wireStartPin) return {};
    return Wire::makeOrthogonalPath(m_wireStartPin->worldPos(), m_wireWaypoints, end);
}

QPointF CanvasWidget::lastWireAnchor() const
{
    if (!m_wireWaypoints.empty()) return m_wireWaypoints.back();
    if (m_wireStartPin) return m_wireStartPin->worldPos();
    return QPointF{};
}

void CanvasWidget::addExplicitJunctionAt(QPointF worldPos)
{
    // A junction is an intentional electrical dot. Do not auto-connect all crossings.
    // Only add it when the cursor is actually on at least one existing wire segment.
    int touching = 0;
    for (const auto& w : m_graph->wires()) {
        if (w && w->hitTest(worldPos, 4.0 / std::max(0.2, m_zoom)))
            ++touching;
    }
    if (touching == 0) return;

    for (const auto& j : m_graph->junctions()) {
        if (j && QLineF(j->pos(), worldPos).length() < 3.0)
            return;
    }

    m_graph->addJunction(std::make_shared<Junction>(worldPos));
}

void CanvasWidget::rerouteAllWires()
{
    for (auto& wire : m_graph->wires()) {
        if (wire) wire->reroutePreservingWaypoints();
    }
    // Do not auto-create junctions at mere crossings; only explicit Junction dots connect nets.
    m_graph->autoDetectJunctions();
}

void CanvasWidget::selectAt(QPointF worldPos)
{
    if (auto comp = componentAt(worldPos)) {
        deselectAll();
        m_selectedComp = comp;
        m_selectedComponents.push_back(comp);
        emit componentSelected(comp);
        update();
        return;
    }
    deselectAll();
    emit componentSelected(nullptr);
    update();
}

void CanvasWidget::deselectAll()
{
    m_selectedComp.reset();
    m_selectedComponents.clear();
    m_selectedWire.reset();
    for (auto& wire : m_graph->wires()) if (wire) wire->setSelected(false);
    for (auto& j : m_graph->junctions()) if (j) j->setSelected(false);
}

void CanvasWidget::deleteSelected()
{
    if (!m_selectedComponents.empty()) {
        auto copy = m_selectedComponents;
        m_selectedComponents.clear();
        m_selectedComp.reset();
        for (auto& c : copy) {
            if (c) m_cmdMgr->execute(std::make_unique<DeleteComponentCommand>(m_graph, c));
        }
        rerouteAllWires();
        emit componentSelected(nullptr);
    } else if (m_selectedComp) {
        auto c = m_selectedComp;
        m_selectedComp.reset();
        m_cmdMgr->execute(std::make_unique<DeleteComponentCommand>(m_graph, c));
        rerouteAllWires();
        emit componentSelected(nullptr);
    }
    if (m_selectedWire) {
        auto w = m_selectedWire;
        m_selectedWire.reset();
        m_cmdMgr->execute(std::make_unique<DeleteWireCommand>(m_graph, w));
        rerouteAllWires();
    }
}

std::shared_ptr<Component> CanvasWidget::createComponent(const QString& type)
{
    if (type == "Resistor")        return std::make_shared<Resistor>();
    if (type == "Capacitor")       return std::make_shared<Capacitor>();
    if (type == "Inductor")        return std::make_shared<Inductor>();
    if (type == "Potentiometer")   return std::make_shared<Potentiometer>();
    if (type == "DCVoltageSource") return std::make_shared<DCVoltageSource>();
    if (type == "Battery")         return std::make_shared<Battery>();
    if (type == "Ground")          return std::make_shared<Ground>();
    if (type == "ClockGenerator")  return std::make_shared<ClockGenerator>();
    if (type == "Switch")          return std::make_shared<Switch>();
    if (type == "PushButton")      return std::make_shared<PushButton>();
    if (type == "LED")             return std::make_shared<LED>();
    if (type == "SevenSegment")    return std::make_shared<SevenSegment>();
    if (type == "AndGate")         return std::make_shared<AndGate>();
    if (type == "OrGate")          return std::make_shared<OrGate>();
    if (type == "NotGate")         return std::make_shared<NotGate>();
    if (type == "XorGate")         return std::make_shared<XorGate>();
    if (type == "NandGate")        return std::make_shared<NandGate>();
    if (type == "DFlipFlop")       return std::make_shared<DFlipFlop>();
    if (type == "SimpleADC")       return std::make_shared<SimpleADC>();
    if (type == "SimpleDAC")       return std::make_shared<SimpleDAC>();
    if (type == "LCD16x2")         return std::make_shared<LCD16x2>();
    if (type == "Keypad")          return std::make_shared<Keypad>();
    if (type == "VoltageProbe")    return std::make_shared<VoltageProbe>();
    if (type == "Voltmeter")       return std::make_shared<Voltmeter>();
    if (type == "Ammeter")         return std::make_shared<Ammeter>();
    if (type == "Oscilloscope")    return std::make_shared<Oscilloscope>();
    if (type == "Microcontroller") return std::make_shared<Microcontroller>();
    if (type == "ExternalMemory")  return std::make_shared<ExternalMemory>();
    return nullptr;
}

void CanvasWidget::exportImage(const QString& filePath)
{
    QPixmap pixmap(size());
    render(&pixmap);
    pixmap.save(filePath, "PNG");
}
