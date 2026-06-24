#include "SimpleDAC.h"
#include <QPainter>
#include <QJsonObject>
#include <algorithm>
#include <QtGlobal>

namespace {
double dacBodyHalfHeight(int bits)
{
    // 12 px per digital input plus comfortable top/bottom margins.
    return std::max(48.0, (std::max(1, bits) - 1) * 6.0 + 16.0);
}
}

SimpleDAC::SimpleDAC()
    : Component("SimpleDAC")
{
    setLabel("DAC?");
    rebuildPins();
}

void SimpleDAC::setBits(int bits)
{
    m_bits = std::clamp(bits, 1, 16);
    rebuildPins();
}

void SimpleDAC::rebuildPins()
{
    m_pins.clear();
    const double halfH = dacBodyHalfHeight(m_bits);
    const double spacing = 12.0;
    const double start = -((m_bits - 1) * spacing) / 2.0;

    for (int i = 0; i < m_bits; ++i) {
        addPin(std::make_shared<Pin>(QString("D%1").arg(i), PinType::Input,
                                     QPointF(-72, start + i * spacing)));
    }

    // Put references on the bottom edge; they no longer collide with D7/D8/... labels.
    addPin(std::make_shared<Pin>("VREF+", PinType::Input, QPointF(-32, halfH + 12)));
    addPin(std::make_shared<Pin>("VREF-", PinType::Input, QPointF( 32, halfH + 12)));
    addPin(std::make_shared<Pin>("VOUT",  PinType::Output, QPointF(72, 0)));
    updatePinWorldPositions();
}

double SimpleDAC::idealOutputFromCurrentInput() const
{
    const uint32_t maxCode = (m_bits >= 32) ? 0xffffffffu : ((1u << m_bits) - 1u);
    if (maxCode == 0) return m_vrefMinus;
    const double ratio = std::clamp(double(m_input) / double(maxCode), 0.0, 1.0);
    return m_vrefMinus + ratio * (m_vrefPlus - m_vrefMinus);
}

void SimpleDAC::tickConversion(double dtSeconds)
{
    const double target = idealOutputFromCurrentInput();
    if (qAbs(target - m_outputVoltage) < 1e-9) {
        m_pending = false;
        m_pendingElapsedMs = 0.0;
        return;
    }
    if (m_conversionDelayMs <= 0.0) {
        m_outputVoltage = target;
        m_pending = false;
        return;
    }
    if (!m_pending || qAbs(m_pendingVoltage - target) > 1e-9) {
        m_pending = true;
        m_pendingVoltage = target;
        m_pendingElapsedMs = 0.0;
    }
    m_pendingElapsedMs += qMax(0.0, dtSeconds) * 1000.0;
    if (m_pendingElapsedMs >= m_conversionDelayMs) {
        m_outputVoltage = m_pendingVoltage;
        m_pending = false;
        m_pendingElapsedMs = 0.0;
    }
}

QRectF SimpleDAC::boundingBox() const
{
    const double halfH = dacBodyHalfHeight(m_bits);
    return QRectF(m_pos.x()-78, m_pos.y()-halfH-4,
                  156, 2.0*halfH + 22.0);
}

void SimpleDAC::draw(QPainter& painter, bool selected) const
{
    painter.save();
    painter.translate(m_pos);
    painter.rotate(m_rotation);
    if (m_mirrorH) painter.scale(-1, 1);
    if (m_mirrorV) painter.scale(1, -1);

    const double halfH = dacBodyHalfHeight(m_bits);
    const QRectF body(-64, -halfH, 128, 2.0*halfH);

    painter.setPen(QPen(selected ? Qt::cyan : Qt::darkGray, 2));
    painter.setBrush(QColor(200,240,200));
    painter.drawRect(body);

    painter.setFont(QFont("Monospace",8,QFont::Bold));
    painter.setPen(Qt::darkGray);
    painter.drawText(QRectF(-22,-24,44,48), Qt::AlignCenter,
                     QString("DAC\n%1-bit\n%2V").arg(m_bits).arg(m_outputVoltage,0,'f',2));

    // Digital input labels are aligned one-to-one with their pins.
    painter.setFont(QFont("Monospace", 6, QFont::Bold));
    for (const auto& pin : m_pins) {
        if (!pin) continue;
        const QPointF pt = pin->localPos();
        if (pin->name().startsWith("D")) {
            painter.drawText(QRectF(-61, pt.y()-5, 34, 10), Qt::AlignLeft|Qt::AlignVCenter, pin->name());
        } else if (pin->name() == "VOUT") {
            painter.drawText(QRectF(27,-5,34,10), Qt::AlignRight|Qt::AlignVCenter, "VOUT");
        }
    }

    // Bottom reference labels use separate non-overlapping rectangles.
    painter.drawText(QRectF(-61, halfH-12, 50, 10), Qt::AlignLeft|Qt::AlignVCenter, "VREF+");
    painter.drawText(QRectF(11, halfH-12, 50, 10), Qt::AlignRight|Qt::AlignVCenter, "VREF-");

    painter.setPen(QPen(Qt::red,1));
    for (const auto& pin : m_pins) {
        painter.setBrush(pin->highlighted() ? Qt::yellow : Qt::red);
        painter.drawEllipse(pin->localPos(), Pin::HoverRadius, Pin::HoverRadius);
    }
    painter.restore();
}

QJsonObject SimpleDAC::serialize() const
{
    auto o = Component::serialize();
    o["input"] = static_cast<qint64>(m_input);
    o["vrefMinus"] = m_vrefMinus;
    o["vrefPlus"] = m_vrefPlus;
    o["bits"] = m_bits;
    o["conversionDelayMs"] = m_conversionDelayMs;
    o["outputVoltage"] = m_outputVoltage;
    return o;
}

void SimpleDAC::deserialize(const QJsonObject& obj)
{
    Component::deserialize(obj);
    m_input = static_cast<uint32_t>(obj["input"].toInteger());
    m_vrefMinus = obj["vrefMinus"].toDouble(0.0);
    m_vrefPlus = obj["vrefPlus"].toDouble(5.0);
    m_bits = std::clamp(obj["bits"].toInt(8), 1, 16);
    m_conversionDelayMs = obj["conversionDelayMs"].toDouble(1.0);
    m_outputVoltage = obj["outputVoltage"].toDouble(idealOutputFromCurrentInput());
    rebuildPins();
    updatePinWorldPositions();
}

QMap<QString,QString> SimpleDAC::properties() const
{
    return {{"label",m_label},
            {"input",QString::number(m_input)},
            {"vrefMinus",QString::number(m_vrefMinus)},
            {"vrefPlus",QString::number(m_vrefPlus)},
            {"bits",QString::number(m_bits)},
            {"conversionDelayMs",QString::number(m_conversionDelayMs)},
            {"outputVoltage",QString::number(m_outputVoltage)}};
}

void SimpleDAC::setProperty(const QString& key, const QString& value)
{
    if (key=="label") m_label=value;
    else if (key=="input") m_input=value.toUInt();
    else if (key=="vrefMinus") m_vrefMinus=value.toDouble();
    else if (key=="vrefPlus") m_vrefPlus=value.toDouble();
    else if (key=="bits") setBits(value.toInt());
    else if (key=="conversionDelayMs") m_conversionDelayMs=qMax(0.0,value.toDouble());
}
