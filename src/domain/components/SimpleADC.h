#pragma once
#include <cstdint>
#include "../Component.h"

/** Idealized ADC with VIN, VREF+/VREF-, configurable bit width, saturation and conversion delay. */
class SimpleADC : public Component {
public:
    SimpleADC();
    void setInputVoltage(double v) { m_inputVoltage = v; }
    void setVRef(double vMinus, double vPlus) { m_vrefMinus = vMinus; m_vrefPlus = vPlus; }
    void setBits(int bits);
    int bits() const { return m_bits; }
    double conversionDelayMs() const { return m_conversionDelayMs; }
    uint32_t digitalOutput() const { return m_outputCode; }
    uint32_t idealCodeFromCurrentInput() const;
    void tickConversion(double dtSeconds);

    QRectF boundingBox() const override;
    void draw(QPainter& painter, bool selected) const override;
    QJsonObject serialize() const override;
    void deserialize(const QJsonObject& obj) override;
    QMap<QString,QString> properties() const override;
    void setProperty(const QString& key, const QString& value) override;

private:
    void rebuildPins();
    double  m_inputVoltage {0.0};
    double  m_vrefMinus    {0.0};
    double  m_vrefPlus     {5.0};
    int     m_bits         {8};
    double  m_conversionDelayMs {1.0};
    uint32_t m_outputCode {0};
    uint32_t m_pendingCode {0};
    bool     m_pending {false};
    double   m_pendingElapsedMs {0.0};
};
