#include "Keypad.h"
#include <QPainter>
#include <QJsonObject>
#include <QtMath>

namespace {
const char* kKeys[4][4] = {
    {"1","2","3","A"},
    {"4","5","6","B"},
    {"7","8","9","C"},
    {"*","0","#","D"}
};

QRectF keyCellRect(int row, int col)
{
    return QRectF(-36 + col * 24, -32 + row * 18, 20, 15);
}
}

Keypad::Keypad() : Component("Keypad")
{
    setLabel("KPD?");
    for (int i = 0; i < 4; ++i)
        addPin(std::make_shared<Pin>(QString("R%1").arg(i + 1), PinType::Input,
                                     QPointF(-50, -30 + i * 20)));
    for (int i = 0; i < 4; ++i)
        addPin(std::make_shared<Pin>(QString("C%1").arg(i + 1), PinType::Output,
                                     QPointF(50, -30 + i * 20)));
    updatePinWorldPositions();
}

void Keypad::setPressedKey(const QString& key)
{
    const QString k = key.trimmed().toUpper();
    m_pressedRow = m_pressedCol = -1;
    if (k.isEmpty() || k == "NONE") return;

    for (int r = 0; r < 4; ++r) {
        for (int c = 0; c < 4; ++c) {
            if (k == kKeys[r][c]) {
                m_pressedRow = r;
                m_pressedCol = c;
                return;
            }
        }
    }
}

QString Keypad::pressedKey() const
{
    if (m_pressedRow < 0 || m_pressedCol < 0) return "none";
    return kKeys[m_pressedRow][m_pressedCol];
}

void Keypad::releaseKey()
{
    m_pressedRow = -1;
    m_pressedCol = -1;
}

QString Keypad::keyAtWorldPos(QPointF worldPos) const
{
    // The visual keypad body is translated and rotated by draw(). Convert the
    // mouse point back into the same local coordinate system.
    const QPointF d = worldPos - m_pos;
    const double rad = qDegreesToRadians(static_cast<double>(m_rotation));
    const double cs = qCos(rad);
    const double sn = qSin(rad);
    const QPointF local(d.x() * cs + d.y() * sn,
                        -d.x() * sn + d.y() * cs);

    for (int r = 0; r < 4; ++r)
        for (int c = 0; c < 4; ++c)
            if (keyCellRect(r, c).contains(local))
                return kKeys[r][c];

    return {};
}

bool Keypad::pressAtWorldPos(QPointF worldPos)
{
    const QString key = keyAtWorldPos(worldPos);
    if (key.isEmpty()) return false;
    setPressedKey(key);
    return true;
}

bool Keypad::columnActive(int col, const QVector<bool>& rowDriveLow) const
{
    if (col != m_pressedCol || m_pressedRow < 0) return false;
    if (m_pressedRow >= rowDriveLow.size()) return false;
    return rowDriveLow[m_pressedRow];
}

QRectF Keypad::boundingBox() const
{
    return QRectF(m_pos.x() - 54, m_pos.y() - 44, 108, 88);
}

void Keypad::draw(QPainter& painter, bool selected) const
{
    painter.save();
    painter.translate(m_pos);
    painter.rotate(m_rotation);
    painter.setPen(QPen(selected ? Qt::cyan : Qt::darkGray, 2));
    painter.setBrush(QColor(50, 50, 50));
    painter.drawRoundedRect(QRectF(-50, -42, 100, 84), 4, 4);
    painter.setFont(QFont("Arial", 8, QFont::Bold));

    for (int r = 0; r < 4; ++r) {
        for (int c = 0; c < 4; ++c) {
            const QRectF cell = keyCellRect(r, c);
            const bool down = (r == m_pressedRow && c == m_pressedCol);
            painter.setBrush(down ? QColor(130, 180, 255) : QColor(230, 230, 230));
            painter.setPen(Qt::black);
            painter.drawRoundedRect(cell, 3, 3);
            painter.drawText(cell, Qt::AlignCenter, kKeys[r][c]);
        }
    }

    drawPinLabels(painter, QRectF(-50, -42, 100, 84), Qt::white, 5);
    painter.setPen(QPen(Qt::red, 1));
    for (const auto& pin : m_pins) {
        painter.setBrush(pin->highlighted() ? Qt::yellow : Qt::red);
        painter.drawEllipse(pin->localPos(), Pin::HoverRadius, Pin::HoverRadius);
    }
    painter.restore();
}

QJsonObject Keypad::serialize() const
{
    auto o = Component::serialize();
    // Pressed state is transient, but keeping it in the file is harmless and
    // backward-compatible with existing .pcj projects.
    o["pressedKey"] = pressedKey();
    return o;
}

void Keypad::deserialize(const QJsonObject& obj)
{
    Component::deserialize(obj);
    setPressedKey(obj["pressedKey"].toString("none"));
}

QMap<QString,QString> Keypad::properties() const
{
    return {{"label", m_label}, {"pressedKey", pressedKey()}};
}

void Keypad::setProperty(const QString& key, const QString& value)
{
    if (key == "label") m_label = value;
    else if (key == "pressedKey") setPressedKey(value);
}
