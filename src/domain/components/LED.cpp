#include "LED.h"
#include <QPainter>

LED::LED()
    : Component("LED")
{
    setLabel("D?");
    addPin(std::make_shared<Pin>("A", PinType::Input,  QPointF(-25, 0))); // Anode
    addPin(std::make_shared<Pin>("K", PinType::Output, QPointF( 25, 0))); // Cathode
    updatePinWorldPositions();
}

QRectF LED::boundingBox() const
{
    return QRectF(m_pos.x()-27, m_pos.y()-16, 54, 32);
}

void LED::draw(QPainter& painter, bool selected) const
{
    painter.save();
    painter.translate(m_pos);
    painter.rotate(m_rotation);
    if (m_mirrorH) painter.scale(-1, 1);

    QPen pen(selected ? Qt::cyan : Qt::darkRed, 2);
    painter.setPen(pen);

    // Lead lines
    painter.drawLine(QPointF(-25,0), QPointF(-12,0));
    painter.drawLine(QPointF( 25,0), QPointF( 12,0));

    // Triangle (diode body)
    QPolygonF tri;
    tri << QPointF(-12,-10) << QPointF(-12,10) << QPointF(12,0);
    painter.setBrush(m_on ? m_color : QColor(m_color.red()/3, m_color.green()/3, m_color.blue()/3));
    painter.drawPolygon(tri);

    // Cathode line
    painter.drawLine(QPointF(12,-10), QPointF(12,10));

    // Emission arrows (when on)
    if (m_on) {
        painter.setPen(QPen(m_color, 1));
        painter.drawLine(QPointF(4,-12), QPointF(12,-20));
        painter.drawLine(QPointF(9,-12), QPointF(17,-20));
    }

    painter.setPen(selected ? Qt::cyan : Qt::black);
    painter.setFont(QFont("Monospace", 7));
    painter.drawText(QRectF(-25,-28,50,14), Qt::AlignCenter, m_label);

    painter.setPen(QPen(Qt::red, 1));
    for (auto& pin : m_pins) {
        painter.setBrush(pin->highlighted() ? Qt::yellow : Qt::red);
        painter.drawEllipse(pin->localPos(), Pin::HoverRadius, Pin::HoverRadius);
    }
    painter.restore();
}

QJsonObject LED::serialize() const
{
    QJsonObject obj = Component::serialize();
    obj["color_r"] = m_color.red();
    obj["color_g"] = m_color.green();
    obj["color_b"] = m_color.blue();
    return obj;
}

void LED::deserialize(const QJsonObject& obj)
{
    Component::deserialize(obj);
    m_color = QColor(obj["color_r"].toInt(255),
                     obj["color_g"].toInt(0),
                     obj["color_b"].toInt(0));
}

QMap<QString,QString> LED::properties() const
{
    QMap<QString,QString> p;
    p["label"] = m_label;
    p["color"] = m_color.name();
    return p;
}

void LED::setProperty(const QString& key, const QString& value)
{
    if (key == "label") m_label  = value;
    if (key == "color") m_color  = QColor(value);
}