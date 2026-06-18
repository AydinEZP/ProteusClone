#include "Resistor.h"
#include <QPainter>
#include <QJsonObject>

Resistor::Resistor()
    : Component("Resistor")
{
    setLabel("R?");
    // Two passive pins: left and right
    addPin(std::make_shared<Pin>("A", PinType::Passive, QPointF(-30, 0)));
    addPin(std::make_shared<Pin>("B", PinType::Passive, QPointF( 30, 0)));
    updatePinWorldPositions();
}

QRectF Resistor::boundingBox() const
{
    // Box around component center in world coords (approximation)
    return QRectF(m_pos.x() - 32, m_pos.y() - 12, 64, 24);
}

void Resistor::draw(QPainter& painter, bool selected) const
{
    painter.save();
    painter.translate(m_pos);
    painter.rotate(m_rotation);
    if (m_mirrorH) painter.scale(-1, 1);
    if (m_mirrorV) painter.scale(1, -1);

    QPen pen(selected ? Qt::cyan : Qt::darkGreen, 2);
    painter.setPen(pen);
    painter.setBrush(Qt::NoBrush);

    // Lead lines
    painter.drawLine(QPointF(-30, 0), QPointF(-18, 0));
    painter.drawLine(QPointF( 18, 0), QPointF( 30, 0));

    // Zigzag body
    const QPointF zig[] = {
        {-18, 0}, {-14,-8}, {-8, 8}, {-2,-8}, {4, 8}, {10,-8}, {14, 8}, {18, 0}
    };
    painter.drawPolyline(zig, 8);

    // Label
    painter.setPen(selected ? Qt::cyan : Qt::black);
    painter.setFont(QFont("Monospace", 7));
    painter.drawText(QRectF(-30, -24, 60, 14), Qt::AlignCenter,
                     QString("%1\n%2Ω").arg(m_label).arg(m_resistance));

    // Draw pins as small circles
    painter.setPen(QPen(Qt::red, 1));
    for (auto& pin : m_pins) {
        QPointF local = pin->localPos();
        if (pin->highlighted())
            painter.setBrush(Qt::yellow);
        else
            painter.setBrush(Qt::red);
        painter.drawEllipse(local, Pin::HoverRadius, Pin::HoverRadius);
    }

    painter.restore();
}

QJsonObject Resistor::serialize() const
{
    QJsonObject obj = Component::serialize();
    obj["resistance"] = m_resistance;
    return obj;
}

void Resistor::deserialize(const QJsonObject& obj)
{
    Component::deserialize(obj);
    m_resistance = obj["resistance"].toDouble(1000.0);
}

QMap<QString,QString> Resistor::properties() const
{
    QMap<QString,QString> p;
    p["label"]      = m_label;
    p["resistance"] = QString::number(m_resistance);
    return p;
}

void Resistor::setProperty(const QString& key, const QString& value)
{
    if (key == "label")      m_label      = value;
    if (key == "resistance") m_resistance = value.toDouble();
}