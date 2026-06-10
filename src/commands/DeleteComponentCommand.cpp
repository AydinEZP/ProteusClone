#include "DeleteComponentCommand.h"

#include <utility>
DeleteComponentCommand::DeleteComponentCommand(CircuitGraph* graph, std::shared_ptr<Component> comp)
    : m_graph(graph), m_comp(std::move(comp))
{}

void DeleteComponentCommand::execute() { m_graph->removeComponent(m_comp->id()); }
void DeleteComponentCommand::undo()    { m_graph->addComponent(m_comp); }
QString DeleteComponentCommand::description() const {
    return QString("Delete %1").arg(m_comp->type());
}
