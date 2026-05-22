#include "ModelingViewportPanel.h"
#include "PassPanel.h"
#include "Core/Input.h"
#include "Core/KeyCodes.h"
#include "Core/MouseButtonCodes.h"
#include "Utils/SceneManager.h"
#include "Utils/GizmoManager.h"

namespace Engine {

    namespace {
        // Squared distance from point p to segment a-b, in screen pixels.
        float PointSegmentDistSq(const ImVec2& p, const ImVec2& a, const ImVec2& b) {
            float vx = b.x - a.x, vy = b.y - a.y;
            float wx = p.x - a.x, wy = p.y - a.y;
            float len2 = vx * vx + vy * vy;
            float t = len2 > 0.0f ? (wx * vx + wy * vy) / len2 : 0.0f;
            t = t < 0.0f ? 0.0f : (t > 1.0f ? 1.0f : t);
            float dx = p.x - (a.x + t * vx);
            float dy = p.y - (a.y + t * vy);
            return dx * dx + dy * dy;
        }
    }

    void ModelingViewportPanel::OnDraw() {
        if (m_Session.IsActive()) {
            DrawOverlay();
            DrawGizmo();
        }
        DrawToolbar();
    }

    void ModelingViewportPanel::DrawToolbar() {
        ImGui::SetCursorScreenPos(ImVec2(m_ViewportBounds[0].x + 8.0f, m_ViewportBounds[0].y + 8.0f));

        if (!m_Session.IsActive()) {
            if (ImGui::Button("Enter Edit Mode")) {
                Entity selected = SceneManager::GetSelectedEntity();
                if (m_Session.Begin(selected))
                    ClearSelection();
            }
            return;
        }

        if (ImGui::Button("Commit")) {
            m_Session.End(true);
            ClearSelection();
            return;
        }
        ImGui::SameLine();
        if (ImGui::Button("Discard")) {
            m_Session.End(false);
            ClearSelection();
            return;
        }

        // Switching the select mode clears the previous mode's selection (the element
        // types don't map onto each other in v1).
        auto modeButton = [&](const char* label, SelectMode mode) {
            bool active = (m_SelectMode == mode);
            if (active)
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.90f, 0.55f, 0.0f, 1.0f));
            if (ImGui::Button(label) && !active) {
                m_SelectMode = mode;
                ClearSelection();
            }
            if (active)
                ImGui::PopStyleColor();
        };

        ImGui::SameLine();
        ImGui::Dummy(ImVec2(12.0f, 0.0f));
        ImGui::SameLine();
        modeButton("Vertex", SelectMode::Vertex);
        ImGui::SameLine();
        modeButton("Edge", SelectMode::Edge);
        ImGui::SameLine();
        modeButton("Face", SelectMode::Face);

        int selCount = m_SelectMode == SelectMode::Vertex ? (int)m_SelectedVertices.size()
                     : m_SelectMode == SelectMode::Edge   ? (int)m_SelectedEdges.size()
                     :                                       (int)m_SelectedFaces.size();
        ImGui::SameLine();
        ImGui::Text("Selected: %d", selCount);

        // Topology ops act on selected faces (v1). Extrude keeps the lifted caps
        // selected so the gizmo can pull them out; delete/subdivide reindex faces.
        if (m_SelectMode == SelectMode::Face && !m_SelectedFaces.empty()) {
            ImGui::SameLine();
            ImGui::Dummy(ImVec2(12.0f, 0.0f));
            ImGui::SameLine();
            if (ImGui::Button("Extrude"))
                RecordMeshEdit([&]{ return m_Session.ExtrudeFaces(m_SelectedFaces); });
            ImGui::SameLine();
            if (ImGui::Button("Delete") && RecordMeshEdit([&]{ return m_Session.DeleteFaces(m_SelectedFaces); }))
                ClearSelection();
            ImGui::SameLine();
            if (ImGui::Button("Subdivide") && RecordMeshEdit([&]{ return m_Session.SubdivideFaces(m_SelectedFaces); }))
                ClearSelection();
        }
    }

    bool ModelingViewportPanel::TryGetMVP(glm::mat4& mvp) {
        if (!m_Session.IsActive() || !m_EditorCamera)
            return false;

        Entity entity = m_Session.GetEntity();
        if (!entity || !entity.HasComponent<TransformComponent>())
            return false;

        glm::mat4 model = entity.GetComponent<TransformComponent>().GetTransform();
        mvp = m_EditorCamera->GetProjection() * m_EditorCamera->GetViewMatrix() * model;
        return true;
    }

    bool ModelingViewportPanel::WorldToScreen(const glm::mat4& mvp, const glm::vec3& local, ImVec2& out) const {
        glm::vec4 clip = mvp * glm::vec4(local, 1.0f);
        if (clip.w <= 0.0f)
            return false;

        glm::vec3 ndc = glm::vec3(clip) / clip.w;
        if (ndc.x < -1.3f || ndc.x > 1.3f || ndc.y < -1.3f || ndc.y > 1.3f)
            return false;

        out.x = m_ViewportBounds[0].x + (ndc.x * 0.5f + 0.5f) * m_ViewportSize.x;
        out.y = m_ViewportBounds[0].y + (1.0f - (ndc.y * 0.5f + 0.5f)) * m_ViewportSize.y;
        return true;
    }

    void ModelingViewportPanel::DrawOverlay() {
        glm::mat4 mvp;
        if (!TryGetMVP(mvp))
            return;

        // The edge wireframe is always drawn for shape context; verts/faces overlay on
        // top only in their respective modes.
        DrawEdgeOverlay(mvp);
        if (m_SelectMode == SelectMode::Vertex)
            DrawVertexOverlay(mvp);
        else if (m_SelectMode == SelectMode::Face)
            DrawFaceOverlay(mvp);
    }

    void ModelingViewportPanel::DrawVertexOverlay(const glm::mat4& mvp) {
        const HalfEdgeMesh& mesh = m_Session.Mesh();
        int vCount = (int)mesh.Vertices().size();

        std::vector<bool> selected(vCount, false);
        for (int v : m_SelectedVertices)
            if (v >= 0 && v < vCount)
                selected[v] = true;

        ImDrawList* draw = ImGui::GetWindowDrawList();
        for (int v = 0; v < vCount; ++v) {
            ImVec2 s;
            if (!WorldToScreen(mvp, mesh.Position(v), s))
                continue;

            if (selected[v]) {
                draw->AddCircleFilled(s, 5.0f, IM_COL32(255, 160, 0, 255));
            } else {
                draw->AddCircleFilled(s, 3.5f, IM_COL32(220, 220, 220, 255));
            }
            draw->AddCircle(s, selected[v] ? 5.0f : 3.5f, IM_COL32(0, 0, 0, 180), 0, 1.0f);
        }
    }

    void ModelingViewportPanel::DrawEdgeOverlay(const glm::mat4& mvp) {
        const HalfEdgeMesh& mesh = m_Session.Mesh();
        std::vector<std::pair<int, int>> edges = mesh.Edges();

        std::vector<bool> selected(edges.size(), false);
        if (m_SelectMode == SelectMode::Edge)
            for (int e : m_SelectedEdges)
                if (e >= 0 && e < (int)edges.size())
                    selected[e] = true;

        ImDrawList* draw = ImGui::GetWindowDrawList();
        for (int e = 0; e < (int)edges.size(); ++e) {
            ImVec2 a, b;
            if (!WorldToScreen(mvp, mesh.Position(edges[e].first), a))
                continue;
            if (!WorldToScreen(mvp, mesh.Position(edges[e].second), b))
                continue;

            if (selected[e])
                draw->AddLine(a, b, IM_COL32(255, 160, 0, 255), 2.5f);
            else
                draw->AddLine(a, b, IM_COL32(0, 0, 0, 160), 1.0f);
        }
    }

    void ModelingViewportPanel::DrawFaceOverlay(const glm::mat4& mvp) {
        const HalfEdgeMesh& mesh = m_Session.Mesh();
        int fCount = (int)mesh.Faces().size();

        std::vector<bool> selected(fCount, false);
        for (int f : m_SelectedFaces)
            if (f >= 0 && f < fCount)
                selected[f] = true;

        ImDrawList* draw = ImGui::GetWindowDrawList();
        for (int f = 0; f < fCount; ++f) {
            // Translucent fill marks selected faces (convex fill: fine for tris/quads).
            if (selected[f]) {
                std::vector<int> fv = mesh.FaceVertices(f);
                std::vector<ImVec2> pts;
                bool ok = true;
                for (int v : fv) {
                    ImVec2 s;
                    if (!WorldToScreen(mvp, mesh.Position(v), s)) { ok = false; break; }
                    pts.push_back(s);
                }
                if (ok && pts.size() >= 3)
                    draw->AddConvexPolyFilled(pts.data(), (int)pts.size(), IM_COL32(255, 160, 0, 80));
            }

            // Centroid dot is the click target for the face.
            ImVec2 c;
            if (!WorldToScreen(mvp, mesh.FaceCentroid(f), c))
                continue;
            draw->AddCircleFilled(c, 3.5f, selected[f] ? IM_COL32(255, 160, 0, 255)
                                                       : IM_COL32(220, 220, 220, 230));
            draw->AddCircle(c, 3.5f, IM_COL32(0, 0, 0, 180), 0, 1.0f);
        }
    }

    void ModelingViewportPanel::DrawGizmo() {
        if (!HasSelection() || !m_EditorCamera)
            return;

        Entity entity = m_Session.GetEntity();
        if (!entity || !entity.HasComponent<TransformComponent>())
            return;

        std::vector<int> verts = SelectedVertexSet();
        if (verts.empty())
            return;

        HalfEdgeMesh& mesh = m_Session.Mesh();

        // World-space centroid of the moved vertices drives the gizmo position.
        glm::vec3 localCentroid(0.0f);
        for (int v : verts)
            localCentroid += mesh.Position(v);
        localCentroid /= (float)verts.size();

        glm::mat4 model = entity.GetComponent<TransformComponent>().GetTransform();
        glm::vec3 worldCentroid = glm::vec3(model * glm::vec4(localCentroid, 1.0f));
        glm::mat4 gizmoMatrix = glm::translate(glm::mat4(1.0f), worldCentroid);

        GizmoManager::BeginFrame();
        GizmoManager::SetViewport(m_ViewportBounds[0], m_ViewportBounds[1] - m_ViewportBounds[0]);
        GizmoManager::SetOperation(ImGuizmo::OPERATION::TRANSLATE);

        bool snap = Input::IsKeyPressed(KEY_LEFT_SHIFT);
        GizmoResult result = GizmoManager::Manipulate(*m_EditorCamera, gizmoMatrix, snap);

        if (result.IsUsing && !m_VertGizmoUsing) {
            m_VertGizmoBefore = m_Session.CaptureState();
            m_VertGizmoMoved = false;
        }

        if (result.IsUsing) {
            glm::vec3 deltaWorld = glm::vec3(result.Transform[3]) - worldCentroid;
            if (deltaWorld != glm::vec3(0.0f)) {
                // A world-space translation maps to a local displacement through the
                // inverse model (handles the entity's rotation/scale).
                glm::vec3 deltaLocal = glm::vec3(glm::inverse(model) * glm::vec4(deltaWorld, 0.0f));
                for (int v : verts)
                    mesh.Position(v) += deltaLocal;
                m_Session.SyncToMesh();
                m_VertGizmoMoved = true;
            }
        }

        if (!result.IsUsing && m_VertGizmoUsing && m_VertGizmoMoved) {
            m_Session.History().Push(
                MakeUnique<MeshEditCommand>(&m_Session, m_VertGizmoBefore, m_Session.CaptureState()));
        }

        m_VertGizmoUsing = result.IsUsing;
    }

    int ModelingViewportPanel::PickVertex(const ImVec2& mouse) {
        glm::mat4 mvp;
        if (!TryGetMVP(mvp))
            return -1;

        const HalfEdgeMesh& mesh = m_Session.Mesh();
        const float threshold = 10.0f;
        float bestDistSq = threshold * threshold;
        int best = -1;

        for (int v = 0; v < (int)mesh.Vertices().size(); ++v) {
            ImVec2 s;
            if (!WorldToScreen(mvp, mesh.Position(v), s))
                continue;

            float dx = s.x - mouse.x;
            float dy = s.y - mouse.y;
            float distSq = dx * dx + dy * dy;
            if (distSq < bestDistSq) {
                bestDistSq = distSq;
                best = v;
            }
        }
        return best;
    }

    int ModelingViewportPanel::PickEdge(const ImVec2& mouse) {
        glm::mat4 mvp;
        if (!TryGetMVP(mvp))
            return -1;

        const HalfEdgeMesh& mesh = m_Session.Mesh();
        std::vector<std::pair<int, int>> edges = mesh.Edges();
        const float threshold = 8.0f;
        float bestDistSq = threshold * threshold;
        int best = -1;

        for (int e = 0; e < (int)edges.size(); ++e) {
            ImVec2 a, b;
            if (!WorldToScreen(mvp, mesh.Position(edges[e].first), a))
                continue;
            if (!WorldToScreen(mvp, mesh.Position(edges[e].second), b))
                continue;

            float distSq = PointSegmentDistSq(mouse, a, b);
            if (distSq < bestDistSq) {
                bestDistSq = distSq;
                best = e;
            }
        }
        return best;
    }

    int ModelingViewportPanel::PickFace(const ImVec2& mouse) {
        glm::mat4 mvp;
        if (!TryGetMVP(mvp))
            return -1;

        const HalfEdgeMesh& mesh = m_Session.Mesh();
        const float threshold = 10.0f;
        float bestDistSq = threshold * threshold;
        int best = -1;

        for (int f = 0; f < (int)mesh.Faces().size(); ++f) {
            ImVec2 c;
            if (!WorldToScreen(mvp, mesh.FaceCentroid(f), c))
                continue;

            float dx = c.x - mouse.x;
            float dy = c.y - mouse.y;
            float distSq = dx * dx + dy * dy;
            if (distSq < bestDistSq) {
                bestDistSq = distSq;
                best = f;
            }
        }
        return best;
    }

    std::vector<int> ModelingViewportPanel::SelectedVertexSet() const {
        const HalfEdgeMesh& mesh = m_Session.Mesh();
        int vCount = (int)mesh.Vertices().size();

        std::vector<bool> seen(vCount, false);
        std::vector<int> result;
        auto add = [&](int v) {
            if (v >= 0 && v < vCount && !seen[v]) {
                seen[v] = true;
                result.push_back(v);
            }
        };

        switch (m_SelectMode) {
            case SelectMode::Vertex:
                for (int v : m_SelectedVertices)
                    add(v);
                break;
            case SelectMode::Edge: {
                std::vector<std::pair<int, int>> edges = mesh.Edges();
                for (int e : m_SelectedEdges)
                    if (e >= 0 && e < (int)edges.size()) {
                        add(edges[e].first);
                        add(edges[e].second);
                    }
                break;
            }
            case SelectMode::Face:
                for (int f : m_SelectedFaces)
                    for (int v : mesh.FaceVertices(f))
                        add(v);
                break;
        }
        return result;
    }

    bool ModelingViewportPanel::HasSelection() const {
        switch (m_SelectMode) {
            case SelectMode::Vertex: return !m_SelectedVertices.empty();
            case SelectMode::Edge:   return !m_SelectedEdges.empty();
            case SelectMode::Face:   return !m_SelectedFaces.empty();
        }
        return false;
    }

    void ModelingViewportPanel::ClearSelection() {
        m_SelectedVertices.clear();
        m_SelectedEdges.clear();
        m_SelectedFaces.clear();
    }

    bool ModelingViewportPanel::OnMousePressed(MouseButtonPressedEvent& e) {
        if (!m_Session.IsActive())
            return false;
        if (e.GetMouseButton() != MOUSE_BUTTON_LEFT || !IsHovered())
            return false;
        // The toolbar buttons overlay the viewport, but IsHovered() is sampled on the
        // image before they are submitted, so it reports hovered even over them. Without
        // this guard a click on (e.g.) Extrude would also run a pick that clears the
        // selection before the button's command fires.
        if (ImGui::IsAnyItemHovered())
            return false;
        // When a gizmo is shown, clicking it should drag, not reselect.
        if (HasSelection() && ImGuizmo::IsOver())
            return false;

        ImVec2 mouse = ImGui::GetMousePos();
        switch (m_SelectMode) {
            case SelectMode::Vertex: {
                int v = PickVertex(mouse);
                m_SelectedVertices.clear();
                if (v >= 0)
                    m_SelectedVertices.push_back(v);
                break;
            }
            case SelectMode::Edge: {
                int ed = PickEdge(mouse);
                m_SelectedEdges.clear();
                if (ed >= 0)
                    m_SelectedEdges.push_back(ed);
                break;
            }
            case SelectMode::Face: {
                int f = PickFace(mouse);
                m_SelectedFaces.clear();
                if (f >= 0)
                    m_SelectedFaces.push_back(f);
                break;
            }
        }
        return false;
    }

}
