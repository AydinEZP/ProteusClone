#pragma once
#include "../Component.h"
#include <QString>

/** Live 16x2 character LCD.  It understands a minimal HD44780-like bus: RS/RW/E/D0..D7. */
class LCD16x2 : public Component {
public:
    LCD16x2();
    void setText(const QString& line1, const QString& line2) { m_line1 = line1.left(16); m_line2 = line2.left(16); }
    QString line1() const { return m_line1; }
    QString line2() const { return m_line2; }
    void tickBus(bool rs, bool rw, bool e, quint8 data);
    QRectF boundingBox() const override;
    void draw(QPainter& painter, bool selected) const override;
    QJsonObject serialize() const override;
    void deserialize(const QJsonObject& obj) override;
    QMap<QString,QString> properties() const override;
    void setProperty(const QString& key, const QString& value) override;
private:
    void clear();
    void putChar(QChar ch);
    QString m_line1 {"                "};
    QString m_line2 {"                "};
    bool m_prevE {false};
    int  m_cursor {0};
};
