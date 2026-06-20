#pragma once
#include "../Component.h"
class Voltmeter : public Component {
public:
    Voltmeter();
    double reading() const { return m_reading; }
    void setReading(double v) { m_reading = v; }
    QRectF boundingBox() const override;
    void draw(QPainter& painter, bool selected) const override;
    QJsonObject serialize() const override;
    void deserialize(const QJsonObject& obj) override;
    QMap<QString,QString> properties() const override;
    void setProperty(const QString& key, const QString& value) override;
private: double m_reading{0.0};
};
