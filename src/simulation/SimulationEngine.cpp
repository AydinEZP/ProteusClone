#include "SimulationEngine.h"

#include "../domain/components/ClockGenerator.h"
#include "../domain/components/LogicGate.h"
#include "../domain/components/DFlipFlop.h"
#include "../domain/components/LED.h"
#include "../domain/components/Switch.h"
#include "../domain/components/PushButton.h"
#include "../domain/components/VoltageProbe.h"
#include "../domain/components/Voltmeter.h"
#include "../domain/components/Ammeter.h"
#include "../domain/components/Oscilloscope.h"
#include "../domain/components/SimpleADC.h"
#include "../domain/components/SimpleDAC.h"
#include "../domain/components/DCVoltageSource.h"
#include "../domain/components/Battery.h"
#include "../domain/components/Resistor.h"
#include "../domain/components/Capacitor.h"
#include "../domain/components/Inductor.h"
#include "../domain/components/Potentiometer.h"
#include "../domain/components/Microcontroller.h"
#include "../domain/components/LCD16x2.h"
#include "../domain/components/Keypad.h"
#include "../domain/components/ExternalMemory.h"
#include "../domain/components/SevenSegment.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <unordered_set>
#include <vector>
#include <QVector>
#include <QStringList>

namespace {
constexpr double kLogicLowMax  = 0.8;
constexpr double kLogicHighMin = 2.0;
constexpr double kLogicHighVoltage = 5.0;
constexpr double kMinConductance = 1e-12;
constexpr double kMaxConductance = 1e9;
constexpr double kAmmeterResistance = 1e-3;   // ideal-ish current meter
constexpr double kLedOnResistance   = 220.0;  // simple conducting LED model
constexpr double kLedLeakResistance = 1e9;
constexpr double kLedThreshold      = 1.8;

bool nearlyEqual(double a, double b, double eps = 1e-9)
{
    return std::abs(a - b) <= eps;
}

std::shared_ptr<Pin> pinNamed(const std::shared_ptr<Component>& comp, const QString& name)
{
    if (!comp) return nullptr;
    for (auto& p : comp->pins())
        if (p && p->name() == name) return p;
    return nullptr;
}

std::shared_ptr<Pin> firstPinByType(const std::shared_ptr<Component>& comp, PinType type)
{
    if (!comp) return nullptr;
    for (auto& p : comp->pins())
        if (p && p->type() == type) return p;
    return nullptr;
}

bool solveLinearSystem(std::vector<std::vector<double>>& A, std::vector<double>& b, std::vector<double>& x)
{
    const int n = static_cast<int>(b.size());
    x.assign(n, 0.0);
    if (n == 0) return true;

    for (int col = 0; col < n; ++col) {
        int pivot = col;
        double best = std::abs(A[col][col]);
        for (int row = col + 1; row < n; ++row) {
            double v = std::abs(A[row][col]);
            if (v > best) { best = v; pivot = row; }
        }
        if (best < 1e-12) return false;
        if (pivot != col) {
            std::swap(A[pivot], A[col]);
            std::swap(b[pivot], b[col]);
        }

        const double div = A[col][col];
        for (int j = col; j < n; ++j) A[col][j] /= div;
        b[col] /= div;

        for (int row = 0; row < n; ++row) {
            if (row == col) continue;
            const double factor = A[row][col];
            if (std::abs(factor) < 1e-18) continue;
            for (int j = col; j < n; ++j) A[row][j] -= factor * A[col][j];
            b[row] -= factor * b[col];
        }
    }

    x = b;
    return true;
}
}

SimulationEngine::SimulationEngine(CircuitGraph* graph, QObject* parent)
    : QObject(parent), m_graph(graph)
{
    connect(&m_timer, &QTimer::timeout, this, &SimulationEngine::tick);
}

void SimulationEngine::start()
{
    if (!m_graph) return;

    m_graph->buildNetlist();
    rebuildReferencedNodes();

    const QStringList faults = detectBlockingFaults();
    if (!faults.isEmpty()) {
        m_timer.stop();
        m_state = SimState::Stopped;
        QStringList log = faults;
        log.prepend("Simulation aborted by DRC: blocking electrical fault detected.");
        emit tickDone(log, computeWireNetValues());
        emit stateChanged(m_state);
        return;
    }

    m_state = SimState::Running;
    m_timer.start(static_cast<int>(m_dt * 1000));
    emit tickDone({"DRC passed: no blocking short circuit or floating input detected. Simulation started."},
                  computeWireNetValues());
    emit stateChanged(m_state);
}

void SimulationEngine::pause()
{
    m_state = SimState::Paused;
    m_timer.stop();
    emit stateChanged(m_state);
}

void SimulationEngine::stop()
{
    m_state = SimState::Stopped;
    m_timer.stop();
    m_time = 0.0;
    m_pinVoltages.clear();
    m_referencedNodes.clear();
    m_keypadEchoLastKey.clear();
    m_keypadEchoText.clear();
    for (auto& comp : m_graph->components()) {
        if (auto* osc = dynamic_cast<Oscilloscope*>(comp.get()))
            osc->clearSamples();
    }
    emit stateChanged(m_state);
}

void SimulationEngine::step()
{
    if (m_state == SimState::Running) return;
    m_state = SimState::Paused;
    emit stateChanged(m_state);
    tick();
}


// ── Electrical reference / blocking DRC helpers ──────────────────────────────

void SimulationEngine::rebuildReferencedNodes()
{
    m_referencedNodes.clear();
    if (!m_graph) return;

    const auto& nodes = m_graph->netNodes();
    if (nodes.empty()) return;

    std::unordered_map<const Pin*, int> pinToNode;
    std::vector<std::vector<int>> adjacency(nodes.size());

    for (const auto& node : nodes) {
        if (node.id < 0 || node.id >= static_cast<int>(nodes.size())) continue;
        for (const auto& pin : node.pins) {
            if (!pin) continue;
            pinToNode[pin.get()] = node.id;
            if (pin->type() == PinType::Ground)
                m_referencedNodes.insert(node.id);
        }
    }

    auto nodeOf = [&](const std::shared_ptr<Pin>& pin) -> int {
        if (!pin) return -1;
        const auto it = pinToNode.find(pin.get());
        return it == pinToNode.end() ? -1 : it->second;
    };

    auto link = [&](const std::shared_ptr<Pin>& a, const std::shared_ptr<Pin>& b) {
        const int na = nodeOf(a);
        const int nb = nodeOf(b);
        if (na < 0 || nb < 0 || na == nb) return;
        adjacency[na].push_back(nb);
        adjacency[nb].push_back(na);
    };

    auto seed = [&](const std::shared_ptr<Pin>& pin) {
        const int n = nodeOf(pin);
        if (n >= 0) m_referencedNodes.insert(n);
    };

    for (const auto& comp : m_graph->components()) {
        if (!comp) continue;

        if (dynamic_cast<Resistor*>(comp.get()) ||
            dynamic_cast<Capacitor*>(comp.get()) ||
            dynamic_cast<Inductor*>(comp.get())) {
            link(pinNamed(comp, "A"), pinNamed(comp, "B"));
        } else if (auto* pot = dynamic_cast<Potentiometer*>(comp.get())) {
            const double w = std::clamp(pot->wiper(), 0.0, 1.0);
            constexpr double eps = 1e-12;
            if (w > eps) link(pinNamed(comp, "A"), pinNamed(comp, "W"));
            if ((1.0 - w) > eps) link(pinNamed(comp, "W"), pinNamed(comp, "B"));
        } else if (dynamic_cast<Ammeter*>(comp.get())) {
            link(pinNamed(comp, "IN"), pinNamed(comp, "OUT"));
        } else if (dynamic_cast<LED*>(comp.get())) {
            link(pinNamed(comp, "A"), pinNamed(comp, "K"));
        } else if (dynamic_cast<DCVoltageSource*>(comp.get()) ||
                   dynamic_cast<Battery*>(comp.get())) {
            link(pinNamed(comp, "POS"), pinNamed(comp, "NEG"));
        } else if (auto* sw = dynamic_cast<Switch*>(comp.get())) {
            if (sw->closed()) link(pinNamed(comp, "A"), pinNamed(comp, "B"));
        }

        // One-terminal ideal outputs are referenced to the global simulator
        // reference and therefore anchor their node even when no passive path
        // reaches a GND symbol.
        if (auto* clk = dynamic_cast<ClockGenerator*>(comp.get())) {
            Q_UNUSED(clk);
            seed(firstPinByType(comp, PinType::Output));
        } else if (auto* btn = dynamic_cast<PushButton*>(comp.get())) {
            Q_UNUSED(btn);
            seed(pinNamed(comp, "OUT"));
        } else if (auto* gate = dynamic_cast<LogicGate*>(comp.get())) {
            if (gate->outputValid()) seed(pinNamed(comp, "OUT"));
        } else if (auto* dff = dynamic_cast<DFlipFlop*>(comp.get())) {
            if (dff->outputValid()) {
                seed(pinNamed(comp, "Q"));
                seed(pinNamed(comp, "QB"));
            }
        } else if (dynamic_cast<SimpleADC*>(comp.get())) {
            for (const auto& pin : comp->pins())
                if (pin && pin->name().startsWith("D")) seed(pin);
        } else if (dynamic_cast<SimpleDAC*>(comp.get())) {
            seed(pinNamed(comp, "VOUT"));
        } else if (auto* mcu = dynamic_cast<Microcontroller*>(comp.get())) {
            for (int port = 0; port < Microcontroller::portCount(); ++port) {
                for (int bit = 0; bit < 8; ++bit) {
                    if (mcu->portBitIsOutput(port, bit))
                        seed(pinNamed(comp, QString("P%1.%2").arg(port).arg(bit)));
                }
            }
        } else if (auto* mem = dynamic_cast<ExternalMemory*>(comp.get())) {
            Q_UNUSED(mem);
            // During an active read cycle the memory is a real digital driver
            // on D0..D7. Mark those nets referenced so wire coloring and input
            // sampling do not mistake them for floating nodes.
            if (getNetValue(pinNamed(comp, "RD")) == 0 &&
                getNetValue(pinNamed(comp, "WR")) != 0) {
                for (int bit = 0; bit < 8; ++bit)
                    seed(pinNamed(comp, QString("D%1").arg(bit)));
            }
        }
    }

    std::vector<int> stack;
    stack.reserve(m_referencedNodes.size());
    for (const int n : m_referencedNodes) stack.push_back(n);

    while (!stack.empty()) {
        const int n = stack.back();
        stack.pop_back();
        if (n < 0 || n >= static_cast<int>(adjacency.size())) continue;
        for (const int next : adjacency[n]) {
            if (m_referencedNodes.insert(next).second)
                stack.push_back(next);
        }
    }
}

QStringList SimulationEngine::detectBlockingFaults() const
{
    QStringList errors;
    if (!m_graph) return errors;

    const auto& nodes = m_graph->netNodes();
    if (nodes.empty()) return errors;

    std::unordered_map<const Pin*, int> pinToNode;
    for (const auto& node : nodes)
        for (const auto& pin : node.pins)
            if (pin) pinToNode[pin.get()] = node.id;

    struct DriverInfo {
        double value;
        QString label;
    };
    std::vector<std::vector<DriverInfo>> drivers(nodes.size());

    auto addDriver = [&](const std::shared_ptr<Pin>& pin, double value, const QString& label) {
        if (!pin || !std::isfinite(value)) return;
        const auto it = pinToNode.find(pin.get());
        if (it == pinToNode.end()) return;
        const int n = it->second;
        if (n >= 0 && n < static_cast<int>(drivers.size()))
            drivers[n].push_back({value, label});
    };

    // Every explicit ground terminal is a 0V ideal driver/reference.
    for (const auto& node : nodes)
        for (const auto& pin : node.pins)
            if (pin && pin->type() == PinType::Ground)
                addDriver(pin, 0.0, "GND=0V");

    for (const auto& comp : m_graph->components()) {
        if (!comp) continue;

        if (auto* src = dynamic_cast<DCVoltageSource*>(comp.get())) {
            addDriver(pinNamed(comp, "POS"), src->voltage(), comp->label() + "=" + QString::number(src->voltage()) + "V");
        } else if (auto* bat = dynamic_cast<Battery*>(comp.get())) {
            addDriver(pinNamed(comp, "POS"), bat->voltage(), comp->label() + "=" + QString::number(bat->voltage()) + "V");
        } else if (auto* clk = dynamic_cast<ClockGenerator*>(comp.get())) {
            addDriver(firstPinByType(comp, PinType::Output), clk->currentOutput() ? 5.0 : 0.0,
                      comp->label() + ":OUT");
        } else if (auto* btn = dynamic_cast<PushButton*>(comp.get())) {
            addDriver(pinNamed(comp, "OUT"), btn->pressed() ? 5.0 : 0.0,
                      comp->label() + ":OUT");
        } else if (auto* gate = dynamic_cast<LogicGate*>(comp.get())) {
            if (gate->outputValid())
                addDriver(pinNamed(comp, "OUT"), gate->boolOutput() ? 5.0 : 0.0,
                          comp->label() + ":OUT");
        } else if (auto* dff = dynamic_cast<DFlipFlop*>(comp.get())) {
            if (dff->outputValid()) {
                addDriver(pinNamed(comp, "Q"), dff->q() ? 5.0 : 0.0, comp->label() + ":Q");
                addDriver(pinNamed(comp, "QB"), dff->qBar() ? 5.0 : 0.0, comp->label() + ":QB");
            }
        } else if (auto* adc = dynamic_cast<SimpleADC*>(comp.get())) {
            const uint32_t value = adc->digitalOutput();
            for (const auto& pin : comp->pins()) {
                if (!pin || !pin->name().startsWith("D")) continue;
                bool ok = false;
                const int bit = pin->name().mid(1).toInt(&ok);
                if (ok && bit >= 0 && bit < 32)
                    addDriver(pin, ((value >> bit) & 1u) ? 5.0 : 0.0,
                              comp->label() + ":" + pin->name());
            }
        } else if (auto* dac = dynamic_cast<SimpleDAC*>(comp.get())) {
            addDriver(pinNamed(comp, "VOUT"), dac->analogOutput(), comp->label() + ":VOUT");
        } else if (auto* mcu = dynamic_cast<Microcontroller*>(comp.get())) {
            for (int port = 0; port < Microcontroller::portCount(); ++port) {
                for (int bit = 0; bit < 8; ++bit) {
                    if (!mcu->portBitIsOutput(port, bit)) continue;
                    const QString pinName = QString("P%1.%2").arg(port).arg(bit);
                    addDriver(pinNamed(comp, pinName), mcu->outputBit(port, bit) ? 5.0 : 0.0,
                              comp->label() + ":" + pinName);
                }
            }
        } else if (auto* mem = dynamic_cast<ExternalMemory*>(comp.get())) {
            const int rd = getNetValue(pinNamed(comp, "RD"));
            const int wr = getNetValue(pinNamed(comp, "WR"));
            if (rd == 0 && wr == 0) {
                errors << QString("External memory '%1': RD and WR are active at the same time.")
                              .arg(comp->label());
            } else if (rd == 0 && wr != 0) {
                quint16 addr = 0;
                for (int i = 0; i < 8; ++i)
                    if (getNetValue(pinNamed(comp, QString("A%1").arg(i))) > 0)
                        addr |= quint16(1u << i);
                const quint8 data = mem->read(addr);
                for (int i = 0; i < 8; ++i)
                    addDriver(pinNamed(comp, QString("D%1").arg(i)),
                              ((data >> i) & 1u) ? 5.0 : 0.0,
                              comp->label() + QString(":D%1").arg(i));
            }
        }
    }

    for (int nodeId = 0; nodeId < static_cast<int>(drivers.size()); ++nodeId) {
        const auto& ds = drivers[nodeId];
        if (ds.size() < 2) continue;

        double minV = ds.front().value;
        double maxV = ds.front().value;
        for (const auto& d : ds) {
            minV = std::min(minV, d.value);
            maxV = std::max(maxV, d.value);
        }
        if (std::abs(maxV - minV) <= 1e-6) continue;

        QStringList details;
        for (const auto& d : ds)
            details << QString("%1 (%2 V)").arg(d.label).arg(d.value, 0, 'g', 8);
        errors << QString("Short circuit: net node %1 has conflicting ideal drivers: %2")
                      .arg(nodeId)
                      .arg(details.join(", "));
    }

    // Page-24 Floating rule: an input that has no electrical path to a real
    // source/reference must block Run. Merely wiring two inputs together does
    // not make either input defined.
    for (const auto& comp : m_graph->components()) {
        if (!comp) continue;
        for (const auto& pin : comp->pins()) {
            if (!pin) continue;
            if (pin->type() != PinType::Input)
                continue;

            const auto it = pinToNode.find(pin.get());
            const int nodeId = (it == pinToNode.end()) ? -1 : it->second;
            if (nodeId < 0 || m_referencedNodes.find(nodeId) == m_referencedNodes.end()) {
                errors << QString("Floating input detected: %1 pin '%2' on '%3'. Run is blocked.")
                              .arg(comp->type()).arg(pin->name()).arg(comp->label());
            }
        }
    }

    return errors;
}

// ── Net value helpers ────────────────────────────────────────────────────────

int SimulationEngine::getNetValue(const std::shared_ptr<Pin>& queryPin) const
{
    if (!queryPin) return -1;

    for (auto& node : m_graph->netNodes()) {
        bool inNode = false;
        for (auto& p : node.pins) {
            if (p == queryPin) { inNode = true; break; }
        }
        if (!inNode) continue;

        bool sawGround = false;
        bool sawDriver = false;
        int driverValue = -1;
        bool conflict = false;

        auto registerDriver = [&](int value) {
            if (!sawDriver) { sawDriver = true; driverValue = value; }
            else if (driverValue != value) conflict = true;
        };

        for (auto& p : node.pins) {
            if (!p) continue;
            if (p->type() == PinType::Ground) sawGround = true;

            for (auto& comp : m_graph->components()) {
                if (!comp) continue;
                bool owns = false;
                for (auto& cp : comp->pins()) if (cp == p) { owns = true; break; }
                if (!owns) continue;

                if (auto* src = dynamic_cast<DCVoltageSource*>(comp.get())) {
                    if (p->name() == "POS") registerDriver(src->voltage() >= kLogicHighMin ? 1 : 0);
                } else if (auto* bat = dynamic_cast<Battery*>(comp.get())) {
                    if (p->name() == "POS") registerDriver(bat->voltage() >= kLogicHighMin ? 1 : 0);
                } else if (auto* clk = dynamic_cast<ClockGenerator*>(comp.get())) {
                    if (p->type() == PinType::Output) registerDriver(clk->currentOutput() ? 1 : 0);
                } else if (auto* btn = dynamic_cast<PushButton*>(comp.get())) {
                    if (p->name() == "OUT") registerDriver(btn->pressed() ? 1 : 0);
                } else if (auto* gate = dynamic_cast<LogicGate*>(comp.get())) {
                    if (p->name() == "OUT" && gate->outputValid()) registerDriver(gate->boolOutput() ? 1 : 0);
                } else if (auto* dff = dynamic_cast<DFlipFlop*>(comp.get())) {
                    if (dff->outputValid()) {
                        if (p->name() == "Q")  registerDriver(dff->q() ? 1 : 0);
                        if (p->name() == "QB") registerDriver(dff->qBar() ? 1 : 0);
                    }
                } else if (auto* adc = dynamic_cast<SimpleADC*>(comp.get())) {
                    if (p->name().startsWith("D")) {
                        bool ok = false;
                        int bit = p->name().mid(1).toInt(&ok);
                        if (ok) registerDriver((adc->digitalOutput() >> bit) & 1u ? 1 : 0);
                    }
                } else if (auto* mcu = dynamic_cast<Microcontroller*>(comp.get())) {
                    if (p->name().startsWith("P")) {
                        const QStringList parts = p->name().mid(1).split('.');
                        if (parts.size() == 2) {
                            const int port = parts[0].toInt();
                            const int bit = parts[1].toInt();
                            if (mcu->portBitIsOutput(port, bit))
                                registerDriver(mcu->outputBit(port, bit) ? 1 : 0);
                        }
                    }
                } else if (auto* mem = dynamic_cast<ExternalMemory*>(comp.get())) {
                    if (p->name().startsWith("D") && getNetValue(pinNamed(comp, "RD")) == 0 &&
                        getNetValue(pinNamed(comp, "WR")) != 0) {
                        bool ok = false;
                        const int bit = p->name().mid(1).toInt(&ok);
                        if (ok && bit >= 0 && bit < 8) {
                            quint16 addr = 0;
                            for (int i = 0; i < 8; ++i)
                                if (getNetValue(pinNamed(comp, QString("A%1").arg(i))) > 0)
                                    addr |= quint16(1u << i);
                            registerDriver(((mem->read(addr) >> bit) & 1u) ? 1 : 0);
                        }
                    }
                }
            }
        }

        if (conflict) return -1;
        if (sawDriver) return driverValue;
        if (sawGround) return 0;

        // A numerical 0V produced only by matrix regularization is NOT a real LOW.
        // In particular, the disconnected side of an open ideal switch must stay FLOATING.
        if (m_referencedNodes.find(node.id) == m_referencedNodes.end())
            return -1;

        // If no explicit digital driver exists, threshold a genuinely referenced analog voltage.
        for (auto& p : node.pins) {
            auto it = m_pinVoltages.find(p.get());
            if (it == m_pinVoltages.end()) continue;
            const double v = it->second;
            if (v >= kLogicHighMin) return 1;
            if (v <= kLogicLowMax)  return 0;
            return -1; // undefined logic region
        }
        return -1;
    }
    return -1;
}

double SimulationEngine::getNetVoltage(const std::shared_ptr<Pin>& queryPin) const
{
    if (!queryPin) return 0.0;

    for (auto& node : m_graph->netNodes()) {
        bool inNode = false;
        for (auto& p : node.pins) {
            if (p == queryPin) { inNode = true; break; }
        }
        if (!inNode) continue;

        // Prefer the solved analog voltage for any pin on the same node.
        for (auto& p : node.pins) {
            auto it = m_pinVoltages.find(p.get());
            if (it != m_pinVoltages.end()) return it->second;
        }

        // Fallback before the first analog solve.
        bool sawGround = false;
        for (auto& p : node.pins) {
            if (!p) continue;
            if (p->type() == PinType::Ground) sawGround = true;
            for (auto& comp : m_graph->components()) {
                if (!comp) continue;
                bool owns = false;
                for (auto& cp : comp->pins()) if (cp == p) { owns = true; break; }
                if (!owns) continue;
                if (auto* src = dynamic_cast<DCVoltageSource*>(comp.get())) if (p->name() == "POS") return src->voltage();
                if (auto* bat = dynamic_cast<Battery*>(comp.get())) if (p->name() == "POS") return bat->voltage();
                if (auto* clk = dynamic_cast<ClockGenerator*>(comp.get())) if (p->type() == PinType::Output) return clk->currentOutput() ? 5.0 : 0.0;
                if (auto* btn = dynamic_cast<PushButton*>(comp.get())) if (p->name() == "OUT") return btn->pressed() ? 5.0 : 0.0;
                if (auto* gate = dynamic_cast<LogicGate*>(comp.get())) if (p->name() == "OUT" && gate->outputValid()) return gate->boolOutput() ? 5.0 : 0.0;
                if (auto* dff = dynamic_cast<DFlipFlop*>(comp.get())) {
                    if (dff->outputValid()) {
                        if (p->name() == "Q") return dff->q() ? 5.0 : 0.0;
                        if (p->name() == "QB") return dff->qBar() ? 5.0 : 0.0;
                    }
                }
            }
        }
        return sawGround ? 0.0 : 0.0;
    }
    return 0.0;
}

std::unordered_map<WireID,int> SimulationEngine::computeWireNetValues() const
{
    std::unordered_map<WireID,int> result;
    for (auto& wire : m_graph->wires()) {
        int val = -1;
        if (wire && wire->startPin()) val = getNetValue(wire->startPin());
        if (wire && val < 0 && wire->endPin()) val = getNetValue(wire->endPin());
        result[wire ? wire->id() : 0] = val;
    }
    return result;
}

// ── Topological sort for combinational logic evaluation ──────────────────────

std::vector<Component*> SimulationEngine::topoSortGates() const
{
    std::vector<LogicGate*> allGates;
    for (auto& comp : m_graph->components())
        if (auto* g = dynamic_cast<LogicGate*>(comp.get()))
            allGates.push_back(g);

    if (allGates.empty()) return {};

    std::unordered_map<LogicGate*, std::vector<LogicGate*>> deps;
    for (auto* gate : allGates) deps[gate] = {};

    for (auto& comp : m_graph->components()) {
        auto* srcGate = dynamic_cast<LogicGate*>(comp.get());
        if (!srcGate) continue;

        std::shared_ptr<Pin> outPin;
        for (auto& pin : comp->pins())
            if (pin->name() == "OUT") { outPin = pin; break; }
        if (!outPin) continue;

        for (auto& node : m_graph->netNodes()) {
            bool hasOut = false;
            for (auto& p : node.pins) if (p == outPin) { hasOut = true; break; }
            if (!hasOut) continue;

            for (auto& p : node.pins) {
                if (p->type() != PinType::Input) continue;
                for (auto& comp2 : m_graph->components()) {
                    auto* dstGate = dynamic_cast<LogicGate*>(comp2.get());
                    if (!dstGate || dstGate == srcGate) continue;
                    for (auto& cp : comp2->pins())
                        if (cp == p) deps[dstGate].push_back(srcGate);
                }
            }
            break;
        }
    }

    std::unordered_map<LogicGate*, int> inDegree;
    for (auto* g : allGates) inDegree[g] = 0;
    for (auto& [gate, upstream] : deps)
        for (auto* u : upstream)
            inDegree[gate]++;

    std::vector<LogicGate*> queue, resultGates;
    for (auto* g : allGates)
        if (inDegree[g] == 0) queue.push_back(g);

    while (!queue.empty()) {
        auto* curr = queue.back(); queue.pop_back();
        resultGates.push_back(curr);
        for (auto& [gate, upstream] : deps) {
            for (auto* u : upstream) {
                if (u == curr) {
                    inDegree[gate]--;
                    if (inDegree[gate] == 0) queue.push_back(gate);
                }
            }
        }
    }

    if (resultGates.size() < allGates.size()) {
        std::vector<Component*> fallback;
        for (auto& comp : m_graph->components())
            if (dynamic_cast<LogicGate*>(comp.get()))
                fallback.push_back(comp.get());
        return fallback;
    }

    std::vector<Component*> result;
    for (auto* g : resultGates)
        for (auto& comp : m_graph->components())
            if (comp.get() == g) { result.push_back(comp.get()); break; }
    return result;
}

// ── Digital phase ────────────────────────────────────────────────────────────

void SimulationEngine::sampleMicrocontrollerInputs()
{
    if (!m_graph) return;

    for (const auto& comp : m_graph->components()) {
        auto* mcu = dynamic_cast<Microcontroller*>(comp.get());
        if (!mcu) continue;

        for (int port = 0; port < Microcontroller::portCount(); ++port) {
            for (int bit = 0; bit < 8; ++bit) {
                if (mcu->portBitIsOutput(port, bit)) continue;
                const auto pin = pinNamed(comp, QString("P%1.%2").arg(port).arg(bit));
                const int value = getNetValue(pin);
                // Undefined/floating inputs are sampled LOW by the tiny CPU core;
                // the simulator still reports floating-input DRC separately.
                mcu->setInputPortBit(port, bit, value > 0);
            }
        }
    }
}

void SimulationEngine::simulateDigital(QStringList& warnings)
{
    for (auto& comp : m_graph->components()) {
        if (auto* clk = dynamic_cast<ClockGenerator*>(comp.get()))
            clk->tick(m_dt);
    }

    // Sample GPIO inputs before executing the next instruction. This makes
    // keypad columns, memory read data, and other external digital signals
    // visible to MOV A,Pn in the same simulation cycle.
    sampleMicrocontrollerInputs();
    for (auto& comp : m_graph->components()) {
        if (auto* mcu = dynamic_cast<Microcontroller*>(comp.get()))
            mcu->tickCpu();
    }

    // DAC digital input side: read D0..Dn pins from current digital/analog values.
    for (auto& comp : m_graph->components()) {
        if (auto* dac = dynamic_cast<SimpleDAC*>(comp.get())) {
            uint32_t code = 0;
            for (auto& pin : comp->pins()) {
                if (!pin || !pin->name().startsWith("D")) continue;
                bool ok = false;
                int bit = pin->name().mid(1).toInt(&ok);
                if (!ok || bit < 0 || bit >= 31) continue;
                if (getNetValue(pin) > 0) code |= (1u << bit);
            }
            dac->setInput(code);
            for (auto& pin : comp->pins()) {
                if (pin->name() == "VREF+") dac->setVRef(0.0, getNetVoltage(pin));
                if (pin->name() == "VREF-") dac->setVRef(getNetVoltage(pin), getNetVoltage(pinNamed(comp,"VREF+")));
            }
            dac->tickConversion(m_dt);
        }
    }

    auto sortedComps = topoSortGates();
    for (auto* compPtr : sortedComps) {
        auto* gate = dynamic_cast<LogicGate*>(compPtr);
        if (!gate) continue;

        int idx = 0;
        bool undefinedInput = false;
        for (auto& pin : compPtr->pins()) {
            if (pin->type() == PinType::Input) {
                int val = getNetValue(pin);
                if (val < 0) {
                    warnings << QString("Floating input detected. gate '%1' pin '%2'")
                                .arg(compPtr->label()).arg(pin->name());
                    undefinedInput = true;
                    val = 0;
                }
                gate->setBoolInput(idx++, val != 0);
            }
        }
        if (undefinedInput) {
            gate->setOutputUndefined();
        } else {
            const bool oldOut = gate->boolOutput();
            const bool oldValid = gate->outputValid();
            gate->evaluate();
            const bool evaluated = gate->boolOutput();
            gate->forceOutputState(oldOut, oldValid);
            gate->applyEvaluatedOutput(evaluated, m_dt);
        }
    }

    for (auto& comp : m_graph->components()) {
        if (auto* dff = dynamic_cast<DFlipFlop*>(comp.get())) {
            int dVal = -1, clkVal = -1;
            for (auto& pin : comp->pins()) {
                if (pin->name() == "D")   dVal   = getNetValue(pin);
                if (pin->name() == "CLK") clkVal = getNetValue(pin);
            }
            if (dVal < 0 || clkVal < 0) {
                dff->setUndefined(true);
                warnings << QString("Floating input detected. DFF '%1'").arg(comp->label());
            } else {
                dff->setUndefined(false);
                dff->setD(dVal > 0);
                dff->setClk(clkVal > 0);
            }
        }
    }
}

// ── Analog phase: small MNA/backward Euler solver ────────────────────────────

bool SimulationEngine::solveAnalogStep(QStringList& warnings)
{
    m_pinVoltages.clear();
    if (!m_graph) return true;

    const auto& nodes = m_graph->netNodes();
    if (nodes.empty()) return true;

    std::unordered_map<const Pin*, int> pinToNode;
    std::vector<bool> isGround(nodes.size(), false);

    for (const auto& node : nodes) {
        if (node.id < 0 || node.id >= static_cast<int>(nodes.size())) continue;
        for (const auto& p : node.pins) {
            if (!p) continue;
            pinToNode[p.get()] = node.id;
            if (p->type() == PinType::Ground) isGround[node.id] = true;
        }
    }

    // Use node 0 as fallback reference if no explicit GND exists. This prevents a singular matrix.
    bool anyGround = std::any_of(isGround.begin(), isGround.end(), [](bool v){ return v; });
    if (!anyGround && !nodes.empty()) {
        isGround[0] = true;
        warnings << "No ground node found. Using one arbitrary net as 0V reference for analog solve.";
    }

    std::unordered_map<int,int> nodeVar;
    int variableCount = 0;
    for (const auto& node : nodes) {
        if (node.id < 0) continue;
        if (!isGround[node.id]) nodeVar[node.id] = variableCount++;
    }

    auto nodeOf = [&](const std::shared_ptr<Pin>& p) -> int {
        if (!p) return -1;
        auto it = pinToNode.find(p.get());
        return it == pinToNode.end() ? -1 : it->second;
    };
    auto varOfNode = [&](int nodeId) -> int {
        if (nodeId < 0) return -1;
        if (nodeId >= static_cast<int>(isGround.size())) return -1;
        if (isGround[nodeId]) return -1;
        auto it = nodeVar.find(nodeId);
        return it == nodeVar.end() ? -1 : it->second;
    };
    auto pinVoltageFromSolution = [&](const std::shared_ptr<Pin>& p, const std::vector<double>& x) -> double {
        int n = nodeOf(p);
        int v = varOfNode(n);
        return v >= 0 && v < static_cast<int>(x.size()) ? x[v] : 0.0;
    };

    struct VSource { int np; int nm; double value; QString label; };
    std::vector<VSource> voltageSources;

    bool sourceConstraintConflict = false;
    auto addVoltageSource = [&](int np, int nm, double value, const QString& label) {
        if (np < 0 && nm < 0) return;

        // A source whose two terminals have already collapsed to the same net is
        // either redundant (0V) or a hard short (non-zero voltage). Never stamp
        // an all-zero MNA source row, because that would make the matrix singular.
        if (np == nm || (varOfNode(np) < 0 && varOfNode(nm) < 0)) {
            if (!nearlyEqual(value, 0.0, 1e-9)) {
                warnings << QString("Short circuit: ideal source '%1' forces %2 V across the same/reference net.")
                                .arg(label).arg(value, 0, 'g', 10);
                sourceConstraintConflict = true;
            }
            return;
        }

        // Avoid duplicate/reversed ideal source constraints. Equal constraints
        // are redundant; contradictory constraints are a blocking electrical fault.
        for (const auto& s : voltageSources) {
            if (s.np == np && s.nm == nm) {
                if (!nearlyEqual(s.value, value, 1e-6)) {
                    warnings << QString("Conflicting voltage sources on same net pair: %1 vs %2").arg(s.label, label);
                    sourceConstraintConflict = true;
                }
                return;
            }
            if (s.np == nm && s.nm == np) {
                if (!nearlyEqual(s.value, -value, 1e-6)) {
                    warnings << QString("Conflicting reversed voltage sources on same net pair: %1 vs %2").arg(s.label, label);
                    sourceConstraintConflict = true;
                }
                return;
            }
        }
        voltageSources.push_back({np, nm, value, label});
    };

    // First collect ideal voltage sources: analog sources and digital outputs.
    for (auto& comp : m_graph->components()) {
        if (!comp) continue;

        if (auto* src = dynamic_cast<DCVoltageSource*>(comp.get())) {
            addVoltageSource(nodeOf(pinNamed(comp,"POS")), nodeOf(pinNamed(comp,"NEG")), src->voltage(), comp->label());
        } else if (auto* bat = dynamic_cast<Battery*>(comp.get())) {
            addVoltageSource(nodeOf(pinNamed(comp,"POS")), nodeOf(pinNamed(comp,"NEG")), bat->voltage(), comp->label());
        } else if (auto* clk = dynamic_cast<ClockGenerator*>(comp.get())) {
            addVoltageSource(nodeOf(firstPinByType(comp, PinType::Output)), -1, clk->currentOutput() ? 5.0 : 0.0, comp->label());
        } else if (auto* btn = dynamic_cast<PushButton*>(comp.get())) {
            addVoltageSource(nodeOf(pinNamed(comp,"OUT")), -1,
                             btn->pressed() ? 5.0 : 0.0,
                             comp->label()+":OUT");
        } else if (auto* gate = dynamic_cast<LogicGate*>(comp.get())) {
            if (gate->outputValid()) addVoltageSource(nodeOf(pinNamed(comp,"OUT")), -1, gate->boolOutput() ? 5.0 : 0.0, comp->label());
        } else if (auto* dff = dynamic_cast<DFlipFlop*>(comp.get())) {
            if (dff->outputValid()) {
                addVoltageSource(nodeOf(pinNamed(comp,"Q")),  -1, dff->q()    ? 5.0 : 0.0, comp->label()+":Q");
                addVoltageSource(nodeOf(pinNamed(comp,"QB")), -1, dff->qBar() ? 5.0 : 0.0, comp->label()+":QB");
            }
        } else if (auto* adc = dynamic_cast<SimpleADC*>(comp.get())) {
            const uint32_t out = adc->digitalOutput();
            for (auto& p : comp->pins()) {
                if (!p || !p->name().startsWith("D")) continue;
                bool ok = false;
                int bit = p->name().mid(1).toInt(&ok);
                if (ok) addVoltageSource(nodeOf(p), -1, ((out >> bit) & 1u) ? 5.0 : 0.0, comp->label()+":"+p->name());
            }
        } else if (auto* dac = dynamic_cast<SimpleDAC*>(comp.get())) {
            addVoltageSource(nodeOf(pinNamed(comp,"VOUT")), -1, dac->analogOutput(), comp->label()+":VOUT");
        } else if (auto* mcu = dynamic_cast<Microcontroller*>(comp.get())) {
            for (int port = 0; port < Microcontroller::portCount(); ++port) {
                for (int bit = 0; bit < 8; ++bit) {
                    if (!mcu->portBitIsOutput(port, bit)) continue;
                    addVoltageSource(nodeOf(pinNamed(comp, QString("P%1.%2").arg(port).arg(bit))), -1,
                                     mcu->outputBit(port, bit) ? 5.0 : 0.0,
                                     comp->label()+QString(":P%1.%2").arg(port).arg(bit));
                }
            }
        } else if (auto* mem = dynamic_cast<ExternalMemory*>(comp.get())) {
            const int rd = getNetValue(pinNamed(comp, "RD"));
            const int wr = getNetValue(pinNamed(comp, "WR"));
            if (rd == 0 && wr != 0) {
                quint16 addr = 0;
                for (int i = 0; i < 8; ++i)
                    if (getNetValue(pinNamed(comp, QString("A%1").arg(i))) > 0)
                        addr |= quint16(1u << i);
                const quint8 data = mem->read(addr);
                for (int i = 0; i < 8; ++i) {
                    addVoltageSource(nodeOf(pinNamed(comp, QString("D%1").arg(i))), -1,
                                     ((data >> i) & 1u) ? 5.0 : 0.0,
                                     comp->label()+QString(":D%1").arg(i));
                }
            }
        }
    }

    if (sourceConstraintConflict)
        return false;

    const int matrixSize = variableCount + static_cast<int>(voltageSources.size());
    if (matrixSize <= 0) {
        // All nets are ground or no unknowns. Set all known pins to 0.
        for (const auto& node : nodes)
            for (auto& p : node.pins)
                if (p) m_pinVoltages[p.get()] = 0.0;
        return true;
    }

    std::vector<std::vector<double>> A(matrixSize, std::vector<double>(matrixSize, 0.0));
    std::vector<double> b(matrixSize, 0.0);

    auto stampConductance = [&](int na, int nb, double g) {
        if (!std::isfinite(g) || g <= 0.0) return;
        g = std::clamp(g, kMinConductance, kMaxConductance);
        const int ia = varOfNode(na);
        const int ib = varOfNode(nb);
        if (ia >= 0) A[ia][ia] += g;
        if (ib >= 0) A[ib][ib] += g;
        if (ia >= 0 && ib >= 0) {
            A[ia][ib] -= g;
            A[ib][ia] -= g;
        }
    };

    // Current source positive current from node A to node B.
    auto stampCurrentSourceAtoB = [&](int na, int nb, double current) {
        if (!std::isfinite(current) || std::abs(current) < 1e-18) return;
        const int ia = varOfNode(na);
        const int ib = varOfNode(nb);
        if (ia >= 0) b[ia] -= current;
        if (ib >= 0) b[ib] += current;
    };

    // Stamp passive/dynamic elements.
    for (auto& comp : m_graph->components()) {
        if (!comp) continue;

        const int nA = nodeOf(pinNamed(comp,"A"));
        const int nB = nodeOf(pinNamed(comp,"B"));

        if (auto* r = dynamic_cast<Resistor*>(comp.get())) {
            const double R = std::max(1e-9, r->resistance());
            stampConductance(nA, nB, 1.0 / R);
        } else if (auto* c = dynamic_cast<Capacitor*>(comp.get())) {
            const double C = c->capacitance();
            if (C > 0.0 && m_dt > 0.0) {
                const double g = C / m_dt;
                const double vPrev = m_capVoltageHistory[comp->id()];
                stampConductance(nA, nB, g);
                // Backward-Euler companion: conductance plus history source.
                // This holds capacitor voltage across ticks instead of resetting it to zero.
                if (varOfNode(nA) >= 0) b[varOfNode(nA)] += g * vPrev;
                if (varOfNode(nB) >= 0) b[varOfNode(nB)] -= g * vPrev;
            }
        } else if (auto* l = dynamic_cast<Inductor*>(comp.get())) {
            const double L = l->inductance();
            if (L > 0.0 && m_dt > 0.0) {
                const double g = m_dt / L;
                const double iPrev = m_inductorCurrentHistory[comp->id()]; // A -> B
                stampConductance(nA, nB, g);
                stampCurrentSourceAtoB(nA, nB, iPrev);
            }
        } else if (auto* pot = dynamic_cast<Potentiometer*>(comp.get())) {
            const double R = std::max(1.0, pot->resistance());
            const double w = std::clamp(pot->wiper(), 0.0, 1.0);
            constexpr double endpointEps = 1e-12;

            // Exact 0 and 1 are handled topologically in CircuitGraph::buildNetlist().
            // Stamp only the non-zero resistance sections; no epsilon resistance,
            // no division by zero, and no artificial near-short conductance.
            if (w > endpointEps)
                stampConductance(nodeOf(pinNamed(comp,"A")), nodeOf(pinNamed(comp,"W")), 1.0 / (R * w));
            if ((1.0 - w) > endpointEps)
                stampConductance(nodeOf(pinNamed(comp,"W")), nodeOf(pinNamed(comp,"B")), 1.0 / (R * (1.0 - w)));
        } else if (auto* sw = dynamic_cast<Switch*>(comp.get())) {
            Q_UNUSED(sw);
            // Ideal topology switch: CircuitGraph::buildNetlist() merges A and B
            // into the exact same net only when the switch is closed. When open,
            // the nets remain completely disconnected. Do not approximate either
            // state with a very small or very large resistance here.
        } else if (auto* am = dynamic_cast<Ammeter*>(comp.get())) {
            Q_UNUSED(am);
            stampConductance(nodeOf(pinNamed(comp,"IN")), nodeOf(pinNamed(comp,"OUT")), 1.0 / kAmmeterResistance);
        } else if (dynamic_cast<LED*>(comp.get())) {
            const int nAnode = nodeOf(pinNamed(comp,"A"));
            const int nCathode = nodeOf(pinNamed(comp,"K"));
            const double vPrev = getNetVoltage(pinNamed(comp,"A")) - getNetVoltage(pinNamed(comp,"K"));
            const double R = (vPrev > kLedThreshold) ? kLedOnResistance : kLedLeakResistance;
            stampConductance(nAnode, nCathode, 1.0 / R);
        }
    }

    // Stamp ideal voltage sources.
    for (int k = 0; k < static_cast<int>(voltageSources.size()); ++k) {
        const auto& s = voltageSources[k];
        const int row = variableCount + k;
        const int ip = varOfNode(s.np);
        const int im = varOfNode(s.nm);
        if (ip >= 0) { A[ip][row] += 1.0; A[row][ip] += 1.0; }
        if (im >= 0) { A[im][row] -= 1.0; A[row][im] -= 1.0; }
        b[row] = s.value;
    }

    // Light diagonal regularization for isolated floating nets. This prevents singular matrices
    // but keeps the node effectively at 0V when unconnected.
    for (int i = 0; i < variableCount; ++i)
        A[i][i] += 1e-12;

    std::vector<double> x;
    std::vector<std::vector<double>> Awork = A;
    std::vector<double> bwork = b;
    if (!solveLinearSystem(Awork, bwork, x)) {
        warnings << "Analog solve failed: singular/ill-conditioned circuit matrix.";
        // Preserve previous voltages instead of wiping them to zero.
        return false;
    }

    // Store solved node voltages per pin.
    for (const auto& node : nodes) {
        double v = 0.0;
        int var = varOfNode(node.id);
        if (var >= 0 && var < static_cast<int>(x.size())) v = x[var];
        for (auto& p : node.pins)
            if (p) m_pinVoltages[p.get()] = v;
    }

    // Update dynamic histories and ammeter readings.
    for (auto& comp : m_graph->components()) {
        if (!comp) continue;
        if (dynamic_cast<Capacitor*>(comp.get())) {
            const double va = pinVoltageFromSolution(pinNamed(comp,"A"), x);
            const double vb = pinVoltageFromSolution(pinNamed(comp,"B"), x);
            m_capVoltageHistory[comp->id()] = va - vb;
        } else if (auto* l = dynamic_cast<Inductor*>(comp.get())) {
            const double va = pinVoltageFromSolution(pinNamed(comp,"A"), x);
            const double vb = pinVoltageFromSolution(pinNamed(comp,"B"), x);
            const double L = std::max(1e-12, l->inductance());
            m_inductorCurrentHistory[comp->id()] += (m_dt / L) * (va - vb);
        } else if (dynamic_cast<Ammeter*>(comp.get())) {
            const double vin = pinVoltageFromSolution(pinNamed(comp,"IN"), x);
            const double vout = pinVoltageFromSolution(pinNamed(comp,"OUT"), x);
            m_ammeterCurrent[comp->id()] = (vin - vout) / kAmmeterResistance;
        }
    }
    return true;
}

void SimulationEngine::updateAnalogConsumers()
{
    for (auto& comp : m_graph->components()) {
        if (!comp) continue;

        if (auto* led = dynamic_cast<LED*>(comp.get())) {
            const double va = getNetVoltage(pinNamed(comp,"A"));
            const double vk = getNetVoltage(pinNamed(comp,"K"));
            led->setOn((va - vk) > kLedThreshold);
        } else if (auto* seven = dynamic_cast<SevenSegment*>(comp.get())) {
            const double common = getNetVoltage(pinNamed(comp, "COM"));
            const QString names[] = {"A","B","C","D","E","F","G","DP"};
            uint8_t mask = 0;
            for (int i = 0; i < 8; ++i) {
                const auto pin = pinNamed(comp, names[i]);
                if (pin && (getNetVoltage(pin) - common) > kLedThreshold)
                    mask |= static_cast<uint8_t>(1u << i);
            }
            seven->setSegments(mask);
        } else if (auto* probe = dynamic_cast<VoltageProbe*>(comp.get())) {
            probe->setVoltage(getNetVoltage(pinNamed(comp,"IN")));
        } else if (auto* vm = dynamic_cast<Voltmeter*>(comp.get())) {
            const double pos = getNetVoltage(pinNamed(comp,"POS"));
            const double neg = getNetVoltage(pinNamed(comp,"NEG"));
            vm->setReading(pos - neg);
        } else if (auto* am = dynamic_cast<Ammeter*>(comp.get())) {
            auto it = m_ammeterCurrent.find(comp->id());
            am->setReading(it == m_ammeterCurrent.end() ? 0.0 : it->second);
        } else if (auto* osc = dynamic_cast<Oscilloscope*>(comp.get())) {
            const double ch1 = getNetVoltage(pinNamed(comp,"CH1"));
            const double ch2 = getNetVoltage(pinNamed(comp,"CH2"));
            const double ref = getNetVoltage(pinNamed(comp,"GND"));
            osc->pushSample(ch1 - ref, ch2 - ref, m_time);
        } else if (auto* adc = dynamic_cast<SimpleADC*>(comp.get())) {
            const double vin = getNetVoltage(pinNamed(comp,"VIN"));
            const double vp = getNetVoltage(pinNamed(comp,"VREF+"));
            const double vmn = getNetVoltage(pinNamed(comp,"VREF-"));
            adc->setInputVoltage(vin);
            adc->setVRef(vmn, vp);
            adc->tickConversion(m_dt);
        } else if (auto* dac = dynamic_cast<SimpleDAC*>(comp.get())) {
            const double vp = getNetVoltage(pinNamed(comp,"VREF+"));
            const double vmn = getNetVoltage(pinNamed(comp,"VREF-"));
            dac->setVRef(vmn, vp);
            dac->tickConversion(m_dt);
        } else if (auto* lcd = dynamic_cast<LCD16x2*>(comp.get())) {
            quint8 data = 0;
            for (int i=0;i<8;++i) if (getNetValue(pinNamed(comp, QString("D%1").arg(i))) > 0) data |= (1u<<i);
            lcd->tickBus(getNetValue(pinNamed(comp,"RS"))>0, getNetValue(pinNamed(comp,"RW"))>0,
                         getNetValue(pinNamed(comp,"E"))>0, data);
        } else if (auto* mem = dynamic_cast<ExternalMemory*>(comp.get())) {
            const int rd = getNetValue(pinNamed(comp, "RD"));
            const int wr = getNetValue(pinNamed(comp, "WR"));
            if (wr == 0 && rd != 0) {
                quint16 addr = 0;
                quint8 data = 0;
                for (int i = 0; i < 8; ++i) {
                    if (getNetValue(pinNamed(comp, QString("A%1").arg(i))) > 0)
                        addr |= quint16(1u << i);
                    if (getNetValue(pinNamed(comp, QString("D%1").arg(i))) > 0)
                        data |= quint8(1u << i);
                }
                mem->write(addr, data);
            }
        }
    }

    // Refresh GPIO input samples after the analog solve as well, so analog
    // thresholds and memory/keypad bus changes are ready for the next CPU tick.
    sampleMicrocontrollerInputs();
}


void SimulationEngine::updateKeypadLcdEchoDemo()
{
    if (!m_graph) return;

    LCD16x2* echoLcd = nullptr;
    for (auto& comp : m_graph->components()) {
        if (!comp) continue;
        auto* lcd = dynamic_cast<LCD16x2*>(comp.get());
        if (!lcd) continue;
        const QString label = lcd->label().toUpper();
        if (label.contains("LCD_ECHO") || label.contains("KEYPAD_LCD")) {
            echoLcd = lcd;
            break;
        }
    }
    if (!echoLcd) return;

    for (auto& comp : m_graph->components()) {
        if (!comp) continue;
        auto* keypad = dynamic_cast<Keypad*>(comp.get());
        if (!keypad) continue;

        const QString label = keypad->label().toUpper();
        if (!label.contains("KEYPAD_ECHO") && !label.contains("KEYPAD_LCD"))
            continue;

        const ComponentID kid = keypad->id();
        const QString key = keypad->pressedKey().trimmed().toUpper();
        if (key.isEmpty() || key == "NONE") {
            m_keypadEchoLastKey[kid].clear();
            continue;
        }

        if (m_keypadEchoLastKey[kid] == key)
            continue;
        m_keypadEchoLastKey[kid] = key;

        QString& text = m_keypadEchoText[echoLcd->id()];
        if (text.size() >= 32)
            text.clear();
        text += key.left(1);

        const QString title = QStringLiteral("KEYPAD INPUT").leftJustified(16, QLatin1Char(' '), true);
        const QString recent = text.right(16).leftJustified(16, QLatin1Char(' '), true);
        echoLcd->setText(title, recent);
    }
}

// ── Main tick ────────────────────────────────────────────────────────────────

void SimulationEngine::tick()
{
    QStringList warnings;

    if (!m_graph) return;
    m_graph->buildNetlist();
    rebuildReferencedNodes();

    // Page-24 DRC rule: contradictory connected drivers are blocking faults.
    // Check before touching the matrix so a short can never destabilize MNA.
    QStringList faults = detectBlockingFaults();
    if (!faults.isEmpty()) {
        faults.prepend("Simulation stopped by DRC: blocking circuit fault detected.");
        emit tickDone(faults, computeWireNetValues());
        stop();
        return;
    }

    // Digital first. A clock edge or gate transition can create a new conflict,
    // so validate once more before stamping ideal sources into the analog matrix.
    simulateDigital(warnings);
    rebuildReferencedNodes();
    faults = detectBlockingFaults();
    if (!faults.isEmpty()) {
        faults.prepend("Simulation stopped by DRC after the circuit state changed into a blocking fault.");
        emit tickDone(faults, computeWireNetValues());
        stop();
        return;
    }

    if (!solveAnalogStep(warnings)) {
        warnings.prepend("Simulation stopped: unable to compute circuit outputs safely.");
        emit tickDone(warnings, computeWireNetValues());
        stop();
        return;
    }

    updateAnalogConsumers();
    updateKeypadLcdEchoDemo();
    m_time += m_dt;

    auto drc = m_graph->runDRC();
    for (const auto& w : drc)
        if (!warnings.contains(w)) warnings << w;

    auto netValues = computeWireNetValues();
    emit tickDone(warnings, netValues);
}
