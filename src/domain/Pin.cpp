#include "Pin.h"

Pin::Pin(const QString& name, PinType type, QPointF localPos)
    : m_name(name), m_type(type), m_localPos(localPos), m_worldPos(localPos)
{}