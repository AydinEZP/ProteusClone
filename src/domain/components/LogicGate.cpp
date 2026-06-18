#include "LogicGate.h"
#include <QPainter>
#include <QJsonObject>
#include <QtGlobal>

LogicGate::LogicGate(const QString& gateType, int numInputs)
    : Component(gateType), m_numInputs(numInputs)
{
    m_inputs.assign(numInputs, false);
    addPin(std::make_shared<Pin>("OUT", PinType::Output, QPointF(32, 0)));
    int spacing = (numInputs > 1) ? 20 : 0;
    int startY  = -(numInputs - 1) * spacing / 2;
    for (int i = 0; i < numInputs; ++i) {
        addPin(std::make_shared<Pin>(QString("IN%1").arg(i+1), PinType::Input,
                                     QPointF(-32, startY + i * spacing)));
    }
    updatePinWorldPositions();
}

void LogicGate::setBoolInput(int index, bool value)
{
    if (index >= 0 && index < (int)m_inputs.size())
        m_inputs[index] = value;
}

void LogicGate::applyEvaluatedOutput(bool value, double dtSeconds)
{
    const double delayMs = qMax(0.0, m_propagationDelayMs);
    if (delayMs <= 0.0) {
        m_output = value;
        m_outputValid = true;
        m_pendingValid = false;
        m_pendingElapsedMs = 0.0;
        return;
    }

    if (!m_outputValid || value == m_output) {
        m_pendingValid = false;
        m_pendingElapsedMs = 0.0;
        m_outputValid = true;
        return;
    }

    if (!m_pendingValid || m_pendingOutput != value) {
        m_pendingValid = true;
        m_pendingOutput = value;
        m_pendingElapsedMs = 0.0;
    }

    m_pendingElapsedMs += qMax(0.0, dtSeconds) * 1000.0;
    if (m_pendingElapsedMs >= delayMs) {
        m_output = m_pendingOutput;
        m_outputValid = true;
        m_pendingValid = false;
        m_pendingElapsedMs = 0.0;
    }
}

QRectF LogicGate::boundingBox() const
{
    return QRectF(m_pos.x()-34, m_pos.y()-24, 68, 48);
}

void LogicGate::draw(QPainter& painter, bool selected) const
{
    painter.save();
    painter.translate(m_pos);
    painter.rotate(m_rotation);
    if (m_mirrorH) painter.scale(-1, 1);
    if (m_mirrorV) painter.scale(1, -1);

    QPen pen(selected ? Qt::cyan : Qt::darkBlue, 2);
    painter.setPen(pen);
    painter.setBrush(QColor(230,230,255));
    painter.drawRoundedRect(QRectF(-28,-20,56,40), 6, 6);

    painter.setPen(QPen(selected ? Qt::cyan : Qt::darkBlue, 1));
    painter.setFont(QFont("Monospace", 10, QFont::Bold));
    painter.drawText(QRectF(-28,-20,56,40), Qt::AlignCenter, m_gateLabel);

    painter.setPen(QPen(selected ? Qt::cyan : Qt::darkBlue, 2));
    painter.drawLine(QPointF(28,0), QPointF(32,0));
    int spacing = (m_numInputs > 1) ? 20 : 0;
    int startY  = -(m_numInputs - 1) * spacing / 2;
    for (int i = 0; i < m_numInputs; ++i)
        painter.drawLine(QPointF(-32, startY + i*spacing), QPointF(-28, startY + i*spacing));

    painter.setBrush(!m_outputValid ? Qt::yellow : (m_output ? Qt::green : Qt::red));
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(QPointF(26, 0), 4, 4);

    drawPinLabels(painter, QRectF(-28,-20,56,40), Qt::darkBlue, 5);
    painter.setPen(QPen(Qt::red, 1));
    for (auto& pin : m_pins) {
        painter.setBrush(pin->highlighted() ? Qt::yellow : Qt::red);
        painter.drawEllipse(pin->localPos(), Pin::HoverRadius, Pin::HoverRadius);
    }
    painter.restore();
}

QJsonObject LogicGate::serialize() const
{
    QJsonObject obj = Component::serialize();
    obj["propagationDelayMs"] = m_propagationDelayMs;
    obj["outputValid"] = m_outputValid;
    obj["output"] = m_output;
    return obj;
}

void LogicGate::deserialize(const QJsonObject& obj)
{
    Component::deserialize(obj);
    m_propagationDelayMs = obj["propagationDelayMs"].toDouble(1.0);
    m_outputValid = obj["outputValid"].toBool(true);
    m_output = obj["output"].toBool(false);
}

QMap<QString,QString> LogicGate::properties() const
{
    QMap<QString,QString> p;
    p["label"] = m_label;
    p["propagationDelayMs"] = QString::number(m_propagationDelayMs);
    p["output"] = m_outputValid ? (m_output ? "HIGH" : "LOW") : "Undefined";
    return p;
}

void LogicGate::setProperty(const QString& key, const QString& value)
{
    if (key == "label") m_label = value;
    else if (key == "propagationDelayMs") m_propagationDelayMs = qMax(0.0, value.toDouble());
}
