#pragma once
#include "../Component.h"

class Switch : public Component {
public:
    Switch();
    bool closed() const { return m_closed; }
    void toggle()       { m_closed = !m_closed; }

    QRectF boundingBox() const override;
    void   draw(QPainter& painter, bool selected) const override;
    QJsonObject           serialize()   const override;
    void                  deserialize(const QJsonObject& obj) override;
    QMap<QString,QString> properties()  const override;
    void                  setProperty(const QString& key, const QString& value) override;

private:
    bool m_closed {false};
};