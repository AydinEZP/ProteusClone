#include "AddWireCommand.h"

#include <utility>
AddWireCommand::AddWireCommand(CircuitGraph* graph, std::shared_ptr<Wire> wire)
    : m_graph(graph), m_wire(std::move(wire))
{}

void AddWireCommand::execute() { m_graph->addWire(m_wire); }
void AddWireCommand::undo()    { m_graph->removeWire(m_wire->id()); }
QString AddWireCommand::description() const { return "Add wire"; }
