#include "Inductor.h"
#include <QPainter>
#include <QtMath>
#include <QRegularExpression>
#include <QLocale>
#include <cmath>

namespace {
static QString formatIndNumber(double v)
{
    QString s = QString::number(v, 'g', 8);
    if (s == "-0") s = "0";
    return s;
}

static bool parseInductanceToHenrys(const QString& text, double defaultMultiplier, double& outHenrys)
{
    QString t = text.trimmed();
    t.replace(QChar(0x00B5), "u");
    t.replace(QChar(0x03BC), "u");

    static const QRegularExpression re(
        R"(^\s*([+-]?(?:\d+(?:\.\d*)?|\.\d+)(?:[eE][+-]?\d+)?)\s*([munp]?h|uh|henry|henrys)?\s*$)",
        QRegularExpression::CaseInsensitiveOption);

    const auto m = re.match(t);
    if (!m.hasMatch()) return false;

    bool ok = false;
    double numeric = QLocale::c().toDouble(m.captured(1), &ok);
    if (!ok || !std::isfinite(numeric) || numeric < 0.0) return false;

    QString unit = m.captured(2).toLower();
    double mult = defaultMultiplier;
    if (unit == "h" || unit == "henry" || unit == "henrys") mult = 1.0;
    else if (unit == "mh") mult = 1e-3;
    else if (unit == "uh") mult = 1e-6;
    else if (unit == "nh") mult = 1e-9;
    else if (unit == "ph") mult = 1e-12;

    outHenrys = numeric * mult;
    return std::isfinite(outHenrys);
}
}

Inductor::Inductor()
    : Component("Inductor")
{
    setLabel("L?");
    addPin(std::make_shared<Pin>("A", PinType::Passive, QPointF(-30, 0)));
    addPin(std::make_shared<Pin>("B", PinType::Passive, QPointF( 30, 0)));
    updatePinWorldPositions();
}

QRectF Inductor::boundingBox() const
{
    return QRectF(m_pos.x()-32, m_pos.y()-12, 64, 24);
}

void Inductor::draw(QPainter& painter, bool selected) const
{
    painter.save();
    painter.translate(m_pos);
    painter.rotate(m_rotation);
    if (m_mirrorH) painter.scale(-1, 1);
    if (m_mirrorV) painter.scale(1, -1);

    QPen pen(selected ? Qt::cyan : Qt::darkMagenta, 2);
    painter.setPen(pen);
    painter.setBrush(Qt::NoBrush);

    painter.drawLine(QPointF(-30,0), QPointF(-18,0));
    painter.drawLine(QPointF( 18,0), QPointF( 30,0));

    // Draw four arcs representing coil turns
    for (int i = 0; i < 4; ++i) {
        double x = -18.0 + i * 9.0;
        painter.drawArc(QRectF(x, -6, 9, 12), 0 * 16, 180 * 16);
    }

    painter.setPen(selected ? Qt::cyan : Qt::black);
    painter.setFont(QFont("Monospace", 7));
    painter.drawText(QRectF(-30,-26,60,14), Qt::AlignCenter,
                     QString("%1\n%2 mH").arg(m_label).arg(formatIndNumber(m_inductance*1e3)));

    painter.setPen(QPen(Qt::red, 1));
    for (auto& pin : m_pins) {
        painter.setBrush(pin->highlighted() ? Qt::yellow : Qt::red);
        painter.drawEllipse(pin->localPos(), Pin::HoverRadius, Pin::HoverRadius);
    }
    painter.restore();
}

QJsonObject Inductor::serialize() const
{
    QJsonObject obj = Component::serialize();
    obj["inductance"] = m_inductance;
    return obj;
}

void Inductor::deserialize(const QJsonObject& obj)
{
    Component::deserialize(obj);
    m_inductance = obj["inductance"].toDouble(1e-3);
}

QMap<QString,QString> Inductor::properties() const
{
    QMap<QString,QString> p;
    p["label"]      = m_label;
    // Numeric-only in mH for reliable editing. Explicit units are also accepted.
    p["inductance_mH"] = formatIndNumber(m_inductance * 1e3);
    return p;
}

void Inductor::setProperty(const QString& key, const QString& value)
{
    if (key == "label") {
        m_label = value;
        return;
    }
    if (key == "inductance" || key == "inductance_mH") {
        double parsedHenrys = m_inductance;
        if (parseInductanceToHenrys(value, 1e-3, parsedHenrys))
            m_inductance = parsedHenrys;
    }
}
