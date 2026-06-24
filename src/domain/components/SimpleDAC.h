#pragma once
#include <cstdint>
#include "../Component.h"

/** Idealized DAC with configurable bit width, Vref+/Vref-, and conversion delay. */
class SimpleDAC : public Component {
public:
    SimpleDAC();
    void setInput(uint32_t input) { m_input = input; }
    void setVRef(double vMinus, double vPlus) { m_vrefMinus = vMinus; m_vrefPlus = vPlus; }
    void setBits(int bits);
    int bits() const { return m_bits; }
    double conversionDelayMs() const { return m_conversionDelayMs; }
    double analogOutput() const { return m_outputVoltage; }
    double idealOutputFromCurrentInput() const;
    void tickConversion(double dtSeconds);

    QRectF boundingBox() const override;
    void draw(QPainter& painter, bool selected) const override;
    QJsonObject serialize() const override;
    void deserialize(const QJsonObject& obj) override;
    QMap<QString,QString> properties() const override;
    void setProperty(const QString& key, const QString& value) override;
private:
    void rebuildPins();
    uint32_t m_input {0};
    double   m_vrefMinus {0.0};
    double   m_vrefPlus {5.0};
    int      m_bits {8};
    double   m_conversionDelayMs {1.0};
    double   m_outputVoltage {0.0};
    double   m_pendingVoltage {0.0};
    bool     m_pending {false};
    double   m_pendingElapsedMs {0.0};
};
