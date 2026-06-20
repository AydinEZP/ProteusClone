#pragma once
#include "../Component.h"
#include <deque>
#include <vector>

/**
 * Two-channel oscilloscope measurement component.
 *
 * The canvas component owns three electrical pins:
 *  - CH1: channel 1 probe input
 *  - CH2: channel 2 probe input
 *  - GND: reference pin shared by both channels
 *
 * SimulationEngine pushes one sample per tick. The full-size user interface is
 * implemented by ui/OscilloscopePanel, while this component also draws a compact
 * live preview on the schematic canvas.
 *
 * Important: CH1 and CH2 have independent vertical scale and vertical offset.
 * Time/Div remains global because it represents the shared horizontal timebase
 * of the oscilloscope display.
 */
class Oscilloscope : public Component {
public:
    Oscilloscope();

    void pushSample(double ch1Voltage, double ch2Voltage, double timeSeconds);
    void clearSamples();

    QRectF boundingBox() const override;
    void draw(QPainter& painter, bool selected) const override;

    QJsonObject serialize() const override;
    void deserialize(const QJsonObject& obj) override;
    QMap<QString,QString> properties() const override;
    void setProperty(const QString& key, const QString& value) override;

    const std::deque<double>& ch1Samples() const { return m_ch1Samples; }
    const std::deque<double>& ch2Samples() const { return m_ch2Samples; }
    const std::deque<double>& timeSamples() const { return m_timeSamples; }

    bool ch1Enabled() const { return m_ch1Enabled; }
    bool ch2Enabled() const { return m_ch2Enabled; }
    void setCh1Enabled(bool enabled) { m_ch1Enabled = enabled; }
    void setCh2Enabled(bool enabled) { m_ch2Enabled = enabled; }

    double ch1VoltsPerDiv() const { return m_ch1VoltsPerDiv; }
    double ch2VoltsPerDiv() const { return m_ch2VoltsPerDiv; }
    void setCh1VoltsPerDiv(double value);
    void setCh2VoltsPerDiv(double value);

    double ch1VerticalOffset() const { return m_ch1VerticalOffset; }
    double ch2VerticalOffset() const { return m_ch2VerticalOffset; }
    void setCh1VerticalOffset(double value);
    void setCh2VerticalOffset(double value);

    // Backward-compatible single-scale API. Old code/properties that call these
    // methods now apply the same value to both channels.
    double voltsPerDiv() const { return m_ch1VoltsPerDiv; }
    void setVoltsPerDiv(double value);
    double verticalOffset() const { return m_ch1VerticalOffset; }
    void setVerticalOffset(double value);

    double timePerDiv() const { return m_timePerDiv; }
    void setTimePerDiv(double value);

    int maxSamples() const { return m_maxSamples; }

    // The oscilloscope memory depth is calculated automatically from
    // the horizontal time scale and the simulator sample interval.
    // It is intentionally not user-editable; changing Time/Div changes this value.
    double sampleInterval() const { return m_sampleInterval; }
    int estimatedRequiredSamples() const;
    int estimatedMemoryBytes() const;
    QString memorySummary() const;

    // Kept for backward compatibility with older saved projects/properties.
    // The next time Time/Div or a sample interval is known, auto-memory overrides it.
    void setMaxSamples(int count);

    QString channelSummary() const;

private:
    static double parseEngineeringValue(const QString& text, double fallback);
    void trimBuffers();
    void updateAutoMemory(bool allowShrink = false);
    bool maybeUpdateSampleInterval(double measuredDt);

    int m_maxSamples{600};             // automatically calculated memory depth
    double m_sampleInterval{0.01};     // seconds/sample, inferred from SimulationEngine ticks
    bool m_hasRealSample{false};       // false until the first simulator-provided sample arrives
    double m_ch1VoltsPerDiv{1.0};      // CH1 vertical scale
    double m_ch2VoltsPerDiv{1.0};      // CH2 vertical scale
    double m_timePerDiv{0.1};          // seconds per division, common horizontal timebase
    double m_ch1VerticalOffset{0.0};   // CH1 screen-centre voltage offset
    double m_ch2VerticalOffset{0.0};   // CH2 screen-centre voltage offset
    bool m_ch1Enabled{true};
    bool m_ch2Enabled{true};

    std::deque<double> m_timeSamples;
    std::deque<double> m_ch1Samples;
    std::deque<double> m_ch2Samples;
};
