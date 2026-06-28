#include "PushButton.h"
#include <QPainter>

PushButton::PushButton()
    : Component("PushButton")
{
    setLabel("BTN?");
    // A momentary digital source: one output, LOW when idle and HIGH while pressed.
    addPin(std::make_shared<Pin>("OUT", PinType::Output, QPointF(28,0)));
    updatePinWorldPositions();
}

QRectF PushButton::boundingBox() const
{
    return QRectF(m_pos.x()-22, m_pos.y()-18, 54, 36);
}

void PushButton::draw(QPainter& painter, bool selected) const
{
    painter.save();
    painter.translate(m_pos);
    painter.rotate(m_rotation);
    if (m_mirrorH) painter.scale(-1, 1);

    QPen pen(selected ? Qt::cyan : Qt::darkBlue, 2);
    painter.setPen(pen);
    painter.setBrush(m_pressed ? QColor(150,200,255) : Qt::white);
    painter.drawRoundedRect(QRectF(-18,-10,36,20), 4, 4);
    painter.drawLine(QPointF(18,0), QPointF(28,0));

    painter.setPen(selected ? Qt::cyan : Qt::black);
    painter.setFont(QFont("Monospace", 7, QFont::Bold));
    painter.drawText(QRectF(-18,-9,36,18), Qt::AlignCenter,
                     m_pressed ? "HIGH" : "LOW");
    painter.setFont(QFont("Monospace", 6));
    painter.drawText(QRectF(-22,-26,54,12), Qt::AlignCenter, m_label);
    painter.drawText(QRectF(8,4,20,10), Qt::AlignRight | Qt::AlignVCenter, "OUT");

    painter.setPen(QPen(Qt::red, 1));
    for (auto& pin : m_pins) {
        painter.setBrush(pin->highlighted() ? Qt::yellow : Qt::red);
        painter.drawEllipse(pin->localPos(), Pin::HoverRadius, Pin::HoverRadius);
    }
    painter.restore();
}

QJsonObject PushButton::serialize() const
{
    QJsonObject obj = Component::serialize();
    obj["pressed"] = m_pressed;
    return obj;
}

void PushButton::deserialize(const QJsonObject& obj)
{
    Component::deserialize(obj);
    m_pressed = obj["pressed"].toBool(false);
}