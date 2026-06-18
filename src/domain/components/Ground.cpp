#include "Ground.h"
#include <QPainter>

Ground::Ground()
    : Component("Ground")
{
    setLabel("GND");
    addPin(std::make_shared<Pin>("GND", PinType::Ground, QPointF(0,-20)));
    updatePinWorldPositions();
}

QRectF Ground::boundingBox() const
{
    return QRectF(m_pos.x()-16, m_pos.y()-22, 32, 36);
}

void Ground::draw(QPainter& painter, bool selected) const
{
    painter.save();
    painter.translate(m_pos);
    painter.rotate(m_rotation);

    QPen pen(selected ? Qt::cyan : Qt::black, 2);
    painter.setPen(pen);
    painter.drawLine(QPointF(0,-20), QPointF(0, 0));
    painter.drawLine(QPointF(-14,  0), QPointF( 14,  0));
    painter.drawLine(QPointF( -9,  5), QPointF(  9,  5));
    painter.drawLine(QPointF( -4, 10), QPointF(  4, 10));

    painter.setPen(QPen(Qt::red, 1));
    for (auto& pin : m_pins) {
        painter.setBrush(pin->highlighted() ? Qt::yellow : Qt::red);
        painter.drawEllipse(pin->localPos(), Pin::HoverRadius, Pin::HoverRadius);
    }
    painter.restore();
}