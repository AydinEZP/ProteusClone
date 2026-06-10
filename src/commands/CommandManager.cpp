#include "CommandManager.h"

void CommandManager::execute(std::unique_ptr<BaseCommand> cmd)
{
    cmd->execute();
    m_undoStack.push_back(std::move(cmd));
    m_redoStack.clear();
}

void CommandManager::undo()
{
    if (m_undoStack.empty()) return;
    auto& cmd = m_undoStack.back();
    cmd->undo();
    m_redoStack.push_back(std::move(cmd));
    m_undoStack.pop_back();
}

void CommandManager::redo()
{
    if (m_redoStack.empty()) return;
    auto& cmd = m_redoStack.back();
    cmd->execute();
    m_undoStack.push_back(std::move(cmd));
    m_redoStack.pop_back();
}

QString CommandManager::undoDescription() const
{
    return m_undoStack.empty() ? QString() : m_undoStack.back()->description();
}

QString CommandManager::redoDescription() const
{
    return m_redoStack.empty() ? QString() : m_redoStack.back()->description();
}