#pragma once
#include "../Component.h"
class Ammeter : public Component {
public:
    Ammeter();
    double reading() const { return m_reading; }
    void setReading(double a) { m_reading = a; }
    QRectF boundingBox() const override;
    void draw(QPainter& painter, bool selected) const override;
    QJsonObject serialize() const override;
    void deserialize(const QJsonObject& obj) override;
    QMap<QString,QString> properties() const override;
    void setProperty(const QString& key, const QString& value) override;
private: double m_reading{0.0};
};
