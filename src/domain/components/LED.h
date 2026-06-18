#pragma once
#include "../Component.h"
#include <QColor>

class LED : public Component {
public:
    LED();

    bool   isOn()   const { return m_on; }
    void   setOn(bool on) { m_on = on; }
    QColor color()  const { return m_color; }

    QRectF boundingBox() const override;
    void   draw(QPainter& painter, bool selected) const override;
    QJsonObject           serialize()   const override;
    void                  deserialize(const QJsonObject& obj) override;
    QMap<QString,QString> properties()  const override;
    void                  setProperty(const QString& key, const QString& value) override;

private:
    bool   m_on    {false};
    QColor m_color {255, 0, 0}; // red by default
};