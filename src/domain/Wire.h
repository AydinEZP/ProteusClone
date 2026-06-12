#pragma once
#include <QPointF>
#include <QJsonObject>
#include <vector>
#include <memory>

class Pin;

using WireID = quint64;

/**
 * Represents a routed electrical connection between two pins.
 * The path consists only of horizontal/vertical segments.
 *
 * A wire may either be auto-routed (single L shape) or manually routed through
 * user-supplied intermediate waypoints. Manual routes are preserved when a
 * connected component moves: only the endpoints are refreshed.
 */
class Wire {
public:
    Wire();
    Wire(std::shared_ptr<Pin> startPin, std::shared_ptr<Pin> endPin);

    WireID id() const { return m_id; }

    const std::vector<QPointF>& path() const { return m_path; }
    void setPath(const std::vector<QPointF>& path);

    std::shared_ptr<Pin> startPin() const { return m_startPin; }
    std::shared_ptr<Pin> endPin()   const { return m_endPin; }

    void setPins(std::shared_ptr<Pin> startPin, std::shared_ptr<Pin> endPin);

    bool selected() const { return m_selected; }
    void setSelected(bool s) { m_selected = s; }

    bool manualRoute() const { return m_manualRoute; }
    void setManualRoute(bool manual) { m_manualRoute = manual; }

    /** Rebuild a simple L-shaped path between the two pin world positions. */
    void routeSimple();

    /** Refresh wire after component movement. Manual routes keep their bends. */
    void reroutePreservingWaypoints();

    /** Make an orthogonal polyline from start -> waypoints -> end. */
    static std::vector<QPointF> makeOrthogonalPath(QPointF start,
                                                   const std::vector<QPointF>& waypoints,
                                                   QPointF end);

    /** Returns true if worldPoint is within tolerance of any wire segment. */
    bool hitTest(QPointF worldPoint, double tolerance = 5.0) const;

    QJsonObject serialize()   const;
    void        deserialize(const QJsonObject& obj);

private:
    static void appendOrthogonal(std::vector<QPointF>& out, QPointF target);
    static void appendIfDistinct(std::vector<QPointF>& out, QPointF p);
    static void removeDuplicateAndTinySegments(std::vector<QPointF>& pts);

    static WireID s_nextId;
    WireID m_id;
    std::vector<QPointF> m_path;
    std::shared_ptr<Pin> m_startPin;
    std::shared_ptr<Pin> m_endPin;
    bool m_selected {false};
    bool m_manualRoute {false};
};
