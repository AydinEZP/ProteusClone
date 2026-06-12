#include "CircuitGraph.h"
#include "../domain/Pin.h"
#include "../domain/components/Switch.h"
#include "../domain/components/PushButton.h"
#include "../domain/components/Potentiometer.h"
#include "../domain/components/Keypad.h"
#include <algorithm>
#include <unordered_map>
#include <functional>
#include <QLineF>
#include <cmath>

void CircuitGraph::addComponent(std::shared_ptr<Component> c) { if (c) m_components.push_back(std::move(c)); }

void CircuitGraph::removeComponent(ComponentID id) {
    std::vector<std::shared_ptr<Pin>> pinsOfRemoved;
    for (auto& c : m_components) if (c && c->id() == id) for (auto& p : c->pins()) pinsOfRemoved.push_back(p);
    m_wires.erase(std::remove_if(m_wires.begin(), m_wires.end(), [&](auto& w){
        return !w || std::find(pinsOfRemoved.begin(), pinsOfRemoved.end(), w->startPin()) != pinsOfRemoved.end() ||
               std::find(pinsOfRemoved.begin(), pinsOfRemoved.end(), w->endPin()) != pinsOfRemoved.end();
    }), m_wires.end());
    m_components.erase(std::remove_if(m_components.begin(), m_components.end(), [id](auto& c){ return !c || c->id() == id; }), m_components.end());
}

void CircuitGraph::addWire(std::shared_ptr<Wire> w) {
    if (!w || !w->startPin() || !w->endPin() || w->startPin() == w->endPin()) return;
    for (const auto& existing : m_wires) {
        if (!existing) continue;
        const bool sameDirection = existing->startPin() == w->startPin() && existing->endPin() == w->endPin();
        const bool reverseDirection = existing->startPin() == w->endPin() && existing->endPin() == w->startPin();
        if (sameDirection || reverseDirection) return;
    }
    // Do not overwrite a custom multi-bend/manual path. Only auto-route when empty.
    if (w->path().size() < 2)
        w->routeSimple();
    else
        w->reroutePreservingWaypoints();
    if (w->path().size() < 2) return;
    m_wires.push_back(std::move(w));
    autoDetectJunctions();
}
void CircuitGraph::removeWire(WireID id) { m_wires.erase(std::remove_if(m_wires.begin(), m_wires.end(), [id](auto& w){ return !w || w->id() == id; }), m_wires.end()); autoDetectJunctions(); }
void CircuitGraph::addJunction(std::shared_ptr<Junction> j) { if (j) m_junctions.push_back(std::move(j)); }
void CircuitGraph::removeJunction(JunctionID id) { m_junctions.erase(std::remove_if(m_junctions.begin(), m_junctions.end(), [id](auto& j){ return !j || j->id() == id; }), m_junctions.end()); }

void CircuitGraph::clear() { m_components.clear(); m_wires.clear(); m_junctions.clear(); m_netNodes.clear(); m_canvasSize = QSizeF(1600,1000); }

std::shared_ptr<Component> CircuitGraph::componentById(ComponentID id) const { for (auto& c : m_components) if (c && c->id() == id) return c; return nullptr; }
std::shared_ptr<Wire> CircuitGraph::wireById(WireID id) const { for (auto& w : m_wires) if (w && w->id() == id) return w; return nullptr; }
std::shared_ptr<Junction> CircuitGraph::junctionById(JunctionID id) const { for (auto& j : m_junctions) if (j && j->id() == id) return j; return nullptr; }

std::shared_ptr<Pin> CircuitGraph::pinAt(QPointF worldPoint) const {
    for (auto& comp : m_components) if (comp) for (auto& pin : comp->pins()) {
        double dx = pin->worldPos().x() - worldPoint.x(); double dy = pin->worldPos().y() - worldPoint.y();
        if (dx*dx + dy*dy <= Pin::HoverRadius * Pin::HoverRadius * 4) return pin;
    }
    return nullptr;
}

bool CircuitGraph::pointAlreadyHasJunction(QPointF p, double tolerance) const {
    for (auto& j : m_junctions) if (j && QLineF(j->pos(), p).length() <= tolerance) return true;
    return false;
}

bool CircuitGraph::axisAlignedIntersection(QPointF a1, QPointF a2, QPointF b1, QPointF b2, QPointF* out) {
    bool aVert = std::abs(a1.x()-a2.x()) < 1e-6;
    bool bVert = std::abs(b1.x()-b2.x()) < 1e-6;
    if (aVert == bVert) return false;
    QPointF v1 = aVert ? a1 : b1; QPointF v2 = aVert ? a2 : b2;
    QPointF h1 = aVert ? b1 : a1; QPointF h2 = aVert ? b2 : a2;
    double x = v1.x(); double y = h1.y();
    double vyMin=std::min(v1.y(),v2.y()), vyMax=std::max(v1.y(),v2.y());
    double hxMin=std::min(h1.x(),h2.x()), hxMax=std::max(h1.x(),h2.x());
    if (x>=hxMin && x<=hxMax && y>=vyMin && y<=vyMax) { if(out)*out=QPointF(x,y); return true; }
    return false;
}

void CircuitGraph::autoDetectJunctions() {
    // Important Proteus-like rule:
    // Crossing wires are NOT electrically connected unless the user explicitly
    // creates a junction dot. Therefore this function intentionally does not
    // create junctions from crossings.
    //
    // It only removes invalid/duplicate explicit junctions and preserves the
    // user's manually placed dots.
    std::vector<std::shared_ptr<Junction>> cleaned;
    for (const auto& j : m_junctions) {
        if (!j) continue;
        bool duplicate = false;
        for (const auto& kept : cleaned) {
            if (kept && QLineF(kept->pos(), j->pos()).length() <= 1.0) {
                duplicate = true;
                break;
            }
        }
        if (!duplicate) cleaned.push_back(j);
    }
    m_junctions.swap(cleaned);
}

void CircuitGraph::buildNetlist() {
    autoDetectJunctions();
    m_netNodes.clear();
    std::unordered_map<Pin*, Pin*> parent;
    std::function<Pin*(Pin*)> find = [&](Pin* p)->Pin*{ if(parent.find(p)==parent.end()) parent[p]=p; if(parent[p]!=p) parent[p]=find(parent[p]); return parent[p]; };
    auto unite=[&](Pin* a, Pin* b){ a=find(a); b=find(b); if(a!=b) parent[a]=b; };
    for(auto& comp:m_components) if (comp) for(auto& pin:comp->pins()) if(pin) find(pin.get());

    // Proteus-like GND semantics: every Ground symbol is the same global 0V net,
    // even if the user did not draw a visible wire between two GND components.
    Pin* firstGround = nullptr;
    for(auto& comp:m_components) if (comp) {
        for(auto& pin:comp->pins()) {
            if(pin && pin->type() == PinType::Ground) {
                if(!firstGround) firstGround = pin.get();
                else unite(firstGround, pin.get());
            }
        }
    }

    for(auto& wire:m_wires){ if(wire && wire->startPin() && wire->endPin()) unite(wire->startPin().get(), wire->endPin().get()); }

    // ── Ideal topology-changing components ─────────────────────────────────
    // Switch: when closed A and B are literally the same electrical net. When
    // open, there is no conductance and no implicit pull-down: the isolated side
    // is allowed to remain FLOATING.
    // Potentiometer: the exact end positions are valid. At wiper==0 W is shorted
    // to A; at wiper==1 W is shorted to B. This avoids fake epsilon resistors and
    // keeps the MNA matrix well-defined at both end stops.
    constexpr double endpointEps = 1e-12;
    for (auto& comp : m_components) {
        if (!comp) continue;
        if (auto* sw = dynamic_cast<Switch*>(comp.get())) {
            if (sw->closed()) {
                auto pA = comp->pinByName("A");
                auto pB = comp->pinByName("B");
                if (pA && pB) unite(pA.get(), pB.get());
            }
        } else if (auto* pot = dynamic_cast<Potentiometer*>(comp.get())) {
            auto pA = comp->pinByName("A");
            auto pB = comp->pinByName("B");
            auto pW = comp->pinByName("W");
            if (!pA || !pB || !pW) continue;
            const double w = std::clamp(pot->wiper(), 0.0, 1.0);
            if (w <= endpointEps) unite(pA.get(), pW.get());
            if (w >= 1.0 - endpointEps) unite(pW.get(), pB.get());
        } else if (auto* keypad = dynamic_cast<Keypad*>(comp.get())) {
            // A pressed matrix key is a real temporary contact between exactly
            // one row and one column. Rebuilding the netlist each simulation
            // tick makes press/release electrically immediate and symmetric.
            if (keypad->pressedRow() >= 0 && keypad->pressedCol() >= 0) {
                auto row = comp->pinByName(QString("R%1").arg(keypad->pressedRow() + 1));
                auto col = comp->pinByName(QString("C%1").arg(keypad->pressedCol() + 1));
                if (row && col) unite(row.get(), col.get());
            }
        }
    }

    // Join wires that have an explicit junction point on their path.
    for(auto& j:m_junctions){
        if (!j) continue;
        std::vector<std::shared_ptr<Wire>> touching;
        for(auto& w:m_wires) if(w && w->hitTest(j->pos(), 1.5)) touching.push_back(w);
        if(touching.size() >= 2){
            std::shared_ptr<Pin> anchor = touching.front()->startPin() ? touching.front()->startPin() : touching.front()->endPin();
            if(anchor){ for(auto& w:touching){ if(w->startPin()) unite(anchor.get(), w->startPin().get()); if(w->endPin()) unite(anchor.get(), w->endPin().get()); } }
        }
    }

    std::unordered_map<Pin*, int> rootToNode;
    for(auto& kv:parent){ Pin* root=find(kv.first); if(!rootToNode.count(root)){ int idx=(int)m_netNodes.size(); rootToNode[root]=idx; NetNode node; node.id=idx; m_netNodes.push_back(node); } }
    for(auto& comp:m_components) if (comp) for(auto& pin:comp->pins()){ if(!pin) continue; Pin* root=find(pin.get()); int nid=rootToNode[root]; m_netNodes[nid].pins.push_back(pin); }
    for(auto& wire:m_wires) if(wire && wire->startPin()){ Pin* root=find(wire->startPin().get()); if(rootToNode.count(root)) m_netNodes[rootToNode[root]].wires.push_back(wire); }
}

QStringList CircuitGraph::runDRC() const {
    QStringList warnings;
    for(auto& comp:m_components) if (comp) for(auto& pin:comp->pins()) {
        if(pin->type()==PinType::Input) {
            bool connected=false; for(auto& wire:m_wires) if(wire && (wire->startPin()==pin || wire->endPin()==pin)){ connected=true; break; }
            if(!connected) warnings << QString("Floating input detected: %1 pin '%2' on '%3'").arg(comp->type()).arg(pin->name()).arg(comp->label());
        }
    }
    for(auto& node:m_netNodes){
        int grounds=0, powers=0;
        for(auto& pin:node.pins){
            if (!pin) continue;
            if(pin->type()==PinType::Ground) grounds++;
            if(pin->type()==PinType::Power) powers++;
        }
        // Structural DRC can prove a power-to-ground collapse. Multiple output
        // pins alone are not automatically a short: equal-valued ideal drivers
        // are allowed, while SimulationEngine checks actual driver voltages and
        // blocks only contradictory values.
        if(grounds>0 && powers>0)
            warnings << QString("Short circuit: net node %1 connects power to ground").arg(node.id);
    }
    if(m_components.empty()) warnings << "Empty project: no components placed.";
    return warnings;
}
