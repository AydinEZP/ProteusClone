#pragma once
#include <QString>
#include <QPointF>

enum class PinType {
    Input,
    Output,
    Bidirectional,
    Passive,
    Power,
    Ground
};

class Pin {
public:
    Pin(const QString& name, PinType type, QPointF localPos);

    QString     name()      const { return m_name; }
    PinType     type()      const { return m_type; }
    QPointF     localPos()  const { return m_localPos; }
    QPointF     worldPos()  const { return m_worldPos; }
    bool        highlighted() const { return m_highlighted; }

    void setWorldPos(QPointF p)    { m_worldPos = p; }
    void setHighlighted(bool h)    { m_highlighted = h; }

    // Radius used for hover-detection hit testing
    static constexpr double HoverRadius = 6.0;

private:
    QString  m_name;
    PinType  m_type;
    QPointF  m_localPos;   // position relative to component origin, unrotated
    QPointF  m_worldPos;   // computed by component after each transform update
    bool     m_highlighted = false;
};