#pragma once

#include "pch.h"
#include "ViewportTab.h"
#include "Modeling/MeshEditSession.h"

namespace Engine {

    // 2D UV map editor (Blender-style). Owns its own MeshEditSession and edits its
    // per-corner UV layer: it draws the UV layout over a 0..1 grid (optionally with the
    // mesh's albedo texture behind it) and lets the user select UV verts and
    // move/rotate/scale them. Edits write straight to the session's UV verts and re-sync
    // the render mesh. The session begins on the selected entity when the UV tab is
    // opened and commits when it is left; a hint is shown if no session is active (e.g.
    // nothing selected, or the mesh could not be edited).
    class UVViewportPanel : public ViewportTab {
    public:
        UVViewportPanel() = default;

        // Edit-mode lifecycle (overrides ViewportTab), driven by the Viewport tab change:
        // opening the UV tab enters edit mode, leaving it commits. Enter begins a session
        // on the selected entity (no-op if already active or nothing valid is selected);
        // Exit commits.
        void Enter() override;
        void Exit() override;

        void OnImGuiRender() override;

    private:
        enum class Tool { Move, Rotate, Scale };
        enum class Drag { None, Transform, Box, Pan };

        void DrawToolbar();
        void HandleInput(const ImVec2& canvasMin, bool hovered);
        void Draw(const ImVec2& canvasMin, const ImVec2& canvasMax);

        void FitView(const ImVec2& canvasSize);
        void ZoomAt(const glm::vec2& mouse, const ImVec2& canvasMin, float wheel);

        // The view maps UV (0,0) to the bottom-left and (1,1) to the top-right.
        ImVec2 UVToScreen(const glm::vec2& uv, const ImVec2& canvasMin) const;
        glm::vec2 ScreenToUV(const glm::vec2& s, const ImVec2& canvasMin) const;

        int PickVert(const glm::vec2& mouse, const ImVec2& canvasMin) const;
        std::vector<bool> ReferencedVerts() const; // UV verts used by at least one corner
        void SelectAll();

        void BeginTransform(const glm::vec2& mouse);
        void ApplyTransform(const glm::vec2& mouse, const ImVec2& canvasMin);
        void FinalizeBox(const ImVec2& canvasMin);

        unsigned int AlbedoTextureID() const;

        MeshEditSession m_Session;

        // screen = canvasMin + m_Pan + (uv.x, 1 - uv.y) * m_Zoom
        glm::vec2 m_Pan = { 0.0f, 0.0f }; // pixels, relative to the canvas top-left
        float m_Zoom = 256.0f;            // pixels per UV unit
        bool m_ViewInitialized = false;

        Tool m_Tool = Tool::Move;
        bool m_ShowTexture = true;

        std::vector<int> m_Selected; // indices into the session's UV verts

        Drag m_Drag = Drag::None;
        glm::vec2 m_DragStartMouse = { 0.0f, 0.0f };
        glm::vec2 m_DragPivotUV = { 0.0f, 0.0f };
        std::vector<glm::vec2> m_DragStartUVs; // selected UVs at drag start, parallel to m_Selected
        glm::vec2 m_BoxStart = { 0.0f, 0.0f };
        glm::vec2 m_BoxEnd = { 0.0f, 0.0f };

        // Session geometry snapshot at the start of a UV transform drag; recorded as one
        // undo step (on the global command history) when the drag ends, if it moved anything.
        MeshEditSession::State m_TransformBefore;
        bool m_TransformMoved = false;
    };

}
