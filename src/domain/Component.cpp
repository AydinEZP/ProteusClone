#include "Component.h"
#include <QtMath>
#include <QJsonArray>

ComponentID Component::s_nextId = 1;

Component::Component(const QString& type)
    : m_id(s_nextId++), m_type(type), m_label(type)
{}

void Component::addPin(std::shared_ptr<Pin> pin)
{
    m_pins.push_back(pin);
}

// ── Geometry ──────────────────────────────────────────────────────────────────

void Component::moveTo(QPointF newPos)
{
    m_pos = newPos;
    updatePinWorldPositions();
}

void Component::rotate90()
{
    m_rotation = (m_rotation + 90) % 360;
    updatePinWorldPositions();
}

void Component::mirrorHorizontal()
{
    m_mirrorH = !m_mirrorH;
    updatePinWorldPositions();
}

void Component::mirrorVertical()
{
    m_mirrorV = !m_mirrorV;
    updatePinWorldPositions();
}

QPointF Component::transformPoint(QPointF local) const
{
    // Apply mirror
    double x = m_mirrorH ? -local.x() : local.x();
    double y = m_mirrorV ? -local.y() : local.y();

    // Apply rotation (counter-clockwise positive, Qt Y-axis points down)
    double rad = qDegreesToRadians(static_cast<double>(m_rotation));
    double cosA = qCos(rad);
    double sinA = qSin(rad);
    double rx = x * cosA - y * sinA;
    double ry = x * sinA + y * cosA;

    return QPointF(m_pos.x() + rx, m_pos.y() + ry);
}

void Component::updatePinWorldPositions()
{
    for (auto& pin : m_pins) {
        pin->setWorldPos(transformPoint(pin->localPos()));
    }
}

std::shared_ptr<Pin> Component::pinByName(const QString& name) const
{
    for (auto& p : m_pins) {
        if (p->name() == name) return p;
    }
    return nullptr;
}

void Component::drawPinLabels(QPainter& painter,
                              const QRectF& bodyRect,
                              const QColor& textColor,
                              int pointSize) const
{
    painter.save();
    painter.setPen(textColor);
    QFont font("Monospace", pointSize);
    font.setBold(true);
    painter.setFont(font);

    constexpr double h = 9.0;
    const double half = bodyRect.width() * 0.5;

    for (const auto& pin : m_pins) {
        if (!pin) continue;
        const QPointF pt = pin->localPos();
        QRectF textRect;
        int flags = Qt::AlignVCenter;

        if (pt.x() <= bodyRect.left() + 1.0) {
            textRect = QRectF(bodyRect.left() + 3.0, pt.y() - h * 0.5,
                              half - 6.0, h);
            flags |= Qt::AlignLeft;
        } else if (pt.x() >= bodyRect.right() - 1.0) {
            textRect = QRectF(bodyRect.center().x() + 3.0, pt.y() - h * 0.5,
                              half - 6.0, h);
            flags |= Qt::AlignRight;
        } else if (pt.y() <= bodyRect.top() + 1.0) {
            textRect = QRectF(pt.x() - 20.0, bodyRect.top() + 2.0, 40.0, h);
            flags |= Qt::AlignHCenter;
        } else if (pt.y() >= bodyRect.bottom() - 1.0) {
            textRect = QRectF(pt.x() - 20.0, bodyRect.bottom() - h - 2.0, 40.0, h);
            flags |= Qt::AlignHCenter;
        } else {
            // Fallback: place the label on the nearer horizontal side.
            if (pt.x() < bodyRect.center().x()) {
                textRect = QRectF(bodyRect.left() + 3.0, pt.y() - h * 0.5,
                                  half - 6.0, h);
                flags |= Qt::AlignLeft;
            } else {
                textRect = QRectF(bodyRect.center().x() + 3.0, pt.y() - h * 0.5,
                                  half - 6.0, h);
                flags |= Qt::AlignRight;
            }
        }

        painter.drawText(textRect, flags, pin->name());
    }
    painter.restore();
}

// ── Hit test ─────────────────────────────────────────────────────────────────

bool Component::hitTest(QPointF worldPoint) const
{
    return boundingBox().contains(worldPoint);
}

// ── Serialization ─────────────────────────────────────────────────────────────

QJsonObject Component::serialize() const
{
    QJsonObject obj;
    obj["id"]       = static_cast<qint64>(m_id);
    obj["type"]     = m_type;
    obj["label"]    = m_label;
    obj["x"]        = m_pos.x();
    obj["y"]        = m_pos.y();
    obj["rotation"] = m_rotation;
    obj["mirrorH"]  = m_mirrorH;
    obj["mirrorV"]  = m_mirrorV;
    return obj;
}

void Component::deserialize(const QJsonObject& obj)
{
    m_id       = static_cast<ComponentID>(obj["id"].toInteger());
    m_label    = obj["label"].toString(m_type);
    m_pos      = QPointF(obj["x"].toDouble(), obj["y"].toDouble());
    m_rotation = obj["rotation"].toInt(0);
    m_mirrorH  = obj["mirrorH"].toBool(false);
    m_mirrorV  = obj["mirrorV"].toBool(false);
    if (m_id >= s_nextId) s_nextId = m_id + 1;
    updatePinWorldPositions();
}