#pragma once
#include "../Component.h"
#include <QByteArray>

class ExternalMemory : public Component {
public:
    ExternalMemory();
    quint8 read(quint16 address) const;
    void write(quint16 address, quint8 value);
    QRectF boundingBox() const override;
    void draw(QPainter& painter, bool selected) const override;
    QJsonObject serialize() const override;
    void deserialize(const QJsonObject& obj) override;
    QMap<QString,QString> properties() const override;
    void setProperty(const QString& key, const QString& value) override;
private:
    QByteArray m_ram;
};
