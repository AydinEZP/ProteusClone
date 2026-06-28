#pragma once
#include "../Component.h"

class Potentiometer : public Component {
public:
    Potentiometer();
    double resistance() const { return m_resistance; }
    double wiper() const { return m_wiper; }
    QRectF boundingBox() const override;
    void draw(QPainter& painter, bool selected) const override;
    QJsonObject serialize() const override;
    void deserialize(const QJsonObject& obj) override;
    QMap<QString,QString> properties() const override;
    void setProperty(const QString& key, const QString& value) override;
private:
    double m_resistance {10000.0};
    double m_wiper {0.5};
};
