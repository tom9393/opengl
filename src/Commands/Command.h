#pragma once

namespace Engine {

    // An undoable editor action. Undo() reverts the action, Redo() (re)applies it.
    // A command must be self-contained: it resolves whatever it needs (entities by
    // UUID, the active scene, etc.) at Undo/Redo time, not by holding raw handles.
    class Command {
    public:
        virtual ~Command() = default;

        virtual void Undo() = 0;
        virtual void Redo() = 0;

        virtual const char* Name() const { return "Command"; }
    };

}
