#include "Potentiometer.h"
#include <QPainter>
#include <QJsonObject>
#include <algorithm>

Potentiometer::Potentiometer()
    : Component("Potentiometer")
{
    setLabel("POT?");
    addPin(std::make_shared<Pin>("A", PinType::Passive, QPointF(-32, 0)));
    addPin(std::make_shared<Pin>("B", PinType::Passive, QPointF( 32, 0)));
    addPin(std::make_shared<Pin>("W", PinType::Passive, QPointF(  0,-28)));
    updatePinWorldPositions();
}

QRectF Potentiometer::boundingBox() const
{
    return QRectF(m_pos.x()-36, m_pos.y()-32, 72, 50);
}

void Potentiometer::draw(QPainter& painter, bool selected) const
{
    painter.save();
    painter.translate(m_pos);
    painter.rotate(m_rotation);
    painter.setPen(QPen(selected ? Qt::cyan : Qt::darkGreen, 2));
    painter.setBrush(Qt::NoBrush);
    painter.drawRect(QRectF(-20,-8,40,16));
    painter.drawLine(QPointF(-32,0), QPointF(-20,0));
    painter.drawLine(QPointF(20,0), QPointF(32,0));
    painter.drawLine(QPointF(0,-28), QPointF(0,-8));
    painter.drawLine(QPointF(-8,-18), QPointF(0,-8));
    painter.setFont(QFont("Monospace",7));
    painter.drawText(QRectF(-34,-42,68,14), Qt::AlignCenter,
                     QString("%1 %2%").arg(m_label).arg(qRound(m_wiper * 100.0)));
    drawPinLabels(painter, QRectF(-20,-8,40,16), Qt::darkGreen, 5);
    painter.setPen(QPen(Qt::red,1));
    for (auto& pin : m_pins) {
        painter.setBrush(pin->highlighted() ? Qt::yellow : Qt::red);
        painter.drawEllipse(pin->localPos(), Pin::HoverRadius, Pin::HoverRadius);
    }
    painter.restore();
}

QJsonObject Potentiometer::serialize() const
{
    auto o = Component::serialize();
    o["resistance"] = m_resistance;
    o["wiper"] = m_wiper;
    return o;
}

void Potentiometer::deserialize(const QJsonObject& obj)
{
    Component::deserialize(obj);
    m_resistance = std::max(1.0, obj["resistance"].toDouble(10000.0));
    // End points are valid positions: 0.0 means W is exactly at A, 1.0 means W is exactly at B.
    m_wiper = std::clamp(obj["wiper"].toDouble(0.5), 0.0, 1.0);
}

QMap<QString,QString> Potentiometer::properties() const
{
    return {{"label",m_label},
            {"resistance",QString::number(m_resistance)},
            {"wiper",QString::number(m_wiper, 'g', 12)}};
}

void Potentiometer::setProperty(const QString& key, const QString& value)
{
    if (key == "label") {
        m_label = value;
    } else if (key == "resistance") {
        bool ok = false;
        const double parsed = value.toDouble(&ok);
        if (ok) m_resistance = std::max(1.0, parsed);
    } else if (key == "wiper") {
        bool ok = false;
        const double parsed = value.toDouble(&ok);
        if (ok) m_wiper = std::clamp(parsed, 0.0, 1.0);
    }
}
