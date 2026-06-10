#pragma once
#include "BaseCommand.h"
#include <memory>
#include "../domain/Component.h"
#include "../graph/CircuitGraph.h"

class DeleteComponentCommand : public BaseCommand {
public:
    DeleteComponentCommand(CircuitGraph* graph, std::shared_ptr<Component> comp);
    void    execute()     override;
    void    undo()        override;
    QString description() const override;

private:
    CircuitGraph*              m_graph;
    std::shared_ptr<Component> m_comp;
};