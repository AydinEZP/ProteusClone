#include "ProjectSerializer.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QSizeF>
#include "../domain/components/Resistor.h"
#include "../domain/components/Capacitor.h"
#include "../domain/components/Inductor.h"
#include "../domain/components/DCVoltageSource.h"
#include "../domain/components/Battery.h"
#include "../domain/components/Ground.h"
#include "../domain/components/ClockGenerator.h"
#include "../domain/components/Switch.h"
#include "../domain/components/PushButton.h"
#include "../domain/components/LED.h"
#include "../domain/components/SevenSegment.h"
#include "../domain/components/AndGate.h"
#include "../domain/components/OrGate.h"
#include "../domain/components/NotGate.h"
#include "../domain/components/XorGate.h"
#include "../domain/components/NandGate.h"
#include "../domain/components/DFlipFlop.h"
#include "../domain/components/SimpleADC.h"
#include "../domain/components/SimpleDAC.h"
#include "../domain/components/LCD16x2.h"
#include "../domain/components/Keypad.h"
#include "../domain/components/VoltageProbe.h"
#include "../domain/components/Voltmeter.h"
#include "../domain/components/Ammeter.h"
#include "../domain/components/Oscilloscope.h"
#include "../domain/components/Microcontroller.h"
#include "../domain/components/Potentiometer.h"
#include "../domain/components/ExternalMemory.h"

#include <QIODevice>
QString ProjectSerializer::save(const CircuitGraph& graph, const QString& filePath)
{
    QJsonObject root;
    root["version"] = 2;
    QJsonObject canvas;
    canvas["width"] = graph.canvasSize().width();
    canvas["height"] = graph.canvasSize().height();
    root["canvas"] = canvas;

    // Components
    QJsonArray compsArr;
    for (auto& comp : graph.components())
        compsArr.append(comp->serialize());
    root["components"] = compsArr;

    // Wires (include pin references by component id + pin name)
    QJsonArray wiresArr;
    for (auto& wire : graph.wires()) {
        QJsonObject wObj = wire->serialize();
        if (wire->startPin() && wire->endPin()) {
            // Find owning components for pin references
            for (auto& comp : graph.components()) {
                for (auto& pin : comp->pins()) {
                    if (pin == wire->startPin()) {
                        wObj["startCompId"]  = static_cast<qint64>(comp->id());
                        wObj["startPinName"] = pin->name();
                    }
                    if (pin == wire->endPin()) {
                        wObj["endCompId"]  = static_cast<qint64>(comp->id());
                        wObj["endPinName"] = pin->name();
                    }
                }
            }
        }
        wiresArr.append(wObj);
    }
    root["wires"] = wiresArr;

    // Junctions
    QJsonArray juncArr;
    for (auto& j : graph.junctions())
        juncArr.append(j->serialize());
    root["junctions"] = juncArr;

    QJsonDocument doc(root);
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return QString("Cannot open file for writing: %1").arg(filePath);
    file.write(doc.toJson());
    return {};
}

QString ProjectSerializer::load(CircuitGraph& graph, const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return QString("Cannot open file: %1").arg(filePath);

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &err);
    if (err.error != QJsonParseError::NoError)
        return QString("JSON parse error: %1").arg(err.errorString());

    graph.clear();
    QJsonObject root = doc.object();
    QJsonObject canvas = root["canvas"].toObject();
    if (!canvas.isEmpty()) graph.setCanvasSize(QSizeF(canvas["width"].toDouble(1600), canvas["height"].toDouble(1000)));

    // Components
    for (auto v : root["components"].toArray()) {
        QJsonObject obj = v.toObject();
        auto comp = createComponent(obj["type"].toString());
        if (!comp) continue;
        comp->deserialize(obj);
        graph.addComponent(comp);
    }

    // Wires
    for (auto v : root["wires"].toArray()) {
        QJsonObject wObj = v.toObject();
        auto wire = std::make_shared<Wire>();
        wire->deserialize(wObj);

        // Reconnect pins
        ComponentID startCid = static_cast<ComponentID>(wObj["startCompId"].toInteger());
        ComponentID endCid   = static_cast<ComponentID>(wObj["endCompId"].toInteger());
        QString startPN = wObj["startPinName"].toString();
        QString endPN   = wObj["endPinName"].toString();

        auto startComp = graph.componentById(startCid);
        auto endComp   = graph.componentById(endCid);
        if (startComp && endComp) {
            // Use the existing wire struct but set pin refs via a convenience constructor
            auto startPin = startComp->pinByName(startPN);
            auto endPin   = endComp->pinByName(endPN);
            auto fullWire = std::make_shared<Wire>(startPin, endPin);
            fullWire->setPath(wire->path()); // keep serialized path
            fullWire->setManualRoute(wObj["manualRoute"].toBool(wire->manualRoute()));
            graph.addWire(fullWire);
        } else {
            graph.addWire(wire);
        }
    }

    // Junctions
    for (auto v : root["junctions"].toArray()) {
        auto j = std::make_shared<Junction>();
        j->deserialize(v.toObject());
        graph.addJunction(j);
    }

    return {};
}

std::shared_ptr<Component> ProjectSerializer::createComponent(const QString& type)
{
    if (type == "Resistor")        return std::make_shared<Resistor>();
    if (type == "Capacitor")       return std::make_shared<Capacitor>();
    if (type == "Inductor")        return std::make_shared<Inductor>();
    if (type == "Potentiometer")   return std::make_shared<Potentiometer>();
    if (type == "DCVoltageSource") return std::make_shared<DCVoltageSource>();
    if (type == "Battery")         return std::make_shared<Battery>();
    if (type == "Ground")          return std::make_shared<Ground>();
    if (type == "ClockGenerator")  return std::make_shared<ClockGenerator>();
    if (type == "Switch")          return std::make_shared<Switch>();
    if (type == "PushButton")      return std::make_shared<PushButton>();
    if (type == "LED")             return std::make_shared<LED>();
    if (type == "SevenSegment")    return std::make_shared<SevenSegment>();
    if (type == "AndGate")         return std::make_shared<AndGate>();
    if (type == "OrGate")          return std::make_shared<OrGate>();
    if (type == "NotGate")         return std::make_shared<NotGate>();
    if (type == "XorGate")         return std::make_shared<XorGate>();
    if (type == "NandGate")        return std::make_shared<NandGate>();
    if (type == "DFlipFlop")       return std::make_shared<DFlipFlop>();
    if (type == "SimpleADC")       return std::make_shared<SimpleADC>();
    if (type == "SimpleDAC")       return std::make_shared<SimpleDAC>();
    if (type == "LCD16x2")         return std::make_shared<LCD16x2>();
    if (type == "Keypad")          return std::make_shared<Keypad>();
    if (type == "VoltageProbe")    return std::make_shared<VoltageProbe>();
    if (type == "Voltmeter")       return std::make_shared<Voltmeter>();
    if (type == "Ammeter")         return std::make_shared<Ammeter>();
    if (type == "Oscilloscope")    return std::make_shared<Oscilloscope>();
    if (type == "Microcontroller") return std::make_shared<Microcontroller>();
    if (type == "ExternalMemory")  return std::make_shared<ExternalMemory>();
    return nullptr;
}
