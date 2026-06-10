#pragma once
#include <memory>
#include <vector>
#include "BaseCommand.h"

/**
 * Manages two stacks for undo/redo.
 * Calling execute() on a new command clears the redo stack.
 */
class CommandManager {
public:
    void execute(std::unique_ptr<BaseCommand> cmd);
    void undo();
    void redo();

    bool canUndo() const { return !m_undoStack.empty(); }
    bool canRedo() const { return !m_redoStack.empty(); }

    QString undoDescription() const;
    QString redoDescription() const;

private:
    std::vector<std::unique_ptr<BaseCommand>> m_undoStack;
    std::vector<std::unique_ptr<BaseCommand>> m_redoStack;
};