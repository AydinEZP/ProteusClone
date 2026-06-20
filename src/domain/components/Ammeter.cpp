#include "Ammeter.h"
#include <QPainter>
Ammeter::Ammeter() : Component("Ammeter") { setLabel("AM?"); addPin(std::make_shared<Pin>("IN", PinType::Passive, QPointF(-30,0))); addPin(std::make_shared<Pin>("OUT", PinType::Passive, QPointF(30,0))); updatePinWorldPositions(); }
QRectF Ammeter::boundingBox() const { return QRectF(m_pos.x()-36,m_pos.y()-22,72,44); }
void Ammeter::draw(QPainter& painter, bool selected) const { painter.save(); painter.translate(m_pos); painter.rotate(m_rotation); QPen pen(selected?Qt::cyan:Qt::darkRed,2); painter.setPen(pen); painter.setBrush(QColor(255,245,245)); painter.drawRoundedRect(QRectF(-32,-18,64,36),5,5); painter.setFont(QFont("Monospace",7,QFont::Bold)); painter.drawText(QRectF(-30,-12,60,24), Qt::AlignCenter, QString("A\n%1").arg(m_reading,0,'f',3)); painter.drawLine(QPointF(-32,0),QPointF(-30,0)); painter.drawLine(QPointF(32,0),QPointF(30,0)); painter.setPen(QPen(Qt::red,1)); for(auto& pin:m_pins){painter.setBrush(pin->highlighted()?Qt::yellow:Qt::red); painter.drawEllipse(pin->localPos(),Pin::HoverRadius,Pin::HoverRadius);} painter.restore(); }
QJsonObject Ammeter::serialize() const { auto o=Component::serialize(); o["reading"]=m_reading; return o; }
void Ammeter::deserialize(const QJsonObject& obj){ Component::deserialize(obj); m_reading=obj["reading"].toDouble(0.0); }
QMap<QString,QString> Ammeter::properties() const { return {{"label",m_label},{"reading",QString::number(m_reading)}}; }
void Ammeter::setProperty(const QString& key,const QString& value){ if(key=="label")m_label=value; if(key=="reading")m_reading=value.toDouble(); }
