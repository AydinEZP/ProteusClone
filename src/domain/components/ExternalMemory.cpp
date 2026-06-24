#include "ExternalMemory.h"
#include <QPainter>
#include <QJsonObject>
#include <QJsonArray>
#include <QtGlobal>

ExternalMemory::ExternalMemory() : Component("ExternalMemory"), m_ram(256, char(0)) { setLabel("RAM?"); for(int i=0;i<8;++i)addPin(std::make_shared<Pin>(QString("A%1").arg(i),PinType::Input,QPointF(-50,-34+i*8))); for(int i=0;i<8;++i)addPin(std::make_shared<Pin>(QString("D%1").arg(i),PinType::Bidirectional,QPointF(50,-34+i*8))); addPin(std::make_shared<Pin>("RD",PinType::Input,QPointF(-18,44))); addPin(std::make_shared<Pin>("WR",PinType::Input,QPointF(18,44))); updatePinWorldPositions(); }
quint8 ExternalMemory::read(quint16 address) const { return quint8(m_ram[int(address)%m_ram.size()]); }
void ExternalMemory::write(quint16 address, quint8 value){ m_ram[int(address)%m_ram.size()]=char(value); }
QRectF ExternalMemory::boundingBox() const { return QRectF(m_pos.x()-55,m_pos.y()-50,110,100); }
void ExternalMemory::draw(QPainter& painter,bool selected) const { painter.save(); painter.translate(m_pos); painter.rotate(m_rotation); painter.setPen(QPen(selected?Qt::cyan:Qt::darkMagenta,2)); painter.setBrush(QColor(230,220,245)); painter.drawRoundedRect(QRectF(-42,-42,84,84),4,4); painter.setFont(QFont("Monospace",8,QFont::Bold)); painter.setPen(Qt::darkMagenta); painter.drawText(QRectF(-40,-20,80,40),Qt::AlignCenter,"EXT\nRAM"); drawPinLabels(painter, QRectF(-42,-42,84,84), Qt::darkMagenta, 5); painter.setPen(QPen(Qt::red,1)); for(auto& pin:m_pins){painter.setBrush(pin->highlighted()?Qt::yellow:Qt::red); painter.drawEllipse(pin->localPos(),Pin::HoverRadius,Pin::HoverRadius);} painter.restore(); }
QJsonObject ExternalMemory::serialize() const { auto o=Component::serialize(); QJsonArray a; for(auto ch:m_ram)a.append(int(quint8(ch))); o["ram"]=a; return o; }
void ExternalMemory::deserialize(const QJsonObject& obj){ Component::deserialize(obj); auto arr=obj["ram"].toArray(); m_ram=QByteArray(256,char(0)); for(int i=0;i<arr.size()&&i<m_ram.size();++i)m_ram[i]=char(arr[i].toInt()&0xff); }
QMap<QString,QString> ExternalMemory::properties() const { return {{"label",m_label},{"sizeBytes",QString::number(m_ram.size())}}; }
void ExternalMemory::setProperty(const QString& key,const QString& value){ if(key=="label")m_label=value; Q_UNUSED(value); }
