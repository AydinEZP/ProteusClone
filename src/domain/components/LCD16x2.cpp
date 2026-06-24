#include "LCD16x2.h"
#include <QPainter>
#include <QJsonObject>

LCD16x2::LCD16x2() : Component("LCD16x2") {
    setLabel("LCD?");
    addPin(std::make_shared<Pin>("RS", PinType::Input, QPointF(-60,-30)));
    addPin(std::make_shared<Pin>("RW", PinType::Input, QPointF(-60,-18)));
    addPin(std::make_shared<Pin>("E",  PinType::Input, QPointF(-60,-6)));
    for(int i=0;i<8;++i) addPin(std::make_shared<Pin>(QString("D%1").arg(i), PinType::Bidirectional, QPointF(-60+i*14,30)));
    addPin(std::make_shared<Pin>("VCC", PinType::Power, QPointF(-30,-30)));
    addPin(std::make_shared<Pin>("GND", PinType::Ground, QPointF(-18,-30)));
    updatePinWorldPositions();
}
void LCD16x2::clear(){ m_line1=QString(16,' '); m_line2=QString(16,' '); m_cursor=0; }
void LCD16x2::putChar(QChar ch){ if(m_cursor<16) m_line1[m_cursor]=ch; else if(m_cursor<32) m_line2[m_cursor-16]=ch; m_cursor=(m_cursor+1)%32; }
void LCD16x2::tickBus(bool rs, bool rw, bool e, quint8 data){ bool rising=!m_prevE && e; m_prevE=e; if(!rising || rw) return; if(!rs){ if(data==0x01) clear(); else if(data & 0x80){ int a=data&0x7f; m_cursor=(a>=0x40)?16+(a-0x40):a; if(m_cursor<0)m_cursor=0; if(m_cursor>31)m_cursor=31; } } else { putChar(QChar(char(data))); } }
QRectF LCD16x2::boundingBox() const { return QRectF(m_pos.x()-65, m_pos.y()-38, 130, 76); }
void LCD16x2::draw(QPainter& painter, bool selected) const { painter.save(); painter.translate(m_pos); painter.rotate(m_rotation); painter.setPen(QPen(selected?Qt::cyan:QColor(0,100,0),2)); painter.setBrush(QColor(0,140,0)); painter.drawRoundedRect(QRectF(-62,-36,124,60),4,4); painter.setBrush(QColor(160,210,160)); painter.setPen(QPen(Qt::black,1)); painter.drawRect(QRectF(-52,-28,104,44)); painter.setFont(QFont("Courier",6,QFont::Bold)); painter.setPen(Qt::darkGreen); painter.drawText(QRectF(-50,-26,100,20),Qt::AlignVCenter|Qt::AlignLeft,m_line1); painter.drawText(QRectF(-50,-6,100,20),Qt::AlignVCenter|Qt::AlignLeft,m_line2); drawPinLabels(painter, QRectF(-62,-36,124,60), QColor(0,70,0), 5); painter.setPen(QPen(Qt::red,1)); for(auto& pin:m_pins){painter.setBrush(pin->highlighted()?Qt::yellow:Qt::red); painter.drawEllipse(pin->localPos(),Pin::HoverRadius,Pin::HoverRadius);} painter.restore(); }
QJsonObject LCD16x2::serialize() const { auto o=Component::serialize(); o["line1"]=m_line1; o["line2"]=m_line2; o["cursor"]=m_cursor; return o; }
void LCD16x2::deserialize(const QJsonObject& obj){ Component::deserialize(obj); m_line1=obj["line1"].toString(QString(16,' ')).left(16); m_line2=obj["line2"].toString(QString(16,' ')).left(16); m_cursor=obj["cursor"].toInt(0); }
QMap<QString,QString> LCD16x2::properties() const { return {{"label",m_label},{"line1",m_line1},{"line2",m_line2}}; }
void LCD16x2::setProperty(const QString& key,const QString& value){ if(key=="label")m_label=value; else if(key=="line1")m_line1=value.left(16); else if(key=="line2")m_line2=value.left(16); }
