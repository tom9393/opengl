#pragma once

namespace Engine {

    // Undoable editor action using command pattern
    class Command {
    public:
        virtual ~Command() = default;

        virtual void Undo() = 0;
        virtual void Redo() = 0;

        virtual const char* Name() const { return "Command"; }
    };

}
