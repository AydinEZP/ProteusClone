#include "ClockGenerator.h"
#include <QPainter>

ClockGenerator::ClockGenerator()
    : Component("ClockGenerator")
{
    setLabel("CLK?");
    addPin(std::make_shared<Pin>("OUT", PinType::Output, QPointF(30, 0)));
    updatePinWorldPositions();
}

void ClockGenerator::tick(double dt)
{
    if (m_frequency <= 0) return;
    m_accumulator += dt;
    double period = 1.0 / m_frequency;
    if (m_accumulator >= period / 2.0) {
        m_output      = !m_output;
        m_accumulator = 0.0;
    }
}

QRectF ClockGenerator::boundingBox() const
{
    return QRectF(m_pos.x()-28, m_pos.y()-18, 60, 36);
}

void ClockGenerator::draw(QPainter& painter, bool selected) const
{
    painter.save();
    painter.translate(m_pos);
    painter.rotate(m_rotation);
    if (m_mirrorH) painter.scale(-1, 1);
    if (m_mirrorV) painter.scale(1, -1);

    QPen pen(selected ? Qt::cyan : Qt::darkCyan, 2);
    painter.setPen(pen);
    painter.setBrush(QColor(240,255,240));
    painter.drawRoundedRect(QRectF(-28,-16,56,32), 4, 4);

    // Clock symbol - square wave
    painter.setPen(QPen(selected ? Qt::cyan : Qt::darkGreen, 2));
    QPointF wave[] = {{-14,-4},{-14,4},{-6,4},{-6,-4},{2,-4},{2,4},{10,4},{10,-4}};
    painter.drawPolyline(wave, 8);

    // Output lead
    painter.setPen(QPen(selected ? Qt::cyan : Qt::darkCyan, 2));
    painter.drawLine(QPointF(28,0), QPointF(30,0));

    painter.setPen(selected ? Qt::cyan : Qt::black);
    painter.setFont(QFont("Monospace", 7));
    painter.drawText(QRectF(-28,-32,60,14), Qt::AlignCenter,
                     QString("%1 %2Hz").arg(m_label).arg(m_frequency));

    painter.setPen(QPen(Qt::red, 1));
    for (auto& pin : m_pins) {
        painter.setBrush(pin->highlighted() ? Qt::yellow : Qt::red);
        painter.drawEllipse(pin->localPos(), Pin::HoverRadius, Pin::HoverRadius);
    }
    painter.restore();
}

QJsonObject ClockGenerator::serialize() const
{
    QJsonObject obj = Component::serialize();
    obj["frequency"] = m_frequency;
    return obj;
}

void ClockGenerator::deserialize(const QJsonObject& obj)
{
    Component::deserialize(obj);
    m_frequency = obj["frequency"].toDouble(1.0);
}

QMap<QString,QString> ClockGenerator::properties() const
{
    QMap<QString,QString> p;
    p["label"]     = m_label;
    p["frequency"] = QString::number(m_frequency);
    return p;
}

void ClockGenerator::setProperty(const QString& key, const QString& value)
{
    if (key == "label")     m_label     = value;
    if (key == "frequency") m_frequency = value.toDouble();
}