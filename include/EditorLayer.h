#pragma once

#include "Application.h"
#include "Buffer.h"
#include "EditorCamera.h"
#include "Events/KeyboardEvent.h"
#include "Layer.h"
#include "Panels/ContentBrowserPanel.h"
#include "Render/RenderGraph.h"
#include "Scene/Scene.h"
#include "Texture.h"
#include "TimeStep.h"
#include "Events/MouseEvent.h"
#include "Panels/SceneHierarchyPanel.h"

namespace Engine
{

    struct Viewport
    {
        glm::vec2 Min;
        glm::vec2 Max;
        glm::vec2 Size;
        bool Focused = false;
        bool Hovered = false;
        bool Updated = false;

        void UpdateViewport()
        {
            Updated = false;

            auto minRegion = ImGui::GetWindowContentRegionMin(); // Top left corner of window's content area
            auto maxRegion = ImGui::GetWindowContentRegionMax(); // Bottom right corner of window's content area
            auto offset = ImGui::GetWindowPos(); // Absolute position, (0, 0) top left corner

            Min = { minRegion.x + offset.x, minRegion.y + offset.y }; // Top left corner
            Max = { maxRegion.x + offset.x, maxRegion.y + offset.y }; // Bottom right corner
            glm::vec2 s = Max - Min;
            if (s.x != Size.x || s.y != Size.y)
            {
                Updated = true;
                Size = s;
            }

            Focused = ImGui::IsWindowFocused();
            Hovered = ImGui::IsWindowHovered();

            Application::Get().GetImGuiLayer()->BlockEvents(!Focused || !Hovered);
        }

        glm::vec2 GetMousePosition()
        {
            glm::vec2 mouse = { ImGui::GetMousePos().x, ImGui::GetMousePos().y };
            glm::vec2 local = mouse - Min;
            local.y = Size.y - local.y;
            return glm::vec2(local);
        }

        bool Contains(const glm::vec2& p) const
        {
            return p.x >= Min.x && p.x <= Max.x && p.y >= Min.y && p.y <= Max.y;
        }
    };

    enum class SceneState
    {
        Edit = 0,
        Play = 1
    };

    class EditorLayer : public Layer
    {
        public:
        EditorLayer();
        ~EditorLayer();

        void OnAttach();
        void OnDetach();
        void OnUpdate(TimeStep ts);
        void OnEvent(Event& event);
        void OnImGuiRender();

        private:
        void DrawStats();
        void DrawMenuBar();
        void DrawViewport();
        void DrawToolbar();

        void UpdateSelectedEntity();
        void UpdateHoveredEntity();

        void SetupGizmo();
        void UseGizmo(Entity entity);

        void OpenScene();
        void OpenScene(const char* path);
        void SaveScene();

        void OnScenePlay();
        void OnSceneStop();

        bool OnMouseButtonPressed(MouseButtonPressedEvent &e);
        bool OnKeyEvent(KeyboardPressEvent &e);

        private:
        float m_Previous = 0;
        Viewport m_Viewport;
        Entity m_HoveredEntity;

        Shared<Scene> m_Scene;
        Shared<EditorCamera> m_EditorCamera;

        int m_GizmoType = -1;

        RenderGraph m_RenderGraph;

        // Panels
        SceneHierarchyPanel m_SceneHierarchyPanel;
        ContentBrowserPanel m_ContentBrowserPanel;

        SceneState m_SceneState = SceneState::Edit;
    };

};
