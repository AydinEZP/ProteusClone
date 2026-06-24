#pragma once
#include "../Component.h"
#include <QByteArray>
#include <array>

/**
 * Minimal educational MCU model.
 *
 * The MCU loads Intel HEX into Flash, keeps PC/ACC/internal RAM, and exposes
 * four 8-bit GPIO ports P0..P3. Each GPIO bit has a direction:
 *   1 = output (drives the circuit)
 *   0 = input  (samples the connected circuit)
 *
 * Bytecode subset:
 *   00          NOP
 *   10 imm      MOV A,#imm
 *   11 addr     MOV [addr],A
 *   12 addr     MOV A,[addr]
 *   20 imm      ADD A,#imm
 *   30 lo hi    JMP address
 *   40 pb       SETB Pn.bit        (also makes that bit output)
 *   41 pb       CLR  Pn.bit        (also makes that bit output)
 *   42 port     MOV Pn,A           (write full port, makes all bits output)
 *   43 port     MOV A,Pn           (read current port pin states)
 *   44 port mask DIR Pn,#mask      (1=output, 0=input)
 *   45 port imm MOV Pn,#imm        (write immediate, all bits output)
 */
class Microcontroller : public Component {
public:
    static constexpr int PortCount = 4;

    Microcontroller();

    bool loadIntelHex(const QString& path, QString* error = nullptr);
    int programSize() const { return m_flash.size(); }

    void resetCpu();
    void tickCpu();

    static constexpr int portCount() { return PortCount; }

    /** Current logic level seen by the CPU on a GPIO bit. */
    bool portBit(int port, int bit) const;

    /** Output latch value. Meaningful only when portBitIsOutput() is true. */
    bool outputBit(int port, int bit) const;

    bool portBitIsOutput(int port, int bit) const;
    quint8 portDirectionMask(int port) const;
    void setPortDirectionMask(int port, quint8 mask);

    /** Update one externally sampled input bit. Does not alter the output latch. */
    void setInputPortBit(int port, int bit, bool high);

    /** Current 8-bit value visible to the CPU, combining output latch and sampled inputs. */
    quint8 portValue(int port) const;

    QRectF boundingBox() const override;
    void draw(QPainter& painter, bool selected) const override;
    QJsonObject serialize() const override;
    void deserialize(const QJsonObject& obj) override;
    QMap<QString,QString> properties() const override;
    void setProperty(const QString& key, const QString& value) override;

private:
    bool validPortBit(int port, int bit) const;
    bool validPort(int port) const;

    QByteArray m_flash;
    QByteArray m_ram;
    QString m_firmwarePath;
    quint16 m_pc {0};
    quint8  m_acc {0};

    // Output latches, sampled external inputs, and direction masks.
    std::array<quint8, PortCount> m_portLatch {{0,0,0,0}};
    std::array<quint8, PortCount> m_portInput {{0,0,0,0}};
    std::array<quint8, PortCount> m_portDir   {{0,0,0,0}};
};
