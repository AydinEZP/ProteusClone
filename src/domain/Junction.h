#pragma once
#include <QPointF>
#include <QJsonObject>
#include <QPainter>

using JunctionID = quint64;

/**
 * A T- or X-junction where multiple wires meet at one node.
 * Rendered as a filled dot.
 */
class Junction {
public:
    Junction();
    explicit Junction(QPointF pos);

    JunctionID id()  const { return m_id; }
    QPointF    pos() const { return m_pos; }
    void       setPos(QPointF p) { m_pos = p; }

    bool selected() const { return m_selected; }
    void setSelected(bool s) { m_selected = s; }

    void draw(QPainter& painter) const;
    bool hitTest(QPointF worldPoint) const;

    QJsonObject serialize()   const;
    void        deserialize(const QJsonObject& obj);

    static constexpr double Radius = 4.0;

private:
    static JunctionID s_nextId;
    JunctionID m_id;
    QPointF    m_pos;
    bool       m_selected {false};
};