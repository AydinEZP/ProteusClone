#pragma once
#include <QObject>
#include <QTimer>
#include <QStringList>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <memory>
#include "../domain/Wire.h"
#include "../graph/CircuitGraph.h"

enum class SimState { Stopped, Running, Paused };

/**
 * Hybrid tick-based simulator.
 *
 * Tick order:
 *  1) Rebuild the netlist from current pins/wires/junctions.
 *  2) Simulate digital components first (clock, gates, flip-flops, ADC/DAC digital side).
 *  3) Treat digital outputs as ideal 0V/5V sources for the analog solve.
 *  4) Solve the analog network with a small MNA/backward-Euler step.
 *  5) Feed the solved node voltages back into probes, meters, scope, LEDs, ADC/DAC state,
 *     and wire coloring.
 *
 * This is not a full SPICE engine. It intentionally implements the subset needed by the
 * OOP project: DC sources/battery, resistors, capacitors, inductors, digital outputs as
 * voltage sources, LED display state, ADC/DAC, meters and oscilloscope.
 */
class SimulationEngine : public QObject {
    Q_OBJECT
public:
    explicit SimulationEngine(CircuitGraph* graph, QObject* parent = nullptr);

    SimState state() const { return m_state; }

public slots:
    void start();
    void pause();
    void stop();
    void step();

signals:
    // warnings: DRC/floating messages this tick
    // netValues: wire id → -1 (float), 0 (LOW), 1 (HIGH)
    void tickDone(QStringList warnings,
                  std::unordered_map<WireID,int> netValues);
    void stateChanged(SimState newState);

private slots:
    void tick();

private:
    int getNetValue(const std::shared_ptr<Pin>& queryPin) const;
    double getNetVoltage(const std::shared_ptr<Pin>& queryPin) const;
    std::unordered_map<WireID,int> computeWireNetValues() const;
    std::vector<Component*> topoSortGates() const;

    void simulateDigital(QStringList& warnings);
    void sampleMicrocontrollerInputs();
    bool solveAnalogStep(QStringList& warnings);
    void updateAnalogConsumers();
    void rebuildReferencedNodes();
    QStringList detectBlockingFaults() const;

    CircuitGraph* m_graph;
    QTimer        m_timer;
    SimState      m_state {SimState::Stopped};
    double        m_dt    {0.01}; // 100 Hz simulation; stable enough for the simple BE analog step
    double        m_time  {0.0};  // simulation time in seconds; used by scope traces

    // Last solved analog voltage per pin. Digital input thresholding and measurement tools read this.
    std::unordered_map<const Pin*, double> m_pinVoltages;

    // Net ids that have a real electrical reference path to GND or to an ideal
    // source. A regularization conductance must never turn an otherwise isolated
    // open-switch node into a fake logic LOW.
    std::unordered_set<int> m_referencedNodes;

    // Dynamic element history for backward Euler companion models.
    // Capacitor history is previous v(A)-v(B). Inductor history is previous current A->B.
    std::unordered_map<ComponentID, double> m_capVoltageHistory;
    std::unordered_map<ComponentID, double> m_inductorCurrentHistory;

    // Estimated ammeter current A->B after the last solve.
    std::unordered_map<ComponentID, double> m_ammeterCurrent;
};
