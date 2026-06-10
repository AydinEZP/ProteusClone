#include "DeleteWireCommand.h"

#include <utility>
DeleteWireCommand::DeleteWireCommand(CircuitGraph* graph, std::shared_ptr<Wire> wire)
    : m_graph(graph), m_wire(std::move(wire))
{}

void DeleteWireCommand::execute() { m_graph->removeWire(m_wire->id()); }
void DeleteWireCommand::undo()    { m_graph->addWire(m_wire); }
QString DeleteWireCommand::description() const { return "Delete wire"; }
