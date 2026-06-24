#include "Microcontroller.h"
#include <QPainter>
#include <QFile>
#include <QTextStream>
#include <QJsonObject>
#include <QJsonArray>
#include <algorithm>

Microcontroller::Microcontroller()
    : Component("Microcontroller"), m_ram(128, char(0))
{
    setLabel("MCU?");

    // Two ports on the left and two on the right. Spacing is intentionally
    // generous so pin labels stay readable even on a dense schematic.
    for (int i = 0; i < 8; ++i)
        addPin(std::make_shared<Pin>(QString("P0.%1").arg(i), PinType::Bidirectional,
                                     QPointF(-68, -76 + i * 8)));
    for (int i = 0; i < 8; ++i)
        addPin(std::make_shared<Pin>(QString("P1.%1").arg(i), PinType::Bidirectional,
                                     QPointF(-68, 20 + i * 8)));
    for (int i = 0; i < 8; ++i)
        addPin(std::make_shared<Pin>(QString("P2.%1").arg(i), PinType::Bidirectional,
                                     QPointF(68, -76 + i * 8)));
    for (int i = 0; i < 8; ++i)
        addPin(std::make_shared<Pin>(QString("P3.%1").arg(i), PinType::Bidirectional,
                                     QPointF(68, 20 + i * 8)));

    addPin(std::make_shared<Pin>("VCC", PinType::Power, QPointF(-18, -104)));
    addPin(std::make_shared<Pin>("GND", PinType::Ground, QPointF(18, 104)));
    updatePinWorldPositions();
}

static int hexByte(const QString& s, bool* ok)
{
    return s.toInt(ok, 16);
}

bool Microcontroller::loadIntelHex(const QString& path, QString* error)
{
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        if (error) *error = "Cannot open HEX file";
        return false;
    }

    QByteArray image;
    QTextStream ts(&f);
    int lineNo = 0;
    quint32 base = 0;

    while (!ts.atEnd()) {
        const QString line = ts.readLine().trimmed();
        ++lineNo;
        if (line.isEmpty()) continue;
        if (!line.startsWith(':')) {
            if (error) *error = QString("HEX line %1 does not start with ':'").arg(lineNo);
            return false;
        }

        bool ok = true;
        const int count = hexByte(line.mid(1, 2), &ok);
        const int addr = line.mid(3, 4).toInt(&ok, 16);
        const int type = hexByte(line.mid(7, 2), &ok);
        if (!ok || line.size() < 11 + count * 2) {
            if (error) *error = QString("Invalid HEX record at line %1").arg(lineNo);
            return false;
        }

        int sum = count + ((addr >> 8) & 0xff) + (addr & 0xff) + type;
        QByteArray data;
        for (int i = 0; i < count; ++i) {
            const int b = hexByte(line.mid(9 + i * 2, 2), &ok);
            if (!ok) {
                if (error) *error = QString("Invalid data byte at line %1").arg(lineNo);
                return false;
            }
            data.append(char(b));
            sum += b;
        }

        const int chk = hexByte(line.mid(9 + count * 2, 2), &ok);
        if (!ok || ((sum + chk) & 0xff) != 0) {
            if (error) *error = QString("Checksum error at line %1").arg(lineNo);
            return false;
        }

        if (type == 0) {
            const quint32 abs = base + quint32(addr);
            if (image.size() < int(abs + count)) image.resize(int(abs + count));
            for (int i = 0; i < count; ++i) image[int(abs) + i] = data[i];
        } else if (type == 1) {
            break;
        } else if (type == 4 && count == 2) {
            base = (quint32(quint8(data[0])) << 24) |
                   (quint32(quint8(data[1])) << 16);
        }
    }

    m_flash = image;
    m_firmwarePath = path;
    resetCpu();
    return true;
}

bool Microcontroller::validPort(int port) const
{
    return port >= 0 && port < PortCount;
}

bool Microcontroller::validPortBit(int port, int bit) const
{
    return validPort(port) && bit >= 0 && bit < 8;
}

void Microcontroller::resetCpu()
{
    m_pc = 0;
    m_acc = 0;
    m_portLatch.fill(0);
    m_portInput.fill(0);
    // GPIO starts high-impedance/input. SETB/CLR and full-port output
    // instructions explicitly turn the affected bits into outputs.
    m_portDir.fill(0);
    std::fill(m_ram.begin(), m_ram.end(), char(0));
}

bool Microcontroller::portBit(int port, int bit) const
{
    if (!validPortBit(port, bit)) return false;
    const quint8 mask = quint8(1u << bit);
    return (portBitIsOutput(port, bit) ? m_portLatch[port] : m_portInput[port]) & mask;
}

bool Microcontroller::outputBit(int port, int bit) const
{
    if (!validPortBit(port, bit)) return false;
    return (m_portLatch[port] >> bit) & 1u;
}

bool Microcontroller::portBitIsOutput(int port, int bit) const
{
    if (!validPortBit(port, bit)) return false;
    return ((m_portDir[port] >> bit) & 1u) != 0;
}

quint8 Microcontroller::portDirectionMask(int port) const
{
    return validPort(port) ? m_portDir[port] : 0;
}

void Microcontroller::setPortDirectionMask(int port, quint8 mask)
{
    if (!validPort(port)) return;
    m_portDir[port] = mask;
}

void Microcontroller::setInputPortBit(int port, int bit, bool high)
{
    if (!validPortBit(port, bit)) return;
    const quint8 mask = quint8(1u << bit);
    if (high) m_portInput[port] |= mask;
    else      m_portInput[port] &= quint8(~mask);
}

quint8 Microcontroller::portValue(int port) const
{
    if (!validPort(port)) return 0;
    return quint8((m_portLatch[port] & m_portDir[port]) |
                  (m_portInput[port] & quint8(~m_portDir[port])));
}

void Microcontroller::tickCpu()
{
    if (m_pc >= m_flash.size()) return;

    auto fetch = [&]() -> quint8 {
        return (m_pc < m_flash.size()) ? quint8(m_flash[m_pc++]) : 0;
    };

    const quint8 op = fetch();
    switch (op) {
    case 0x00:
        break; // NOP

    case 0x10:
        m_acc = fetch(); // MOV A,#imm
        break;

    case 0x11: { // MOV [addr],A
        const quint8 a = fetch();
        if (a < m_ram.size()) m_ram[a] = char(m_acc);
        break;
    }

    case 0x12: { // MOV A,[addr]
        const quint8 a = fetch();
        if (a < m_ram.size()) m_acc = quint8(m_ram[a]);
        break;
    }

    case 0x20:
        m_acc = quint8(m_acc + fetch()); // ADD A,#imm
        break;

    case 0x30: { // JMP addr
        const quint8 lo = fetch();
        const quint8 hi = fetch();
        m_pc = quint16(lo | (quint16(hi) << 8));
        break;
    }

    case 0x40: { // SETB Pn.bit
        const quint8 pb = fetch();
        const int p = (pb >> 4) & 0x0f;
        const int b = pb & 7;
        if (validPortBit(p, b)) {
            const quint8 mask = quint8(1u << b);
            m_portDir[p] |= mask;
            m_portLatch[p] |= mask;
        }
        break;
    }

    case 0x41: { // CLR Pn.bit
        const quint8 pb = fetch();
        const int p = (pb >> 4) & 0x0f;
        const int b = pb & 7;
        if (validPortBit(p, b)) {
            const quint8 mask = quint8(1u << b);
            m_portDir[p] |= mask;
            m_portLatch[p] &= quint8(~mask);
        }
        break;
    }

    case 0x42: { // MOV Pn,A
        const int p = int(fetch());
        if (validPort(p)) {
            m_portLatch[p] = m_acc;
            m_portDir[p] = 0xff;
        }
        break;
    }

    case 0x43: { // MOV A,Pn
        const int p = int(fetch());
        if (validPort(p)) m_acc = portValue(p);
        break;
    }

    case 0x44: { // DIR Pn,#mask ; 1=output, 0=input
        const int p = int(fetch());
        const quint8 mask = fetch();
        if (validPort(p)) m_portDir[p] = mask;
        break;
    }

    case 0x45: { // MOV Pn,#imm
        const int p = int(fetch());
        const quint8 value = fetch();
        if (validPort(p)) {
            m_portLatch[p] = value;
            m_portDir[p] = 0xff;
        }
        break;
    }

    default:
        break;
    }
}

QRectF Microcontroller::boundingBox() const
{
    return QRectF(m_pos.x() - 74, m_pos.y() - 112, 148, 224);
}

void Microcontroller::draw(QPainter& painter, bool selected) const
{
    painter.save();
    painter.translate(m_pos);
    painter.rotate(m_rotation);
    painter.setPen(QPen(selected ? Qt::cyan : Qt::darkGray, 2));
    painter.setBrush(QColor(30, 30, 35));
    const QRectF body(-60, -96, 120, 192);
    painter.drawRoundedRect(body, 5, 5);

    painter.setPen(Qt::white);
    painter.setFont(QFont("Monospace", 8, QFont::Bold));
    painter.drawText(QRectF(-50, -18, 100, 36), Qt::AlignCenter,
                     QString("MCU 4x8 GPIO\n%1 B  PC=%2")
                         .arg(m_flash.size()).arg(m_pc));

    drawPinLabels(painter, body, Qt::white, 5);
    painter.setPen(QPen(Qt::red, 1));
    for (const auto& pin : m_pins) {
        painter.setBrush(pin->highlighted() ? Qt::yellow : Qt::red);
        painter.drawEllipse(pin->localPos(), Pin::HoverRadius, Pin::HoverRadius);
    }
    painter.restore();
}

QJsonObject Microcontroller::serialize() const
{
    auto o = Component::serialize();
    o["firmwarePath"] = m_firmwarePath;
    o["pc"] = int(m_pc);
    o["acc"] = int(m_acc);

    for (int p = 0; p < PortCount; ++p) {
        o[QString("p%1").arg(p)] = int(m_portLatch[p]);
        o[QString("pin%1").arg(p)] = int(m_portInput[p]);
        o[QString("dir%1").arg(p)] = int(m_portDir[p]);
    }

    QJsonArray flash;
    for (auto c : m_flash) flash.append(int(quint8(c)));
    o["flash"] = flash;
    return o;
}

void Microcontroller::deserialize(const QJsonObject& obj)
{
    Component::deserialize(obj);
    m_firmwarePath = obj["firmwarePath"].toString();
    m_pc = quint16(obj["pc"].toInt(0));
    m_acc = quint8(obj["acc"].toInt(0));

    m_portLatch.fill(0);
    m_portInput.fill(0);
    m_portDir.fill(0);

    for (int p = 0; p < PortCount; ++p) {
        m_portLatch[p] = quint8(obj[QString("p%1").arg(p)].toInt(0));
        m_portInput[p] = quint8(obj[QString("pin%1").arg(p)].toInt(0));
        // Old project files had only p0/p1 and implicitly drove every MCU pin.
        // Preserve that behavior for those two saved ports; new files persist dirN.
        const QString dirKey = QString("dir%1").arg(p);
        if (obj.contains(dirKey)) m_portDir[p] = quint8(obj[dirKey].toInt(0));
        else if (p < 2 && obj.contains(QString("p%1").arg(p))) m_portDir[p] = 0xff;
    }

    const QJsonArray flash = obj["flash"].toArray();
    m_flash.clear();
    m_flash.resize(flash.size());
    for (int i = 0; i < flash.size(); ++i)
        m_flash[i] = char(flash[i].toInt() & 0xff);
}

QMap<QString,QString> Microcontroller::properties() const
{
    QMap<QString,QString> p {
        {"label", m_label},
        {"firmwarePath", m_firmwarePath},
        {"programSize", QString::number(m_flash.size())},
        {"PC", QString::number(m_pc)},
        {"ACC", QString::number(m_acc)}
    };

    for (int port = 0; port < PortCount; ++port) {
        p[QString("P%1").arg(port)] = QString::number(portValue(port));
        p[QString("DIR%1").arg(port)] = QString("0x%1")
            .arg(portDirectionMask(port), 2, 16, QLatin1Char('0')).toUpper();
    }
    return p;
}

void Microcontroller::setProperty(const QString& key, const QString& value)
{
    if (key == "label") {
        m_label = value;
    } else if (key == "firmwarePath") {
        QString err;
        loadIntelHex(value, &err);
    } else if (key == "PC") {
        m_pc = quint16(value.toUInt());
    } else if (key == "ACC") {
        m_acc = quint8(value.toUInt());
    } else if (key.startsWith("DIR")) {
        bool ok = false;
        const int port = key.mid(3).toInt(&ok);
        if (ok && validPort(port)) {
            QString text = value.trimmed();
            int base = 10;
            if (text.startsWith("0x", Qt::CaseInsensitive)) {
                text = text.mid(2);
                base = 16;
            }
            const uint parsed = text.toUInt(&ok, base);
            if (ok) setPortDirectionMask(port, quint8(parsed & 0xffu));
        }
    }
}
