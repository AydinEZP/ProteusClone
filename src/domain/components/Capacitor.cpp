#include "Capacitor.h"
#include <QPainter>
#include <QRegularExpression>
#include <QLocale>
#include <cmath>

namespace {
static QString formatNumber(double v)
{
    QString s = QString::number(v, 'g', 8);
    if (s == "-0") s = "0";
    return s;
}

// Parses values such as "100", "100 nF", "0.1 uF", "1e-6 F", "470pF".
// defaultMultiplier is used when the user does not type a unit.
static bool parseCapacitanceToFarads(const QString& text, double defaultMultiplier, double& outFarads)
{
    QString t = text.trimmed();
    t.replace(QChar(0x00B5), "u"); // micro sign -> u
    t.replace(QChar(0x03BC), "u"); // greek mu -> u

    static const QRegularExpression re(
        R"(^\s*([+-]?(?:\d+(?:\.\d*)?|\.\d+)(?:[eE][+-]?\d+)?)\s*([munp]?f|uf|farad|farads)?\s*$)",
        QRegularExpression::CaseInsensitiveOption);

    const auto m = re.match(t);
    if (!m.hasMatch()) return false;

    bool ok = false;
    double numeric = QLocale::c().toDouble(m.captured(1), &ok);
    if (!ok || !std::isfinite(numeric) || numeric < 0.0) return false;

    QString unit = m.captured(2).toLower();
    double mult = defaultMultiplier;
    if (unit == "f" || unit == "farad" || unit == "farads") mult = 1.0;
    else if (unit == "mf") mult = 1e-3;
    else if (unit == "uf") mult = 1e-6;
    else if (unit == "nf") mult = 1e-9;
    else if (unit == "pf") mult = 1e-12;

    outFarads = numeric * mult;
    return std::isfinite(outFarads);
}
}

Capacitor::Capacitor()
    : Component("Capacitor")
{
    setLabel("C?");
    addPin(std::make_shared<Pin>("A", PinType::Passive, QPointF(-30, 0)));
    addPin(std::make_shared<Pin>("B", PinType::Passive, QPointF( 30, 0)));
    updatePinWorldPositions();
}

QRectF Capacitor::boundingBox() const
{
    return QRectF(m_pos.x() - 32, m_pos.y() - 12, 64, 24);
}

void Capacitor::draw(QPainter& painter, bool selected) const
{
    painter.save();
    painter.translate(m_pos);
    painter.rotate(m_rotation);
    if (m_mirrorH) painter.scale(-1, 1);
    if (m_mirrorV) painter.scale(1, -1);

    QPen pen(selected ? Qt::cyan : Qt::darkBlue, 2);
    painter.setPen(pen);
    painter.setBrush(Qt::NoBrush);

    painter.drawLine(QPointF(-30, 0), QPointF(-5, 0));
    painter.drawLine(QPointF(  5, 0), QPointF(30, 0));
    // Two plates
    painter.drawLine(QPointF(-5,-12), QPointF(-5, 12));
    painter.drawLine(QPointF( 5,-12), QPointF( 5, 12));

    painter.setPen(selected ? Qt::cyan : Qt::black);
    painter.setFont(QFont("Monospace", 7));
    painter.drawText(QRectF(-30,-26,60,14), Qt::AlignCenter,
                     QString("%1\n%2 nF").arg(m_label).arg(formatNumber(m_capacitance*1e9)));

    painter.setPen(QPen(Qt::red, 1));
    for (auto& pin : m_pins) {
        painter.setBrush(pin->highlighted() ? Qt::yellow : Qt::red);
        painter.drawEllipse(pin->localPos(), Pin::HoverRadius, Pin::HoverRadius);
    }
    painter.restore();
}

QJsonObject Capacitor::serialize() const
{
    QJsonObject obj = Component::serialize();
    obj["capacitance"] = m_capacitance;
    return obj;
}

void Capacitor::deserialize(const QJsonObject& obj)
{
    Component::deserialize(obj);
    m_capacitance = obj["capacitance"].toDouble(100e-9);
}

QMap<QString,QString> Capacitor::properties() const
{
    QMap<QString,QString> p;
    p["label"]       = m_label;
    // The editable field is numeric-only in nF to avoid QString::toDouble("100 nF") -> 0.
    p["capacitance_nF"] = formatNumber(m_capacitance * 1e9);
    return p;
}

void Capacitor::setProperty(const QString& key, const QString& value)
{
    if (key == "label") {
        m_label = value;
        return;
    }

    if (key == "capacitance" || key == "capacitance_nF") {
        double parsedFarads = m_capacitance;

        // For the PropertiesPanel's capacitance_nF field, a bare number means nF.
        // If the user explicitly types a unit, that unit wins: e.g. 0.1uF, 100nF, 1e-6F.
        const double defaultUnit = 1e-9;
        if (parseCapacitanceToFarads(value, defaultUnit, parsedFarads)) {
            m_capacitance = parsedFarads;
        }
    }
}
