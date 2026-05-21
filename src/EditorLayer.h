#pragma once

#include "Core/Application.h"
#include "Core/Layer.h"
#include "Core/TimeStep.h"
#include "Events/KeyboardEvent.h"
#include "Events/MouseEvent.h"
#include "Panels/SceneHierarchy/SceneHierarchyPanel.h"
#include "Panels/ContentBrowser/ContentBrowserPanel.h"
#include "Panels/SettingsPanel.h"
#include "Panels/Viewport/ViewportPanel.h"
#include "Panels/StatsPanel.h"
#include "Panels/ToolbarPanel.h"
#include "Panels/MenubarPanel.h"
#include "Panels/ShaderPanel.h"
#include "Render/EditorCamera.h"
#include "Scene/Scene.h"
#include "SceneRenderer.h"

namespace Engine
{

    class EditorLayer : public Layer
    {
        public:
        EditorLayer();
        ~EditorLayer();

        void OnAttach();
        void OnDetach();
        void OnUpdate(TimeStep ts);
        void OnImGuiRender();

        private:
        void UpdateSelectedEntity();
        void UpdateHoveredEntity();

        void OpenScene();
        void OpenScene(const UUID handle);
        void SaveScene();

        void OnScenePlay();
        void OnSceneStop();

        bool OnKeyEvent(KeyboardPressEvent &e);

        private:
        Shared<Scene> m_EditingScene;
        Shared<Scene> m_RuntimeScene;
        Shared<SceneRenderer> m_SceneRenderer;
        Shared<EditorCamera> m_EditorCamera;

        using Keycode = unsigned int;

        std::unordered_map<Keycode, std::function<void()>> m_Shortcuts;

        SceneHierarchyPanel m_SceneHierarchyPanel;
        ContentBrowserPanel m_ContentBrowserPanel;
        SettingsPanel m_SettingsPanel;
        ViewportPanel m_ViewportPanel;
        StatsPanel m_StatsPanel;
        ToolbarPanel m_ToolbarPanel;
        MenuBarPanel m_MenuBarPanel;
        ShaderPanel m_ShaderPanel;
    };

};
