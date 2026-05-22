#include "UVViewportPanel.h"
#include "Asset/AssetManager.h"
#include "Asset/Material.h"
#include "Asset/Texture.h"
#include "Commands/MeshEditCommand.h"
#include "Scene/Components.h"

namespace Engine {

    void UVViewportPanel::OnImGuiRender() {
        if (!m_Session || !m_Session->IsActive()) {
            // No edit session: drop any stale state and re-fit when one next begins.
            m_Selected.clear();
            m_Drag = Drag::None;
            m_ViewInitialized = false;
            ImGui::TextDisabled("Enter Edit Mode in the Modeling tab to edit UVs.");
            return;
        }

        DrawToolbar();

        ImVec2 canvasMin = ImGui::GetCursorScreenPos();
        ImVec2 canvasSize = ImGui::GetContentRegionAvail();
        if (canvasSize.x < 16.0f) canvasSize.x = 16.0f;
        if (canvasSize.y < 16.0f) canvasSize.y = 16.0f;
        ImVec2 canvasMax(canvasMin.x + canvasSize.x, canvasMin.y + canvasSize.y);

        ImGui::InvisibleButton("uv_canvas", canvasSize,
            ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonMiddle);
        bool hovered = ImGui::IsItemHovered();

        if (!m_ViewInitialized) {
            FitView(canvasSize);
            m_ViewInitialized = true;
        }

        HandleInput(canvasMin, hovered);
        Draw(canvasMin, canvasMax);
    }

    void UVViewportPanel::DrawToolbar() {
        auto toolButton = [&](const char* label, Tool t) {
            bool active = (m_Tool == t);
            if (active)
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.90f, 0.55f, 0.0f, 1.0f));
            if (ImGui::Button(label))
                m_Tool = t;
            if (active)
                ImGui::PopStyleColor();
            ImGui::SameLine();
        };

        toolButton("Move", Tool::Move);
        toolButton("Rotate", Tool::Rotate);
        toolButton("Scale", Tool::Scale);

        ImGui::Dummy(ImVec2(12.0f, 0.0f));
        ImGui::SameLine();
        if (ImGui::Button("Fit"))
            m_ViewInitialized = false;
        ImGui::SameLine();
        if (ImGui::Button("Select All"))
            SelectAll();
        ImGui::SameLine();
        if (ImGui::Button("Deselect"))
            m_Selected.clear();
        ImGui::SameLine();
        ImGui::Checkbox("Texture", &m_ShowTexture);
        ImGui::SameLine();
        ImGui::Text("Selected: %d", (int)m_Selected.size());
    }

    void UVViewportPanel::FitView(const ImVec2& canvasSize) {
        float side = 0.8f * std::min(canvasSize.x, canvasSize.y);
        if (side < 1.0f)
            side = 1.0f;
        m_Zoom = side;
        m_Pan = { (canvasSize.x - side) * 0.5f, (canvasSize.y - side) * 0.5f };
    }

    void UVViewportPanel::ZoomAt(const glm::vec2& mouse, const ImVec2& canvasMin, float wheel) {
        glm::vec2 uvUnder = ScreenToUV(mouse, canvasMin);
        float factor = (wheel > 0.0f) ? 1.15f : (1.0f / 1.15f);
        m_Zoom = glm::clamp(m_Zoom * factor, 8.0f, 16384.0f);
        // Re-anchor the pan so the UV point under the cursor stays put.
        m_Pan.x = mouse.x - canvasMin.x - uvUnder.x * m_Zoom;
        m_Pan.y = mouse.y - canvasMin.y - (1.0f - uvUnder.y) * m_Zoom;
    }

    ImVec2 UVViewportPanel::UVToScreen(const glm::vec2& uv, const ImVec2& canvasMin) const {
        return ImVec2(canvasMin.x + m_Pan.x + uv.x * m_Zoom,
                      canvasMin.y + m_Pan.y + (1.0f - uv.y) * m_Zoom);
    }

    glm::vec2 UVViewportPanel::ScreenToUV(const glm::vec2& s, const ImVec2& canvasMin) const {
        return glm::vec2((s.x - canvasMin.x - m_Pan.x) / m_Zoom,
                         1.0f - (s.y - canvasMin.y - m_Pan.y) / m_Zoom);
    }

    std::vector<bool> UVViewportPanel::ReferencedVerts() const {
        const std::vector<glm::vec2>& uvs = m_Session->UVVerts();
        const std::vector<int>& corner = m_Session->CornerUV();
        std::vector<bool> ref(uvs.size(), false);
        for (int c : corner)
            if (c >= 0 && c < (int)uvs.size())
                ref[c] = true;
        return ref;
    }

    void UVViewportPanel::SelectAll() {
        m_Selected.clear();
        std::vector<bool> ref = ReferencedVerts();
        for (int i = 0; i < (int)ref.size(); ++i)
            if (ref[i])
                m_Selected.push_back(i);
    }

    int UVViewportPanel::PickVert(const glm::vec2& mouse, const ImVec2& canvasMin) const {
        const std::vector<glm::vec2>& uvs = m_Session->UVVerts();
        std::vector<bool> ref = ReferencedVerts();
        const float threshold = 8.0f;
        float best = threshold * threshold;
        int hit = -1;
        for (int i = 0; i < (int)uvs.size(); ++i) {
            if (i >= (int)ref.size() || !ref[i])
                continue;
            ImVec2 s = UVToScreen(uvs[i], canvasMin);
            float dx = s.x - mouse.x, dy = s.y - mouse.y;
            float d = dx * dx + dy * dy;
            if (d < best) {
                best = d;
                hit = i;
            }
        }
        return hit;
    }

    void UVViewportPanel::BeginTransform(const glm::vec2& mouse) {
        m_DragStartMouse = mouse;
        const std::vector<glm::vec2>& uvs = m_Session->UVVerts();
        m_DragStartUVs.clear();
        glm::vec2 centroid(0.0f);
        for (int idx : m_Selected) {
            glm::vec2 uv = (idx >= 0 && idx < (int)uvs.size()) ? uvs[idx] : glm::vec2(0.0f);
            m_DragStartUVs.push_back(uv);
            centroid += uv;
        }
        if (!m_Selected.empty())
            centroid /= (float)m_Selected.size();
        m_DragPivotUV = centroid;
    }

    void UVViewportPanel::ApplyTransform(const glm::vec2& mouse, const ImVec2& canvasMin) {
        if (m_Selected.empty())
            return;
        ImGuiIO& io = ImGui::GetIO();
        if (io.MouseDelta.x == 0.0f && io.MouseDelta.y == 0.0f)
            return; // nothing moved this frame; skip the mesh rebuild

        std::vector<glm::vec2>& uvs = m_Session->UVVerts();
        ImVec2 pivot = UVToScreen(m_DragPivotUV, canvasMin);
        glm::vec2 pivotScreen(pivot.x, pivot.y);

        // Transform is computed in screen space (about the selection's pivot) and mapped
        // back to UV, which keeps rotate/scale visually intuitive despite the v-flip.
        for (size_t i = 0; i < m_Selected.size() && i < m_DragStartUVs.size(); ++i) {
            ImVec2 startS = UVToScreen(m_DragStartUVs[i], canvasMin);
            glm::vec2 startScreen(startS.x, startS.y);
            glm::vec2 out;

            switch (m_Tool) {
                case Tool::Move:
                    out = startScreen + (mouse - m_DragStartMouse);
                    break;
                case Tool::Rotate: {
                    float a0 = atan2f(m_DragStartMouse.y - pivotScreen.y, m_DragStartMouse.x - pivotScreen.x);
                    float a1 = atan2f(mouse.y - pivotScreen.y, mouse.x - pivotScreen.x);
                    float c = cosf(a1 - a0), s = sinf(a1 - a0);
                    glm::vec2 r = startScreen - pivotScreen;
                    out = pivotScreen + glm::vec2(r.x * c - r.y * s, r.x * s + r.y * c);
                    break;
                }
                case Tool::Scale: {
                    float d0 = glm::length(m_DragStartMouse - pivotScreen);
                    float d1 = glm::length(mouse - pivotScreen);
                    float f = d0 > 1e-4f ? d1 / d0 : 1.0f;
                    out = pivotScreen + (startScreen - pivotScreen) * f;
                    break;
                }
            }

            int idx = m_Selected[i];
            if (idx >= 0 && idx < (int)uvs.size())
                uvs[idx] = ScreenToUV(out, canvasMin);
        }

        m_Session->SyncToMesh();
    }

    void UVViewportPanel::FinalizeBox(const ImVec2& canvasMin) {
        float minx = std::min(m_BoxStart.x, m_BoxEnd.x);
        float maxx = std::max(m_BoxStart.x, m_BoxEnd.x);
        float miny = std::min(m_BoxStart.y, m_BoxEnd.y);
        float maxy = std::max(m_BoxStart.y, m_BoxEnd.y);
        if (maxx - minx < 3.0f && maxy - miny < 3.0f)
            return; // treat a tiny box as an empty click (selection already cleared)

        const std::vector<glm::vec2>& uvs = m_Session->UVVerts();
        std::vector<bool> ref = ReferencedVerts();
        for (int i = 0; i < (int)uvs.size(); ++i) {
            if (i >= (int)ref.size() || !ref[i])
                continue;
            ImVec2 s = UVToScreen(uvs[i], canvasMin);
            if (s.x >= minx && s.x <= maxx && s.y >= miny && s.y <= maxy) {
                if (std::find(m_Selected.begin(), m_Selected.end(), i) == m_Selected.end())
                    m_Selected.push_back(i);
            }
        }
    }

    void UVViewportPanel::HandleInput(const ImVec2& canvasMin, bool hovered) {
        ImGuiIO& io = ImGui::GetIO();
        glm::vec2 mouse(io.MousePos.x, io.MousePos.y);

        if (m_Drag == Drag::None && hovered) {
            if (ImGui::IsMouseClicked(ImGuiMouseButton_Middle)) {
                m_Drag = Drag::Pan;
            } else if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
                int hit = PickVert(mouse, canvasMin);
                if (hit >= 0) {
                    bool already = std::find(m_Selected.begin(), m_Selected.end(), hit) != m_Selected.end();
                    if (io.KeyShift) {
                        if (already)
                            m_Selected.erase(std::remove(m_Selected.begin(), m_Selected.end(), hit), m_Selected.end());
                        else
                            m_Selected.push_back(hit);
                    } else if (!already) {
                        m_Selected.clear();
                        m_Selected.push_back(hit);
                    }
                    BeginTransform(mouse);
                    m_TransformBefore = m_Session->CaptureState();
                    m_TransformMoved = false;
                    m_Drag = Drag::Transform;
                } else {
                    if (!io.KeyShift)
                        m_Selected.clear();
                    m_BoxStart = mouse;
                    m_BoxEnd = mouse;
                    m_Drag = Drag::Box;
                }
            }
        }

        switch (m_Drag) {
            case Drag::Pan:
                if (ImGui::IsMouseDown(ImGuiMouseButton_Middle)) {
                    m_Pan.x += io.MouseDelta.x;
                    m_Pan.y += io.MouseDelta.y;
                } else {
                    m_Drag = Drag::None;
                }
                break;
            case Drag::Transform:
                if (ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
                    ApplyTransform(mouse, canvasMin);
                    if (io.MouseDelta.x != 0.0f || io.MouseDelta.y != 0.0f)
                        m_TransformMoved = true;
                } else {
                    if (m_TransformMoved)
                        m_Session->History().Push(MakeUnique<MeshEditCommand>(
                            m_Session, m_TransformBefore, m_Session->CaptureState()));
                    m_Drag = Drag::None;
                }
                break;
            case Drag::Box:
                if (ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
                    m_BoxEnd = mouse;
                } else {
                    FinalizeBox(canvasMin);
                    m_Drag = Drag::None;
                }
                break;
            case Drag::None:
                break;
        }

        if (hovered && m_Drag == Drag::None && io.MouseWheel != 0.0f)
            ZoomAt(mouse, canvasMin, io.MouseWheel);
    }

    unsigned int UVViewportPanel::AlbedoTextureID() const {
        Entity e = m_Session->GetEntity();
        if (!e || !e.HasComponent<MaterialComponent>())
            return 0;

        UUID handle = e.GetComponent<MaterialComponent>().MaterialHandle;
        Shared<Material> mat = AssetManager::GetAsset<Material>(handle);
        if (!mat)
            return 0;

        auto it = mat->m_Properties.find(MaterialType::Albedo);
        if (it == mat->m_Properties.end() || !std::holds_alternative<Shared<Texture2D>>(it->second))
            return 0;

        Shared<Texture2D> tex = std::get<Shared<Texture2D>>(it->second);
        return tex ? tex->GetID() : 0;
    }

    void UVViewportPanel::Draw(const ImVec2& canvasMin, const ImVec2& canvasMax) {
        ImDrawList* dl = ImGui::GetWindowDrawList();
        dl->PushClipRect(canvasMin, canvasMax, true);

        dl->AddRectFilled(canvasMin, canvasMax, IM_COL32(30, 30, 30, 255));

        if (m_ShowTexture) {
            unsigned int tex = AlbedoTextureID();
            if (tex != 0) {
                ImVec2 tl = UVToScreen({ 0.0f, 1.0f }, canvasMin); // top-left of the unit square
                ImVec2 br = UVToScreen({ 1.0f, 0.0f }, canvasMin); // bottom-right
                dl->AddImage((void*)(intptr_t)tex, tl, br, ImVec2(0, 1), ImVec2(1, 0));
            }
        }

        // 0..1 grid; the outer lines (i == 0 / 4) mark the unit square.
        for (int i = 0; i <= 4; ++i) {
            float t = i / 4.0f;
            ImU32 col = (i == 0 || i == 4) ? IM_COL32(130, 130, 130, 255) : IM_COL32(70, 70, 70, 255);
            dl->AddLine(UVToScreen({ t, 0.0f }, canvasMin), UVToScreen({ t, 1.0f }, canvasMin), col, 1.0f);
            dl->AddLine(UVToScreen({ 0.0f, t }, canvasMin), UVToScreen({ 1.0f, t }, canvasMin), col, 1.0f);
        }

        const HalfEdgeMesh& mesh = m_Session->Mesh();
        const std::vector<int>& corner = m_Session->CornerUV();
        const std::vector<glm::vec2>& uvs = m_Session->UVVerts();

        // UV edges, drawn per face corner-to-corner.
        for (int f = 0; f < (int)mesh.Faces().size(); ++f) {
            std::vector<int> he = mesh.FaceHalfEdges(f);
            int n = (int)he.size();
            for (int k = 0; k < n; ++k) {
                int a = corner[he[k]];
                int b = corner[he[(k + 1) % n]];
                if (a < 0 || b < 0 || a >= (int)uvs.size() || b >= (int)uvs.size())
                    continue;
                dl->AddLine(UVToScreen(uvs[a], canvasMin), UVToScreen(uvs[b], canvasMin),
                            IM_COL32(210, 210, 210, 160), 1.0f);
            }
        }

        // UV verts (only those referenced by a corner); selected ones highlighted.
        std::vector<bool> ref = ReferencedVerts();
        std::vector<bool> sel(uvs.size(), false);
        for (int i : m_Selected)
            if (i >= 0 && i < (int)uvs.size())
                sel[i] = true;
        for (int i = 0; i < (int)uvs.size(); ++i) {
            if (i >= (int)ref.size() || !ref[i])
                continue;
            ImVec2 s = UVToScreen(uvs[i], canvasMin);
            if (sel[i]) {
                dl->AddCircleFilled(s, 4.0f, IM_COL32(255, 160, 0, 255));
                dl->AddCircle(s, 4.0f, IM_COL32(0, 0, 0, 180), 0, 1.0f);
            } else {
                dl->AddCircleFilled(s, 3.0f, IM_COL32(225, 225, 225, 255));
                dl->AddCircle(s, 3.0f, IM_COL32(0, 0, 0, 160), 0, 1.0f);
            }
        }

        if (m_Drag == Drag::Box) {
            ImVec2 a(m_BoxStart.x, m_BoxStart.y);
            ImVec2 b(m_BoxEnd.x, m_BoxEnd.y);
            dl->AddRectFilled(a, b, IM_COL32(255, 160, 0, 40));
            dl->AddRect(a, b, IM_COL32(255, 160, 0, 220));
        }

        dl->PopClipRect();
    }

}
