#pragma once
#include "../Component.h"

class PushButton : public Component {
public:
    PushButton();
    bool pressed() const { return m_pressed; }
    void setPressed(bool p) { m_pressed = p; }

    QRectF boundingBox() const override;
    void   draw(QPainter& painter, bool selected) const override;
    QJsonObject serialize()  const override;
    void        deserialize(const QJsonObject& obj) override;

private:
    bool m_pressed {false};
};