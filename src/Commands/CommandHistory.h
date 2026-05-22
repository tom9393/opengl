#pragma once

#include "pch.h"
#include "Command.h"

namespace Engine {

    // Undo/redo stacks of Commands. There is one Global() instance for scene editing;
    // the mesh-edit session owns a separate instance so edit-mode undo stays local.
    class CommandHistory {
    public:
        // Apply a freshly-built command (calls Redo) and record it. Use for actions
        // that have not been performed yet.
        void Execute(Unique<Command> command);

        // Record a command whose effect has already been applied live (e.g. an
        // ImGui drag the user just finished). Does not call Redo.
        void Push(Unique<Command> command);

        void Undo();
        void Redo();
        void Clear();

        bool CanUndo() const { return !m_Undo.empty(); }
        bool CanRedo() const { return !m_Redo.empty(); }

        static CommandHistory& Global();

    private:
        // Oldest commands are dropped once the undo stack exceeds this depth.
        static constexpr size_t s_MaxDepth = 100;

        std::vector<Unique<Command>> m_Undo;
        std::vector<Unique<Command>> m_Redo;
    };

}
