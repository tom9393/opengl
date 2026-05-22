#include "CommandHistory.h"

namespace Engine {

    void CommandHistory::Execute(Unique<Command> command)
    {
        command->Redo();
        Push(std::move(command));
    }

    void CommandHistory::Push(Unique<Command> command)
    {
        m_Redo.clear();
        m_Undo.push_back(std::move(command));

        if (m_Undo.size() > s_MaxDepth)
            m_Undo.erase(m_Undo.begin());
    }

    void CommandHistory::Undo()
    {
        if (m_Undo.empty()) return;

        Unique<Command> command = std::move(m_Undo.back());
        m_Undo.pop_back();
        command->Undo();
        m_Redo.push_back(std::move(command));
    }

    void CommandHistory::Redo()
    {
        if (m_Redo.empty()) return;

        Unique<Command> command = std::move(m_Redo.back());
        m_Redo.pop_back();
        command->Redo();
        m_Undo.push_back(std::move(command));
    }

    void CommandHistory::Clear()
    {
        m_Undo.clear();
        m_Redo.clear();
    }

    CommandHistory& CommandHistory::Global()
    {
        static CommandHistory s_Instance;
        return s_Instance;
    }

}
