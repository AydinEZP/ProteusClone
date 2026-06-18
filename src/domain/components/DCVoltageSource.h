#pragma once
#include "../Component.h"

class DCVoltageSource : public Component {
public:
    DCVoltageSource();
    double voltage() const { return m_voltage; }

    QRectF boundingBox() const override;
    void   draw(QPainter& painter, bool selected) const override;
    QJsonObject           serialize()   const override;
    void                  deserialize(const QJsonObject& obj) override;
    QMap<QString,QString> properties()  const override;
    void                  setProperty(const QString& key, const QString& value) override;

private:
    double m_voltage {5.0};
};