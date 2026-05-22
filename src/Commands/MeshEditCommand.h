#pragma once

#include "Command.h"
#include "Modeling/MeshEditSession.h"

namespace Engine {

    // Undo/redo for a single in-session mesh edit (extrude, delete/subdivide faces,
    // vertex move, UV move). It restores the whole session geometry and re-syncs the
    // render mesh. These commands live on the session's local history, which is cleared
    // when the session ends, so the session pointer is always valid when they run.
    class MeshEditCommand : public Command {
    public:
        MeshEditCommand(MeshEditSession* session,
                        MeshEditSession::State before,
                        MeshEditSession::State after)
            : m_Session(session), m_Before(std::move(before)), m_After(std::move(after)) {}

        void Undo() override { Apply(m_Before); }
        void Redo() override { Apply(m_After); }
        const char* Name() const override { return "Mesh Edit"; }

    private:
        void Apply(const MeshEditSession::State& state)
        {
            if (!m_Session || !m_Session->IsActive()) return;
            m_Session->RestoreState(state);
            m_Session->SyncToMesh();
        }

        MeshEditSession* m_Session;
        MeshEditSession::State m_Before;
        MeshEditSession::State m_After;
    };

}
