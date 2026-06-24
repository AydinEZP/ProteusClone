#include "SimpleADC.h"
#include <QPainter>
#include <QJsonObject>
#include <algorithm>
#include <cmath>
#include <QtGlobal>

namespace {
double adcBodyHalfHeight(int bits)
{
    // 12 px per digital output plus comfortable top/bottom margins.
    return std::max(48.0, (std::max(1, bits) - 1) * 6.0 + 16.0);
}
}

SimpleADC::SimpleADC()
    : Component("SimpleADC")
{
    setLabel("ADC?");
    rebuildPins();
}

void SimpleADC::setBits(int bits)
{
    m_bits = std::clamp(bits, 1, 16);
    rebuildPins();
    m_outputCode &= ((1u << m_bits) - 1u);
}

void SimpleADC::rebuildPins()
{
    m_pins.clear();
    const double halfH = adcBodyHalfHeight(m_bits);
    const double spacing = 12.0;
    const double start = -((m_bits - 1) * spacing) / 2.0;

    // The project specification requires VIN and the two references. There is
    // no clock input in this ideal delayed-conversion model; conversion delay is
    // handled by simulation time, so an unused CLK pin must not create a false
    // floating-input DRC error.
    addPin(std::make_shared<Pin>("VIN",   PinType::Input, QPointF(-72, -20)));
    addPin(std::make_shared<Pin>("VREF+", PinType::Input, QPointF(-72,   0)));
    addPin(std::make_shared<Pin>("VREF-", PinType::Input, QPointF(-72,  20)));

    for (int i = 0; i < m_bits; ++i) {
        addPin(std::make_shared<Pin>(QString("D%1").arg(i), PinType::Output,
                                     QPointF(72, start + i * spacing)));
    }
    updatePinWorldPositions();
}

uint32_t SimpleADC::idealCodeFromCurrentInput() const
{
    if (m_vrefPlus <= m_vrefMinus) return 0;
    double ratio = (m_inputVoltage - m_vrefMinus) / (m_vrefPlus - m_vrefMinus);
    ratio = std::clamp(ratio, 0.0, 1.0);
    const uint32_t maxCode = (1u << m_bits) - 1u;
    return static_cast<uint32_t>(std::round(ratio * maxCode));
}

void SimpleADC::tickConversion(double dtSeconds)
{
    const uint32_t target = idealCodeFromCurrentInput();
    if (target == m_outputCode) {
        m_pending = false;
        m_pendingElapsedMs = 0.0;
        return;
    }
    if (m_conversionDelayMs <= 0.0) {
        m_outputCode = target;
        m_pending = false;
        return;
    }
    if (!m_pending || m_pendingCode != target) {
        m_pending = true;
        m_pendingCode = target;
        m_pendingElapsedMs = 0.0;
    }
    m_pendingElapsedMs += qMax(0.0, dtSeconds) * 1000.0;
    if (m_pendingElapsedMs >= m_conversionDelayMs) {
        m_outputCode = m_pendingCode;
        m_pending = false;
        m_pendingElapsedMs = 0.0;
    }
}

QRectF SimpleADC::boundingBox() const
{
    const double halfH = adcBodyHalfHeight(m_bits);
    return QRectF(m_pos.x()-78, m_pos.y()-halfH-4,
                  156, 2.0*halfH + 8.0);
}

void SimpleADC::draw(QPainter& painter, bool selected) const
{
    painter.save();
    painter.translate(m_pos);
    painter.rotate(m_rotation);
    if (m_mirrorH) painter.scale(-1, 1);
    if (m_mirrorV) painter.scale(1, -1);

    const double halfH = adcBodyHalfHeight(m_bits);
    const QRectF body(-64, -halfH, 128, 2.0*halfH);

    painter.setPen(QPen(selected ? Qt::cyan : Qt::darkGray, 2));
    painter.setBrush(QColor(240,240,200));
    painter.drawRect(body);

    painter.setFont(QFont("Monospace", 8, QFont::Bold));
    painter.setPen(Qt::darkGray);
    painter.drawText(QRectF(-22,-24,44,48), Qt::AlignCenter,
                     QString("ADC\n%1-bit\n%2").arg(m_bits).arg(m_outputCode));

    painter.setFont(QFont("Monospace", 6, QFont::Bold));
    for (const auto& pin : m_pins) {
        if (!pin) continue;
        const QPointF pt = pin->localPos();
        if (pin->name().startsWith("D")) {
            painter.drawText(QRectF(28, pt.y()-5, 33, 10),
                             Qt::AlignRight|Qt::AlignVCenter, pin->name());
        } else {
            painter.drawText(QRectF(-61, pt.y()-5, 40, 10),
                             Qt::AlignLeft|Qt::AlignVCenter, pin->name());
        }
    }

    painter.setPen(QPen(Qt::red, 1));
    for (const auto& pin : m_pins) {
        painter.setBrush(pin->highlighted() ? Qt::yellow : Qt::red);
        painter.drawEllipse(pin->localPos(), Pin::HoverRadius, Pin::HoverRadius);
    }
    painter.restore();
}

QJsonObject SimpleADC::serialize() const
{
    auto o = Component::serialize();
    o["inputVoltage"] = m_inputVoltage;
    o["vrefMinus"] = m_vrefMinus;
    o["vrefPlus"] = m_vrefPlus;
    o["bits"] = m_bits;
    o["conversionDelayMs"] = m_conversionDelayMs;
    o["outputCode"] = static_cast<qint64>(m_outputCode);
    return o;
}

void SimpleADC::deserialize(const QJsonObject& obj)
{
    Component::deserialize(obj);
    m_inputVoltage = obj["inputVoltage"].toDouble(0.0);
    m_vrefMinus = obj["vrefMinus"].toDouble(0.0);
    m_vrefPlus = obj["vrefPlus"].toDouble(5.0);
    m_bits = std::clamp(obj["bits"].toInt(8), 1, 16);
    m_conversionDelayMs = qMax(0.0, obj["conversionDelayMs"].toDouble(1.0));
    m_outputCode = static_cast<uint32_t>(obj["outputCode"].toInteger());
    rebuildPins();
    updatePinWorldPositions();
}

QMap<QString,QString> SimpleADC::properties() const
{
    return {{"label",m_label},
            {"inputVoltage",QString::number(m_inputVoltage)},
            {"vrefMinus",QString::number(m_vrefMinus)},
            {"vrefPlus",QString::number(m_vrefPlus)},
            {"bits",QString::number(m_bits)},
            {"conversionDelayMs",QString::number(m_conversionDelayMs)},
            {"outputCode",QString::number(m_outputCode)}};
}

void SimpleADC::setProperty(const QString& key,const QString& value)
{
    if (key=="label") m_label=value;
    else if (key=="inputVoltage") m_inputVoltage=value.toDouble();
    else if (key=="vrefMinus") m_vrefMinus=value.toDouble();
    else if (key=="vrefPlus") m_vrefPlus=value.toDouble();
    else if (key=="bits") setBits(value.toInt());
    else if (key=="conversionDelayMs") m_conversionDelayMs=qMax(0.0,value.toDouble());
}
