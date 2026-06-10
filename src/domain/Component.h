#pragma once
#include <QtGlobal>
#include <QMap>
#include <QString>
#include <QPointF>
#include <QRectF>
#include <QPainter>
#include <QJsonObject>
#include <QMap>
#include <vector>
#include <memory>
#include "Pin.h"

// Unique component ID counter
using ComponentID = quint64;

/**
 * Abstract base class for every circuit element.
 *
 * Responsibilities:
 *  - Store position, rotation, mirror state, label, unique ID
 *  - Own a list of Pins and keep their world positions up to date
 *  - Declare pure-virtual interface: draw, hitTest, serialize, deserialize
 *  - Provide concrete helpers: moveTo, rotate90, mirrorH, mirrorV
 */
class Component {
public:
    explicit Component(const QString& type);
    virtual ~Component() = default;

    // ── Identity ──────────────────────────────────────────────
    ComponentID   id()    const { return m_id; }
    QString       type()  const { return m_type; }
    QString       label() const { return m_label; }
    void          setLabel(const QString& l) { m_label = l; }

    // ── Geometry ──────────────────────────────────────────────
    QPointF position()          const { return m_pos; }
    int     rotation()          const { return m_rotation; }   // 0,90,180,270
    bool    mirroredH()         const { return m_mirrorH; }
    bool    mirroredV()         const { return m_mirrorV; }

    void moveTo(QPointF newPos);
    void rotate90();
    void mirrorHorizontal();
    void mirrorVertical();

    virtual QRectF boundingBox() const = 0;

    // ── Pins ──────────────────────────────────────────────────
    const std::vector<std::shared_ptr<Pin>>& pins() const { return m_pins; }
    std::shared_ptr<Pin> pinByName(const QString& name) const;

    // ── Rendering ─────────────────────────────────────────────
    /**
     * Draw the component.
     * @param painter  Active QPainter (already in world coordinates)
     * @param selected Whether to draw selection highlight
     */
    virtual void draw(QPainter& painter, bool selected) const = 0;

    /** Returns true if worldPoint is inside / on this component. */
    virtual bool hitTest(QPointF worldPoint) const;

    // ── Serialization ─────────────────────────────────────────
    virtual QJsonObject serialize()   const;
    virtual void        deserialize(const QJsonObject& obj);

    // ── Component-specific properties (override in subclasses) ─
    /** Key/value property map for the PropertiesPanel */
    virtual QMap<QString,QString> properties() const { return {}; }
    virtual void setProperty(const QString& key, const QString& value) { Q_UNUSED(key); Q_UNUSED(value); }

protected:
    void addPin(std::shared_ptr<Pin> pin);
    /** Recompute every pin's worldPos from m_pos, m_rotation, m_mirrorH, m_mirrorV */
    void updatePinWorldPositions();

    /** Apply current transform to a local point */
    QPointF transformPoint(QPointF local) const;

    /** Draw compact pin-name labels inside the component body, next to each pin. */
    void drawPinLabels(QPainter& painter,
                       const QRectF& bodyRect,
                       const QColor& textColor = Qt::black,
                       int pointSize = 6) const;

    // ── State ─────────────────────────────────────────────────
    ComponentID m_id;
    QString     m_type;
    QString     m_label;
    QPointF     m_pos       {0, 0};
    int         m_rotation  {0};    // degrees, multiples of 90
    bool        m_mirrorH   {false};
    bool        m_mirrorV   {false};

    std::vector<std::shared_ptr<Pin>> m_pins;

private:
    static ComponentID s_nextId;
};
