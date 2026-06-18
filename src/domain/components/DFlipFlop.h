#pragma once
#include "../Component.h"

class DFlipFlop : public Component {
public:
    DFlipFlop();
    void setD(bool d) { m_d = d; }
    void setClk(bool c);
    void setUndefined(bool u) { m_undefined = u; }
    bool q() const { return m_q; }
    bool qBar() const { return !m_q; }
    bool outputValid() const { return !m_undefined; }

    QRectF boundingBox() const override;
    void draw(QPainter& painter, bool selected) const override;
    QJsonObject serialize() const override;
    void deserialize(const QJsonObject& obj) override;
    QMap<QString,QString> properties() const override;
    void setProperty(const QString& key, const QString& value) override;
private:
    bool m_d {false};
    bool m_clk {false};
    bool m_q {false};
    bool m_undefined {false};
};
