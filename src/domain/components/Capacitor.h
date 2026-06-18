#pragma once
#include "../Component.h"

class Capacitor : public Component {
public:
    Capacitor();
    double capacitance() const { return m_capacitance; }
    void   setCapacitance(double c) { if (c >= 0.0) m_capacitance = c; }

    QRectF boundingBox() const override;
    void   draw(QPainter& painter, bool selected) const override;
    QJsonObject           serialize()   const override;
    void                  deserialize(const QJsonObject& obj) override;
    QMap<QString,QString> properties()  const override;
    void                  setProperty(const QString& key, const QString& value) override;

private:
    double m_capacitance {100e-9}; // Farads
};