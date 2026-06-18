#pragma once
#include "../Component.h"

class ClockGenerator : public Component {
public:
    ClockGenerator();

    double frequency() const { return m_frequency; }
    void   setFrequency(double f) { m_frequency = f; }

    // Simulation state
    bool   currentOutput() const { return m_output; }
    void   tick(double dt);          // called by SimulationEngine

    QRectF boundingBox() const override;
    void   draw(QPainter& painter, bool selected) const override;
    QJsonObject           serialize()   const override;
    void                  deserialize(const QJsonObject& obj) override;
    QMap<QString,QString> properties()  const override;
    void                  setProperty(const QString& key, const QString& value) override;

private:
    double m_frequency {1.0};  // Hz
    double m_accumulator {0.0};
    bool   m_output {false};
};