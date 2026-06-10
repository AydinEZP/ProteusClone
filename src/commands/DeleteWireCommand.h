#pragma once
#include "BaseCommand.h"
#include <memory>
#include "../domain/Wire.h"
#include "../graph/CircuitGraph.h"

class DeleteWireCommand : public BaseCommand {
public:
    DeleteWireCommand(CircuitGraph* graph, std::shared_ptr<Wire> wire);
    void    execute()     override;
    void    undo()        override;
    QString description() const override;

private:
    CircuitGraph*          m_graph;
    std::shared_ptr<Wire>  m_wire;
};