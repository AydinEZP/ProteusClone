#include "Junction.h"
#include <QLineF>

JunctionID Junction::s_nextId = 1;

Junction::Junction()
    : m_id(s_nextId++)
{}

Junction::Junction(QPointF pos)
    : m_id(s_nextId++), m_pos(pos)
{}

void Junction::draw(QPainter& painter) const
{
    painter.save();
    QColor c = m_selected ? Qt::cyan : Qt::darkGreen;
    painter.setBrush(c);
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(m_pos, Radius, Radius);
    painter.restore();
}

bool Junction::hitTest(QPointF worldPoint) const
{
    return QLineF(m_pos, worldPoint).length() <= Radius * 1.5;
}

QJsonObject Junction::serialize() const
{
    QJsonObject obj;
    obj["id"] = static_cast<qint64>(m_id);
    obj["x"]  = m_pos.x();
    obj["y"]  = m_pos.y();
    return obj;
}

void Junction::deserialize(const QJsonObject& obj)
{
    m_id  = static_cast<JunctionID>(obj["id"].toInteger());
    if (m_id >= s_nextId) s_nextId = m_id + 1;
    m_pos = QPointF(obj["x"].toDouble(), obj["y"].toDouble());
}