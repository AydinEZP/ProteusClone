#include "AddComponentCommand.h"

#include <utility>
AddComponentCommand::AddComponentCommand(CircuitGraph* graph, std::shared_ptr<Component> comp)
    : m_graph(graph), m_comp(std::move(comp))
{}

void AddComponentCommand::execute() { m_graph->addComponent(m_comp); }
void AddComponentCommand::undo()    { m_graph->removeComponent(m_comp->id()); }
QString AddComponentCommand::description() const {
    return QString("Add %1").arg(m_comp->type());
}
