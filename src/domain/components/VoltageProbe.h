#pragma once
#include "../Component.h"

class VoltageProbe : public Component {
public:
    VoltageProbe();
    double voltage() const { return m_voltage; }
    void setVoltage(double v) { m_voltage = v; }
    QRectF boundingBox() const override;
    void draw(QPainter& painter, bool selected) const override;
    QJsonObject serialize() const override;
    void deserialize(const QJsonObject& obj) override;
    QMap<QString,QString> properties() const override;
    void setProperty(const QString& key, const QString& value) override;
private:
    double m_voltage {0.0};
};
