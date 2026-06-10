#pragma once
#include "BaseCommand.h"
#include <memory>
#include "../graph/CircuitGraph.h"
#include "../domain/Component.h"

class AddComponentCommand : public BaseCommand {
public:
    AddComponentCommand(CircuitGraph* graph, std::shared_ptr<Component> comp);
    void    execute()      override;
    void    undo()         override;
    QString description()  const override;

private:
    CircuitGraph*              m_graph;
    std::shared_ptr<Component> m_comp;
};