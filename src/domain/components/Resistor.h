#pragma once
#include "../Component.h"

class Resistor : public Component {
public:
    Resistor();

    double resistance() const { return m_resistance; }
    void   setResistance(double r) { m_resistance = r; }

    QRectF boundingBox() const override;
    void   draw(QPainter& painter, bool selected) const override;

    QJsonObject           serialize()   const override;
    void                  deserialize(const QJsonObject& obj) override;
    QMap<QString,QString> properties()  const override;
    void                  setProperty(const QString& key, const QString& value) override;

private:
    double m_resistance {1000.0}; // Ohms
};