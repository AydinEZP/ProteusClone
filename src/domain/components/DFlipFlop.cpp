#include "DFlipFlop.h"
#include <QPainter>

DFlipFlop::DFlipFlop()
    : Component("DFlipFlop")
{
    setLabel("DFF?");
    addPin(std::make_shared<Pin>("D",   PinType::Input,  QPointF(-32, -10)));
    addPin(std::make_shared<Pin>("CLK", PinType::Input,  QPointF(-32,  10)));
    addPin(std::make_shared<Pin>("Q",   PinType::Output, QPointF( 32, -10)));
    addPin(std::make_shared<Pin>("QB",  PinType::Output, QPointF( 32,  10)));
    updatePinWorldPositions();
}

void DFlipFlop::setClk(bool c)
{
    bool risingEdge = (!m_clk && c);
    m_clk = c;
    if (risingEdge) { m_q = m_d; m_undefined = false; }
}

QRectF DFlipFlop::boundingBox() const
{
    return QRectF(m_pos.x()-34, m_pos.y()-26, 68, 52);
}

void DFlipFlop::draw(QPainter& painter, bool selected) const
{
    painter.save();
    painter.translate(m_pos);
    painter.rotate(m_rotation);

    QPen pen(selected ? Qt::cyan : Qt::darkBlue, 2);
    painter.setPen(pen);
    painter.setBrush(QColor(220,220,255));
    painter.drawRect(QRectF(-28,-24,56,48));

    painter.setFont(QFont("Monospace", 8, QFont::Bold));
    painter.setPen(QPen(Qt::darkBlue));
    painter.drawText(QRectF(-28,-24,56,48), Qt::AlignCenter, "D\nFF");

    // Clock triangle symbol
    QPolygonF tri;
    tri << QPointF(-28,6) << QPointF(-22,10) << QPointF(-28,14);
    painter.setBrush(Qt::darkBlue);
    painter.drawPolygon(tri);

    // Q state
    painter.setBrush(m_undefined ? Qt::yellow : (m_q ? Qt::green : Qt::red));
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(QPointF(26,-10), 4, 4);

    drawPinLabels(painter, QRectF(-28,-24,56,48), Qt::darkBlue, 6);
    painter.setPen(QPen(Qt::red, 1));
    for (auto& pin : m_pins) {
        painter.setBrush(pin->highlighted() ? Qt::yellow : Qt::red);
        painter.drawEllipse(pin->localPos(), Pin::HoverRadius, Pin::HoverRadius);
    }
    painter.restore();
}

QJsonObject DFlipFlop::serialize() const { auto o=Component::serialize(); o["q"]=m_q; o["undefined"]=m_undefined; return o; }
void DFlipFlop::deserialize(const QJsonObject& obj){ Component::deserialize(obj); m_q=obj["q"].toBool(false); m_undefined=obj["undefined"].toBool(false); }
QMap<QString,QString> DFlipFlop::properties() const { return {{"label",m_label},{"Q",m_undefined?"Undefined":(m_q?"HIGH":"LOW")}}; }
void DFlipFlop::setProperty(const QString& key,const QString& value){ if(key=="label") m_label=value; }
