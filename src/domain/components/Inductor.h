#pragma once
#include "../Component.h"

class Inductor : public Component {
public:
    Inductor();
    double inductance() const { return m_inductance; }

    QRectF boundingBox() const override;
    void   draw(QPainter& painter, bool selected) const override;
    QJsonObject           serialize()   const override;
    void                  deserialize(const QJsonObject& obj) override;
    QMap<QString,QString> properties()  const override;
    void                  setProperty(const QString& key, const QString& value) override;

private:
    double m_inductance {1e-3}; // Henrys
};