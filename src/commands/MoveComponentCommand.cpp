#include "MoveComponentCommand.h"

MoveComponentCommand::MoveComponentCommand(CircuitGraph* graph, ComponentID id,
                                           QPointF oldPos, QPointF newPos)
    : m_graph(graph), m_id(id), m_oldPos(oldPos), m_newPos(newPos)
{}

void MoveComponentCommand::execute() {
    // Skip the very first call: the component is already at m_newPos
    // (it was moved live during drag). Only re-apply on redo.
    if (m_firstExecute) {
        m_firstExecute = false;
        return;
    }
    if (auto c = m_graph->componentById(m_id)) {
        c->moveTo(m_newPos);
        for (auto& wire : m_graph->wires()) wire->routeSimple();
    }
}

void MoveComponentCommand::undo() {
    m_firstExecute = false; // after undo, redo must re-apply
    if (auto c = m_graph->componentById(m_id)) {
        c->moveTo(m_oldPos);
        for (auto& wire : m_graph->wires()) wire->routeSimple();
    }
}

QString MoveComponentCommand::description() const { return "Move component"; }
