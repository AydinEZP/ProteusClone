#include "SevenSegment.h"
#include <QPainter>

SevenSegment::SevenSegment()
    : Component("SevenSegment")
{
    setLabel("SEG?");

    // Four inputs on each side, with enough vertical spacing for readable labels.
    const QString leftNames[]  = {"A", "B", "C", "D"};
    const QString rightNames[] = {"E", "F", "G", "DP"};
    const double ys[] = {-24.0, -8.0, 8.0, 24.0};

    for (int i = 0; i < 4; ++i)
        addPin(std::make_shared<Pin>(leftNames[i], PinType::Input, QPointF(-48, ys[i])));
    for (int i = 0; i < 4; ++i)
        addPin(std::make_shared<Pin>(rightNames[i],
                                     rightNames[i] == "DP" ? PinType::Passive : PinType::Input,
                                     QPointF(48, ys[i])));

    // COM is a real terminal, not an implicit global ground. The user must wire it.
    addPin(std::make_shared<Pin>("COM", PinType::Input, QPointF(0, 54)));
    updatePinWorldPositions();
}

QRectF SevenSegment::boundingBox() const
{
    return QRectF(m_pos.x()-52, m_pos.y()-46, 104, 106);
}

void SevenSegment::draw(QPainter& painter, bool selected) const
{
    painter.save();
    painter.translate(m_pos);
    painter.rotate(m_rotation);
    if (m_mirrorH) painter.scale(-1, 1);
    if (m_mirrorV) painter.scale(1, -1);

    const QRectF body(-40,-42,80,88);
    painter.setBrush(QColor(20,20,20));
    painter.setPen(QPen(selected ? Qt::cyan : Qt::black, 2));
    painter.drawRoundedRect(body, 4, 4);

    auto segColor = [&](int i) -> QColor {
        return seg(i) ? QColor(255,35,35) : QColor(65,12,12);
    };

    painter.fillRect(QRectF(-20,-32,40,5), segColor(0)); // A
    painter.fillRect(QRectF(18,-29,5,25), segColor(1));  // B
    painter.fillRect(QRectF(18,4,5,25), segColor(2));    // C
    painter.fillRect(QRectF(-20,28,40,5), segColor(3));  // D
    painter.fillRect(QRectF(-23,4,5,25), segColor(4));   // E
    painter.fillRect(QRectF(-23,-29,5,25), segColor(5)); // F
    painter.fillRect(QRectF(-20,-2,40,5), segColor(6));  // G

    painter.setPen(Qt::NoPen);
    painter.setBrush(segColor(7));
    painter.drawEllipse(QPointF(30,30), 3.5, 3.5);       // DP

    painter.setFont(QFont("Monospace", 6, QFont::Bold));
    painter.setPen(QColor(225,225,225));
    for (const auto& pin : m_pins) {
        if (!pin) continue;
        const QPointF pt = pin->localPos();
        if (pt.x() < 0) {
            painter.drawText(QRectF(-37, pt.y()-5, 28, 10), Qt::AlignLeft|Qt::AlignVCenter, pin->name());
        } else if (pt.x() > 0) {
            painter.drawText(QRectF(9, pt.y()-5, 28, 10), Qt::AlignRight|Qt::AlignVCenter, pin->name());
        } else {
            painter.drawText(QRectF(-22, 34, 44, 10), Qt::AlignCenter, pin->name());
        }
    }

    painter.setPen(QPen(Qt::red, 1));
    for (const auto& pin : m_pins) {
        painter.setBrush(pin->highlighted() ? Qt::yellow : Qt::red);
        painter.drawEllipse(pin->localPos(), Pin::HoverRadius, Pin::HoverRadius);
    }

    painter.setPen(selected ? Qt::cyan : Qt::black);
    painter.setFont(QFont("Monospace", 7));
    painter.drawText(QRectF(-40,-58,80,12), Qt::AlignCenter, m_label);
    painter.restore();
}
