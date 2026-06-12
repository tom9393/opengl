#pragma once

#include "pch.h"
#include "Command.h"

namespace Engine {

    // Undo/redo stacks of Commands
    // Scene editing -> SceneCommands
    // Mesh/UV editing -> MeshEditCommand & MaterialEditCommand
    class CommandHistory {
    public:
        // Fresh command
        void Execute(Unique<Command> command);

        // Push fresh command onto the stack; wipe m_Redo
        void Push(Unique<Command> command);

        void Undo();
        void Redo();
        void Clear();

        bool CanUndo() const { return !m_Undo.empty(); }
        bool CanRedo() const { return !m_Redo.empty(); }

        static CommandHistory& Global();

    private:
        static constexpr size_t s_MaxDepth = 100;

        std::vector<Unique<Command>> m_Undo;
        std::vector<Unique<Command>> m_Redo;
    };

}
