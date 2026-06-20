#include "VoltageProbe.h"
#include <QPainter>

VoltageProbe::VoltageProbe() : Component("VoltageProbe") {
    setLabel("VP?");
    addPin(std::make_shared<Pin>("IN", PinType::Input, QPointF(0, 22)));
    updatePinWorldPositions();
}

QRectF VoltageProbe::boundingBox() const { return QRectF(m_pos.x()-22, m_pos.y()-18, 44, 46); }

void VoltageProbe::draw(QPainter& painter, bool selected) const {
    painter.save();
    painter.translate(m_pos);
    painter.rotate(m_rotation);
    QPen pen(selected ? Qt::cyan : Qt::darkBlue, 2);
    painter.setPen(pen);
    painter.setBrush(QColor(235,245,255));
    painter.drawEllipse(QPointF(0,0), 16, 16);
    painter.setFont(QFont("Monospace", 7, QFont::Bold));
    painter.drawText(QRectF(-16,-10,32,20), Qt::AlignCenter, "V");
    painter.drawLine(QPointF(0,16), QPointF(0,22));
    painter.setFont(QFont("Monospace", 6));
    painter.setPen(selected ? Qt::cyan : Qt::black);
    painter.drawText(QRectF(-30,-32,60,14), Qt::AlignCenter, QString("%1 %2V").arg(m_label).arg(m_voltage,0,'f',2));
    painter.setPen(QPen(Qt::red,1));
    for (auto& pin : m_pins) { painter.setBrush(pin->highlighted()?Qt::yellow:Qt::red); painter.drawEllipse(pin->localPos(), Pin::HoverRadius, Pin::HoverRadius); }
    painter.restore();
}

QJsonObject VoltageProbe::serialize() const { auto o = Component::serialize(); o["voltage"] = m_voltage; return o; }
void VoltageProbe::deserialize(const QJsonObject& obj) { Component::deserialize(obj); m_voltage = obj["voltage"].toDouble(0.0); }
QMap<QString,QString> VoltageProbe::properties() const { return {{"label",m_label},{"voltage",QString::number(m_voltage)}}; }
void VoltageProbe::setProperty(const QString& key, const QString& value) { if(key=="label") m_label=value; if(key=="voltage") m_voltage=value.toDouble(); }
