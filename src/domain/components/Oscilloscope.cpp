#include "Oscilloscope.h"
#include <QPainter>
#include <QJsonObject>
#include <QRegularExpression>
#include <algorithm>
#include <cmath>

namespace {
constexpr double kMinVoltsPerDiv = 1e-6;
constexpr double kMinTimePerDiv  = 1e-6;
constexpr int    kScopeDivX      = 10;
constexpr int    kScopeDivY      = 8;
constexpr int    kMinScopeSamples = 50;
constexpr int    kMaxScopeSamples = 200000;
constexpr double kMemoryHeadroom  = 2.50;  // intentionally over-allocate to avoid trace jumps during runtime
}

Oscilloscope::Oscilloscope() : Component("Oscilloscope")
{
    setLabel("OSC?");
    addPin(std::make_shared<Pin>("CH1", PinType::Input,  QPointF(-55, -12)));
    addPin(std::make_shared<Pin>("CH2", PinType::Input,  QPointF(-55,  12)));
    addPin(std::make_shared<Pin>("GND", PinType::Ground, QPointF(-55,  32)));
    clearSamples();
    updatePinWorldPositions();
}

void Oscilloscope::setCh1VoltsPerDiv(double value)
{
    if (std::isfinite(value) && value >= kMinVoltsPerDiv)
        m_ch1VoltsPerDiv = value;
}

void Oscilloscope::setCh2VoltsPerDiv(double value)
{
    if (std::isfinite(value) && value >= kMinVoltsPerDiv)
        m_ch2VoltsPerDiv = value;
}

void Oscilloscope::setVoltsPerDiv(double value)
{
    setCh1VoltsPerDiv(value);
    setCh2VoltsPerDiv(value);
}

void Oscilloscope::setCh1VerticalOffset(double value)
{
    if (std::isfinite(value))
        m_ch1VerticalOffset = value;
}

void Oscilloscope::setCh2VerticalOffset(double value)
{
    if (std::isfinite(value))
        m_ch2VerticalOffset = value;
}

void Oscilloscope::setVerticalOffset(double value)
{
    setCh1VerticalOffset(value);
    setCh2VerticalOffset(value);
}

void Oscilloscope::setTimePerDiv(double value)
{
    if (std::isfinite(value) && value >= kMinTimePerDiv) {
        m_timePerDiv = value;
        // User changed the horizontal timebase, so shrinking/growing memory is allowed here.
        updateAutoMemory(true);
    }
}

int Oscilloscope::estimatedRequiredSamples() const
{
    const double visibleWindow = std::max(kMinTimePerDiv, m_timePerDiv) * kScopeDivX;
    const double dt = std::max(1e-9, m_sampleInterval);
    const int required = static_cast<int>(std::ceil((visibleWindow / dt) * kMemoryHeadroom)) + 2;
    return std::clamp(required, kMinScopeSamples, kMaxScopeSamples);
}

int Oscilloscope::estimatedMemoryBytes() const
{
    // Three double arrays are stored: time, CH1 voltage, CH2 voltage.
    return estimatedRequiredSamples() * static_cast<int>(3 * sizeof(double));
}

QString Oscilloscope::memorySummary() const
{
    const int samples = estimatedRequiredSamples();
    const double bytes = static_cast<double>(estimatedMemoryBytes());
    QString size;
    if (bytes >= 1024.0 * 1024.0)
        size = QString("%1 MiB").arg(bytes / (1024.0 * 1024.0), 0, 'f', 2);
    else
        size = QString("%1 KiB").arg(bytes / 1024.0, 0, 'f', 1);

    return QString("%1 samples, %2, dt=%3 s")
        .arg(samples)
        .arg(size)
        .arg(m_sampleInterval, 0, 'g', 4);
}

void Oscilloscope::updateAutoMemory(bool allowShrink)
{
    const int required = estimatedRequiredSamples();

    // During live sampling, never continuously shrink/reallocate the trace buffer.
    // Repeated resize/trim events shift the left edge of the plot and create a visible
    // one-frame horizontal jump. We grow immediately when required, and shrink only
    // after an explicit Time/Div change, Clear, Load, or legacy setMaxSamples call.
    if (allowShrink) {
        m_maxSamples = required;
        trimBuffers();
        return;
    }

    if (required > m_maxSamples) {
        m_maxSamples = required;
        trimBuffers();
    }
}

void Oscilloscope::setMaxSamples(int count)
{
    // Legacy/manual value from old projects. Keep it safe, then immediately recalculate
    // from Time/Div and sample interval so the UI never depends on user-entered memory.
    m_maxSamples = std::clamp(count, kMinScopeSamples, kMaxScopeSamples);
    updateAutoMemory(true);
}

void Oscilloscope::trimBuffers()
{
    while ((int)m_timeSamples.size() > m_maxSamples) m_timeSamples.pop_front();
    while ((int)m_ch1Samples.size() > m_maxSamples) m_ch1Samples.pop_front();
    while ((int)m_ch2Samples.size() > m_maxSamples) m_ch2Samples.pop_front();
}

void Oscilloscope::clearSamples()
{
    m_timeSamples.clear();
    m_ch1Samples.clear();
    m_ch2Samples.clear();
    m_hasRealSample = false;
    updateAutoMemory(true);
}

bool Oscilloscope::maybeUpdateSampleInterval(double measuredDt)
{
    if (!std::isfinite(measuredDt) || measuredDt <= 1e-9)
        return false;

    // Ignore tiny timer jitter. Only a real dt/timebase change should alter the
    // oscilloscope memory calculation. This prevents buffer-length oscillation and
    // the visible one-frame shift in the plot.
    const double oldDt = std::max(1e-9, m_sampleInterval);
    const double relativeChange = std::abs(measuredDt - oldDt) / oldDt;
    if (relativeChange < 0.05)
        return false;

    m_sampleInterval = measuredDt;
    updateAutoMemory(false);
    return true;
}

void Oscilloscope::pushSample(double ch1Voltage, double ch2Voltage, double timeSeconds)
{
    if (!std::isfinite(ch1Voltage)) ch1Voltage = 0.0;
    if (!std::isfinite(ch2Voltage)) ch2Voltage = 0.0;

    if (!std::isfinite(timeSeconds))
        timeSeconds = m_timeSamples.empty() ? 0.0 : (m_timeSamples.back() + m_sampleInterval);

    // The scope display assumes strictly increasing simulation-time samples.
    // If the simulator was restarted, a project was loaded, or old seeded/future samples
    // exist, reset the trace instead of appending a smaller timestamp after a larger one.
    if (!m_timeSamples.empty() && timeSeconds < m_timeSamples.back() - 1e-9) {
        m_timeSamples.clear();
        m_ch1Samples.clear();
        m_ch2Samples.clear();
        m_hasRealSample = false;
    }

    if (!m_timeSamples.empty() && timeSeconds <= m_timeSamples.back())
        timeSeconds = m_timeSamples.back() + m_sampleInterval;

    if (!m_timeSamples.empty())
        maybeUpdateSampleInterval(timeSeconds - m_timeSamples.back());
    else
        updateAutoMemory(false);

    m_timeSamples.push_back(timeSeconds);
    m_ch1Samples.push_back(ch1Voltage);
    m_ch2Samples.push_back(ch2Voltage);
    m_hasRealSample = true;
    trimBuffers();
}

QRectF Oscilloscope::boundingBox() const
{
    return QRectF(m_pos.x() - 65, m_pos.y() - 45, 130, 90);
}

static void drawWaveform(QPainter& painter,
                         const std::deque<double>& samples,
                         const QRectF& r,
                         double voltsPerDiv,
                         double verticalOffset,
                         const QColor& color)
{
    if (samples.size() < 2) return;
    const double halfH = r.height() / 2.0;
    const double voltsFullScale = voltsPerDiv * (kScopeDivY / 2.0);
    if (voltsFullScale <= 0.0) return;

    const int n = static_cast<int>(samples.size());
    painter.setPen(QPen(color, 2));

    QPointF prevPoint;
    bool havePrev = false;
    for (int i = 0; i < n; ++i) {
        const double x = r.left() + r.width() * i / std::max(1, n - 1);
        const double normalized = (samples[i] + verticalOffset) / voltsFullScale;
        const double y = std::clamp(r.center().y() - normalized * halfH, r.top(), r.bottom());
        const QPointF currentPoint(x, y);

        if (havePrev) {
            painter.drawLine(prevPoint, currentPoint);
        }

        prevPoint = currentPoint;
        havePrev = true;
    }
}

void Oscilloscope::draw(QPainter& painter, bool selected) const
{
    painter.save();
    painter.translate(m_pos);
    painter.rotate(m_rotation);
    if (m_mirrorH) painter.scale(-1, 1);
    if (m_mirrorV) painter.scale(1, -1);

    const QRectF body(-58, -38, 116, 76);
    const QRectF screen(-42, -26, 84, 48);

    painter.setPen(QPen(selected ? Qt::cyan : Qt::black, 2));
    painter.setBrush(QColor(12, 20, 12));
    painter.drawRoundedRect(body, 5, 5);

    // Draw the compact scope grid with cosmetic pens so the grid does not become
    // visually unstable under canvas zoom/pan transforms.
    const bool oldAA = painter.renderHints().testFlag(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::Antialiasing, false);
    QPen minorPen(QColor(35, 80, 35), 1);
    QPen majorPen(QColor(75, 135, 75), 1);
    QPen axisPen(QColor(95, 190, 95), 1);
    minorPen.setCosmetic(true);
    majorPen.setCosmetic(true);
    axisPen.setCosmetic(true);

    const int minorX = kScopeDivX * 2;
    const int minorY = kScopeDivY * 2;
    for (int i = 0; i <= minorX; ++i) {
        double x = screen.left() + screen.width() * i / minorX;
        painter.setPen((i % 2 == 0) ? majorPen : minorPen);
        painter.drawLine(QPointF(x, screen.top()), QPointF(x, screen.bottom()));
    }
    for (int i = 0; i <= minorY; ++i) {
        double y = screen.top() + screen.height() * i / minorY;
        painter.setPen((i % 2 == 0) ? majorPen : minorPen);
        painter.drawLine(QPointF(screen.left(), y), QPointF(screen.right(), y));
    }

    painter.setPen(axisPen);
    painter.drawLine(QPointF(screen.left(), screen.center().y()), QPointF(screen.right(), screen.center().y()));
    painter.drawLine(QPointF(screen.center().x(), screen.top()), QPointF(screen.center().x(), screen.bottom()));
    painter.setRenderHint(QPainter::Antialiasing, oldAA);

    if (m_ch1Enabled) drawWaveform(painter, m_ch1Samples, screen, m_ch1VoltsPerDiv, m_ch1VerticalOffset, QColor(0, 255, 70));
    if (m_ch2Enabled) drawWaveform(painter, m_ch2Samples, screen, m_ch2VoltsPerDiv, m_ch2VerticalOffset, QColor(255, 210, 0));

    painter.setPen(QPen(Qt::white, 1));
    painter.setFont(QFont("Monospace", 6));
    painter.drawText(QRectF(-54, -36, 108, 9), Qt::AlignCenter,
                     QString("%1  CH1:%2V/div CH2:%3V/div  %4s/div").arg(m_label).arg(m_ch1VoltsPerDiv).arg(m_ch2VoltsPerDiv).arg(m_timePerDiv));
    painter.setPen(QPen(QColor(0,255,70), 1));
    painter.drawText(QPointF(18, 35), "CH1");
    painter.setPen(QPen(QColor(255,210,0), 1));
    painter.drawText(QPointF(37, 35), "CH2");

    drawPinLabels(painter, body, QColor(210,255,210), 5);
    painter.setPen(QPen(Qt::red, 1));
    for (auto& pin : m_pins) {
        painter.setBrush(pin->highlighted() ? Qt::yellow : Qt::red);
        painter.drawEllipse(pin->localPos(), Pin::HoverRadius, Pin::HoverRadius);
    }

    painter.restore();
}

QJsonObject Oscilloscope::serialize() const
{
    auto obj = Component::serialize();
    obj["ch1VoltsPerDiv"] = m_ch1VoltsPerDiv;
    obj["ch2VoltsPerDiv"] = m_ch2VoltsPerDiv;
    obj["voltsPerDiv"] = m_ch1VoltsPerDiv; // legacy compatibility
    obj["timePerDiv"] = m_timePerDiv;
    obj["ch1VerticalOffset"] = m_ch1VerticalOffset;
    obj["ch2VerticalOffset"] = m_ch2VerticalOffset;
    obj["verticalOffset"] = m_ch1VerticalOffset; // legacy compatibility
    obj["ch1Enabled"] = m_ch1Enabled;
    obj["ch2Enabled"] = m_ch2Enabled;
    obj["maxSamples"] = m_maxSamples;
    obj["sampleInterval"] = m_sampleInterval;
    return obj;
}

void Oscilloscope::deserialize(const QJsonObject& obj)
{
    Component::deserialize(obj);
    const double legacyScale = obj["voltsPerDiv"].toDouble(obj["scale"].toDouble(1.0));
    setCh1VoltsPerDiv(obj["ch1VoltsPerDiv"].toDouble(legacyScale));
    setCh2VoltsPerDiv(obj["ch2VoltsPerDiv"].toDouble(legacyScale));
    setTimePerDiv(obj["timePerDiv"].toDouble(0.1));
    const double legacyOffset = obj["verticalOffset"].toDouble(0.0);
    setCh1VerticalOffset(obj["ch1VerticalOffset"].toDouble(legacyOffset));
    setCh2VerticalOffset(obj["ch2VerticalOffset"].toDouble(legacyOffset));
    m_ch1Enabled = obj["ch1Enabled"].toBool(true);
    m_ch2Enabled = obj["ch2Enabled"].toBool(true);
    m_sampleInterval = obj["sampleInterval"].toDouble(0.01);
    setMaxSamples(obj["maxSamples"].toInt(600));
}

QMap<QString,QString> Oscilloscope::properties() const
{
    return {
        {"label", m_label},
        {"ch1_volts_per_div", QString::number(m_ch1VoltsPerDiv)},
        {"ch2_volts_per_div", QString::number(m_ch2VoltsPerDiv)},
        {"time_per_div_s", QString::number(m_timePerDiv)},
        {"ch1_vertical_offset_v", QString::number(m_ch1VerticalOffset)},
        {"ch2_vertical_offset_v", QString::number(m_ch2VerticalOffset)},
        {"ch1_enabled", m_ch1Enabled ? "true" : "false"},
        {"ch2_enabled", m_ch2Enabled ? "true" : "false"},
        {"sample_interval_s", QString::number(m_sampleInterval)},
        {"memory_auto", memorySummary()}
    };
}

void Oscilloscope::setProperty(const QString& key, const QString& value)
{
    if (key == "label") m_label = value;
    else if (key == "ch1_volts_per_div") setCh1VoltsPerDiv(parseEngineeringValue(value, m_ch1VoltsPerDiv));
    else if (key == "ch2_volts_per_div") setCh2VoltsPerDiv(parseEngineeringValue(value, m_ch2VoltsPerDiv));
    else if (key == "volts_per_div" || key == "scale") setVoltsPerDiv(parseEngineeringValue(value, m_ch1VoltsPerDiv));
    else if (key == "time_per_div_s") setTimePerDiv(parseEngineeringValue(value, m_timePerDiv));
    else if (key == "ch1_vertical_offset_v") setCh1VerticalOffset(parseEngineeringValue(value, m_ch1VerticalOffset));
    else if (key == "ch2_vertical_offset_v") setCh2VerticalOffset(parseEngineeringValue(value, m_ch2VerticalOffset));
    else if (key == "vertical_offset_v") setVerticalOffset(parseEngineeringValue(value, m_ch1VerticalOffset));
    else if (key == "ch1_enabled") m_ch1Enabled = (value.trimmed().toLower() != "false" && value.trimmed() != "0");
    else if (key == "ch2_enabled") m_ch2Enabled = (value.trimmed().toLower() != "false" && value.trimmed() != "0");
    else if (key == "sample_interval_s") { m_sampleInterval = parseEngineeringValue(value, m_sampleInterval); updateAutoMemory(true); }
    else if (key == "max_samples") setMaxSamples(value.toInt()); // legacy compatibility only
}

QString Oscilloscope::channelSummary() const
{
    const double ch1 = m_ch1Samples.empty() ? 0.0 : m_ch1Samples.back();
    const double ch2 = m_ch2Samples.empty() ? 0.0 : m_ch2Samples.back();
    return QString("CH1=%1 V, CH2=%2 V").arg(ch1, 0, 'f', 3).arg(ch2, 0, 'f', 3);
}

double Oscilloscope::parseEngineeringValue(const QString& text, double fallback)
{
    QString s = text.trimmed().toLower();
    if (s.isEmpty()) return fallback;

    QRegularExpression re(R"(^\s*([-+]?\d*\.?\d+(?:[eE][-+]?\d+)?)\s*([a-zµ]*)\s*$)");
    auto m = re.match(s);
    if (!m.hasMatch()) return fallback;

    bool ok = false;
    double v = m.captured(1).toDouble(&ok);
    if (!ok || !std::isfinite(v)) return fallback;

    QString unit = m.captured(2);
    if (unit.startsWith("mv")) v *= 1e-3;
    else if (unit.startsWith("uv") || unit.startsWith(QString::fromUtf8("µv"))) v *= 1e-6;
    else if (unit.startsWith("kv")) v *= 1e3;
    else if (unit.startsWith("ms")) v *= 1e-3;
    else if (unit.startsWith("us") || unit.startsWith(QString::fromUtf8("µs"))) v *= 1e-6;
    else if (unit.startsWith("ns")) v *= 1e-9;
    return v;
}
