#include "DCVoltageSource.h"
#include <QPainter>

DCVoltageSource::DCVoltageSource()
    : Component("DCVoltageSource")
{
    setLabel("V?");
    addPin(std::make_shared<Pin>("POS", PinType::Power,  QPointF(0, -30)));
    addPin(std::make_shared<Pin>("NEG", PinType::Ground, QPointF(0,  30)));
    updatePinWorldPositions();
}

QRectF DCVoltageSource::boundingBox() const
{
    return QRectF(m_pos.x()-20, m_pos.y()-32, 40, 64);
}

void DCVoltageSource::draw(QPainter& painter, bool selected) const
{
    painter.save();
    painter.translate(m_pos);
    painter.rotate(m_rotation);
    if (m_mirrorH) painter.scale(-1, 1);
    if (m_mirrorV) painter.scale(1, -1);

    QPen pen(selected ? Qt::cyan : Qt::darkRed, 2);
    painter.setPen(pen);
    painter.setBrush(Qt::NoBrush);

    painter.drawLine(QPointF(0,-30), QPointF(0,-16));
    painter.drawLine(QPointF(0, 16), QPointF(0, 30));
    painter.drawEllipse(QRectF(-14,-14,28,28));

    // + and - symbols
    painter.setFont(QFont("Monospace", 8, QFont::Bold));
    painter.drawText(QRectF(-6,-14, 12, 14), Qt::AlignCenter, "+");
    painter.drawText(QRectF(-6,  0, 12, 14), Qt::AlignCenter, "−");

    painter.setPen(selected ? Qt::cyan : Qt::black);
    painter.setFont(QFont("Monospace", 7));
    painter.drawText(QRectF(-30,-46,60,14), Qt::AlignCenter,
                     QString("%1 %2V").arg(m_label).arg(m_voltage));

    painter.setPen(QPen(Qt::red, 1));
    for (auto& pin : m_pins) {
        painter.setBrush(pin->highlighted() ? Qt::yellow : Qt::red);
        painter.drawEllipse(pin->localPos(), Pin::HoverRadius, Pin::HoverRadius);
    }
    painter.restore();
}

QJsonObject DCVoltageSource::serialize() const
{
    QJsonObject obj = Component::serialize();
    obj["voltage"] = m_voltage;
    return obj;
}

void DCVoltageSource::deserialize(const QJsonObject& obj)
{
    Component::deserialize(obj);
    m_voltage = obj["voltage"].toDouble(5.0);
}

QMap<QString,QString> DCVoltageSource::properties() const
{
    QMap<QString,QString> p;
    p["label"]   = m_label;
    p["voltage"] = QString::number(m_voltage);
    return p;
}

void DCVoltageSource::setProperty(const QString& key, const QString& value)
{
    if (key == "label")   m_label   = value;
    if (key == "voltage") m_voltage = value.toDouble();
}