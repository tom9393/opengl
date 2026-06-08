#pragma once

#include "ViewportPanelBase.h"
#include "Commands/MeshEditCommand.h"
#include "Modeling/MeshEditSession.h"

namespace Engine {

    class ModelingViewportPanel : public ViewportPanelBase {
    public:
        ModelingViewportPanel() = default;

        // Edit-mode lifecycle (overrides ViewportTab). Driven by the toolbar's
        // Enter/Commit buttons and by the Viewport tab change: opening the Modeling tab
        // enters edit mode, leaving it commits. Enter begins a session on the selected
        // entity (no-op if already active or nothing valid is selected); Exit commits.
        void Enter() override;
        void Exit() override;

    protected:
        void OnDraw() override;
        bool OnMousePressed(MouseButtonPressedEvent& e) override;

    private:
        enum class SelectMode { Vertex, Edge, Face };

        void DrawToolbar();
        void DrawOverlay();
        void DrawVertexOverlay(const glm::mat4& mvp);
        void DrawEdgeOverlay(const glm::mat4& mvp);
        void DrawFaceOverlay(const glm::mat4& mvp);
        void DrawGizmo();

        int PickVertex(const ImVec2& mouse);
        int PickEdge(const ImVec2& mouse);
        int PickFace(const ImVec2& mouse);

        // Vertices the active selection moves: in vertex mode the selected verts, in
        // edge mode each edge's endpoints, in face mode each face's corners (deduped).
        std::vector<int> SelectedVertexSet() const;
        bool HasSelection() const;
        void ClearSelection();

        // Project a vertex (in the edited entity's local space) to viewport screen
        // pixels using mvp. Returns false if behind the camera or far off-screen.
        bool WorldToScreen(const glm::mat4& mvp, const glm::vec3& local, ImVec2& out) const;

        // Builds projection * view * model for the active session's entity. False if
        // there is no active session or the entity lacks a transform.
        bool TryGetMVP(glm::mat4& mvp);

        // Runs a topology op (extrude/delete/subdivide) and records one undo step on the
        // global command history if it succeeds. Returns the op's success result.
        template <typename Op>
        bool RecordMeshEdit(Op&& op)
        {
            MeshEditSession::State before = m_Session.CaptureState();
            if (!op())
                return false;
            CommandHistory::Global().Push(
                MakeUnique<MeshEditCommand>(&m_Session, std::move(before), m_Session.CaptureState()));
            return true;
        }

        MeshEditSession m_Session;
        SelectMode m_SelectMode = SelectMode::Vertex;
        std::vector<int> m_SelectedVertices; // vertex indices
        std::vector<int> m_SelectedEdges;    // indices into Mesh().Edges()
        std::vector<int> m_SelectedFaces;    // face indices

        // Coalesce a vertex-gizmo drag into one undo step (snapshot at drag start,
        // record at drag end if anything actually moved).
        bool m_VertGizmoUsing = false;
        bool m_VertGizmoMoved = false;
        MeshEditSession::State m_VertGizmoBefore;
    };

}
