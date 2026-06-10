#pragma once
#include "BaseCommand.h"
#include <QPointF>
#include <memory>
#include "../domain/Component.h"
#include "../graph/CircuitGraph.h"

/**
 * Records a completed component move for undo/redo.
 * 
 * IMPORTANT: execute() is called by CommandManager immediately.
 * During drag-move, the component is ALREADY at newPos when we create
 * this command. So execute() must NOT re-apply the move (it would
 * cause a visual jump). We use m_firstExecute to skip the first call.
 */
class MoveComponentCommand : public BaseCommand {
public:
    MoveComponentCommand(CircuitGraph* graph, ComponentID id,
                         QPointF oldPos, QPointF newPos);
    void    execute()     override;
    void    undo()        override;
    QString description() const override;

private:
    CircuitGraph* m_graph;
    ComponentID   m_id;
    QPointF       m_oldPos, m_newPos;
    bool          m_firstExecute {true}; // skip re-applying on first call
};
