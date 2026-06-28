#include "Switch.h"
#include <QPainter>

Switch::Switch()
    : Component("Switch")
{
    setLabel("SW?");
    addPin(std::make_shared<Pin>("A", PinType::Passive, QPointF(-25, 0)));
    addPin(std::make_shared<Pin>("B", PinType::Passive, QPointF( 25, 0)));
    updatePinWorldPositions();
}

QRectF Switch::boundingBox() const
{
    return QRectF(m_pos.x()-27, m_pos.y()-16, 54, 32);
}

void Switch::draw(QPainter& painter, bool selected) const
{
    painter.save();
    painter.translate(m_pos);
    painter.rotate(m_rotation);
    if (m_mirrorH) painter.scale(-1, 1);

    QPen pen(selected ? Qt::cyan : Qt::darkBlue, 2);
    painter.setPen(pen);
    painter.setBrush(Qt::NoBrush);

    // Leads
    painter.drawLine(QPointF(-25,0), QPointF(-12,0));
    painter.drawLine(QPointF( 25,0), QPointF( 12,0));

    // Contacts
    painter.setBrush(Qt::darkBlue);
    painter.drawEllipse(QPointF(-12,0), 2, 2);
    painter.drawEllipse(QPointF( 12,0), 2, 2);

    // Lever
    if (m_closed) {
        painter.drawLine(QPointF(-12,0), QPointF(12,0));
    } else {
        painter.drawLine(QPointF(-12,0), QPointF(10,-10));
    }

    painter.setPen(selected ? Qt::cyan : Qt::black);
    painter.setFont(QFont("Monospace", 7));
    painter.drawText(QRectF(-25,-24,50,14), Qt::AlignCenter, m_label);

    painter.setPen(QPen(Qt::red, 1));
    for (auto& pin : m_pins) {
        painter.setBrush(pin->highlighted() ? Qt::yellow : Qt::red);
        painter.drawEllipse(pin->localPos(), Pin::HoverRadius, Pin::HoverRadius);
    }
    painter.restore();
}

QJsonObject Switch::serialize() const
{
    QJsonObject obj = Component::serialize();
    obj["closed"] = m_closed;
    return obj;
}

void Switch::deserialize(const QJsonObject& obj)
{
    Component::deserialize(obj);
    m_closed = obj["closed"].toBool(false);
}

QMap<QString,QString> Switch::properties() const
{
    QMap<QString,QString> p;
    p["label"]  = m_label;
    p["closed"] = m_closed ? "true" : "false";
    return p;
}

void Switch::setProperty(const QString& key, const QString& value)
{
    if (key == "label")  m_label  = value;
    if (key == "closed") m_closed = (value == "true");
}