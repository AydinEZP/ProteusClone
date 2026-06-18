#pragma once
#include "../Component.h"
#include <vector>
#include <QtGlobal>

/**
 * Abstract base for all logic gates.
 *
 * The project spec requires three logic regions: LOW, HIGH and Undefined.
 * This base class therefore stores both the boolean output value and whether
 * that value is currently valid.  It also owns a per-instance propagation
 * delay that can be edited from the Properties panel.
 */
class LogicGate : public Component {
public:
    explicit LogicGate(const QString& gateType, int numInputs = 2);
    virtual ~LogicGate() = default;

    static constexpr double LowVoltage       = 0.0;
    static constexpr double HighVoltage      = 5.0;
    static constexpr double LowThresholdMax  = 0.8;
    static constexpr double HighThresholdMin = 2.0;

    void setBoolInput(int index, bool value);
    bool boolOutput() const { return m_output; }
    bool outputValid() const { return m_outputValid; }
    void setOutputUndefined() { m_outputValid = false; m_pendingValid = false; }
    void forceOutputState(bool value, bool valid) { m_output = value; m_outputValid = valid; }

    double propagationDelayMs() const { return m_propagationDelayMs; }
    void setPropagationDelayMs(double ms) { m_propagationDelayMs = qMax(0.0, ms); }

    /**
     * Applies a newly evaluated output while respecting propagation delay.
     * If delay is zero, output updates immediately. Otherwise the previous
     * output is held until the requested delay has elapsed.
     */
    void applyEvaluatedOutput(bool value, double dtSeconds);

    /** Evaluate truth table given current inputs, update m_output target. */
    virtual void evaluate() = 0;

    QRectF boundingBox() const override;
    void   draw(QPainter& painter, bool selected) const override;

    QJsonObject serialize() const override;
    void deserialize(const QJsonObject& obj) override;
    QMap<QString,QString> properties() const override;
    void setProperty(const QString& key, const QString& value) override;

protected:
    void setRawOutput(bool value) { m_output = value; m_outputValid = true; }

    std::vector<bool> m_inputs;
    bool              m_output {false};
    bool              m_outputValid {true};
    QString           m_gateLabel;
    int               m_numInputs;

private:
    double m_propagationDelayMs {1.0};
    bool   m_pendingValid {false};
    bool   m_pendingOutput {false};
    double m_pendingElapsedMs {0.0};
};
