#include "Wire.h"
#include "Pin.h"
#include <QLineF>
#include <QJsonArray>
#include <algorithm>
#include <cmath>
#include <utility>

WireID Wire::s_nextId = 1;

Wire::Wire()
    : m_id(s_nextId++)
{}

Wire::Wire(std::shared_ptr<Pin> startPin, std::shared_ptr<Pin> endPin)
    : m_id(s_nextId++), m_startPin(std::move(startPin)), m_endPin(std::move(endPin))
{
    routeSimple();
}

void Wire::setPins(std::shared_ptr<Pin> startPin, std::shared_ptr<Pin> endPin)
{
    m_startPin = std::move(startPin);
    m_endPin   = std::move(endPin);
}

void Wire::appendIfDistinct(std::vector<QPointF>& out, QPointF p)
{
    if (out.empty() || QLineF(out.back(), p).length() > 0.5)
        out.push_back(p);
}

void Wire::appendOrthogonal(std::vector<QPointF>& out, QPointF target)
{
    if (out.empty()) {
        out.push_back(target);
        return;
    }

    const QPointF last = out.back();
    const bool sameX = std::abs(last.x() - target.x()) < 0.5;
    const bool sameY = std::abs(last.y() - target.y()) < 0.5;

    if (sameX || sameY) {
        appendIfDistinct(out, target);
        return;
    }

    // Default routing policy: horizontal first, then vertical.
    // This gives a clear multi-bend 90-degree polyline.
    appendIfDistinct(out, QPointF(target.x(), last.y()));
    appendIfDistinct(out, target);
}

void Wire::removeDuplicateAndTinySegments(std::vector<QPointF>& pts)
{
    std::vector<QPointF> cleaned;
    for (const auto& p : pts)
        appendIfDistinct(cleaned, p);

    // Remove collinear middle points that are unnecessary.
    bool changed = true;
    while (changed && cleaned.size() >= 3) {
        changed = false;
        for (size_t i = 1; i + 1 < cleaned.size(); ++i) {
            const QPointF a = cleaned[i - 1];
            const QPointF b = cleaned[i];
            const QPointF c = cleaned[i + 1];
            const bool sameX = std::abs(a.x() - b.x()) < 0.5 && std::abs(b.x() - c.x()) < 0.5;
            const bool sameY = std::abs(a.y() - b.y()) < 0.5 && std::abs(b.y() - c.y()) < 0.5;
            if (sameX || sameY) {
                cleaned.erase(cleaned.begin() + static_cast<long>(i));
                changed = true;
                break;
            }
        }
    }
    pts.swap(cleaned);
}

std::vector<QPointF> Wire::makeOrthogonalPath(QPointF start,
                                              const std::vector<QPointF>& waypoints,
                                              QPointF end)
{
    std::vector<QPointF> out;
    out.push_back(start);
    for (const auto& wp : waypoints)
        appendOrthogonal(out, wp);
    appendOrthogonal(out, end);
    removeDuplicateAndTinySegments(out);
    return out;
}

void Wire::setPath(const std::vector<QPointF>& path)
{
    m_path = path;
    removeDuplicateAndTinySegments(m_path);
    // More than a simple L/direct route means the user deliberately shaped it.
    m_manualRoute = (m_path.size() > 3);
}

void Wire::routeSimple()
{
    m_path.clear();
    m_manualRoute = false;
    if (!m_startPin || !m_endPin) return;

    const QPointF A = m_startPin->worldPos();
    const QPointF B = m_endPin->worldPos();
    if (QLineF(A, B).length() < 0.5) return;

    m_path = makeOrthogonalPath(A, {}, B);
}

void Wire::reroutePreservingWaypoints()
{
    if (!m_startPin || !m_endPin) return;

    if (!m_manualRoute) {
        routeSimple();
        return;
    }

    // Keep existing internal bends; only refresh endpoint positions.
    std::vector<QPointF> waypoints;
    if (m_path.size() >= 3) {
        for (size_t i = 1; i + 1 < m_path.size(); ++i)
            waypoints.push_back(m_path[i]);
    }

    m_path = makeOrthogonalPath(m_startPin->worldPos(), waypoints, m_endPin->worldPos());
    m_manualRoute = true;
}

bool Wire::hitTest(QPointF worldPoint, double tolerance) const
{
    if (m_path.size() < 2) return false;
    for (size_t i = 1; i < m_path.size(); ++i) {
        QLineF seg(m_path[i-1], m_path[i]);
        const double len = seg.length();
        if (len < 1e-9) continue;
        const QPointF d = seg.p2() - seg.p1();
        double t = QPointF::dotProduct(worldPoint - seg.p1(), d) / (len * len);
        t = std::clamp(t, 0.0, 1.0);
        const QPointF closest = seg.p1() + t * d;
        const double dist = QLineF(worldPoint, closest).length();
        if (dist <= tolerance) return true;
    }
    return false;
}

QJsonObject Wire::serialize() const
{
    QJsonObject obj;
    obj["id"] = static_cast<qint64>(m_id);
    obj["manualRoute"] = m_manualRoute;
    QJsonArray pathArr;
    for (auto& pt : m_path) {
        QJsonObject ptObj;
        ptObj["x"] = pt.x();
        ptObj["y"] = pt.y();
        pathArr.append(ptObj);
    }
    obj["path"] = pathArr;
    return obj;
}

void Wire::deserialize(const QJsonObject& obj)
{
    m_id = static_cast<WireID>(obj["id"].toInteger());
    if (m_id >= s_nextId) s_nextId = m_id + 1;
    m_path.clear();
    for (auto v : obj["path"].toArray()) {
        auto ptObj = v.toObject();
        m_path.push_back(QPointF(ptObj["x"].toDouble(), ptObj["y"].toDouble()));
    }
    removeDuplicateAndTinySegments(m_path);
    m_manualRoute = obj["manualRoute"].toBool(m_path.size() > 3);
}
