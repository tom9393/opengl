#include "pch.h"
#include "Asset/AssetManager.h"
#include "Core/KeyCodes.h"
#include "Core/Application.h"
#include "Core/MouseButtonCodes.h"
#include "EditorLayer.h"
#include "Events/EventBroker.h"
#include "Render/Renderer.h"
#include "Scene/Entity.h"
#include "Scene/SceneSerializer.h"
#include "Scripting/ScriptEngine.h"
#include "Utils/PlatformUtils.h"
#include "Utils/GizmoManager.h"
#include "Utils/SceneManager.h"

namespace Engine
{

    EditorLayer::EditorLayer()
    {
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

        io.Fonts->AddFontFromFileTTF("../assets/fonts/opensans/OpenSans-Bold.ttf", 18.0f);
        io.FontDefault = io.Fonts->AddFontFromFileTTF("../assets/fonts/opensans/OpenSans-Regular.ttf", 18.0f);

        m_EditorCamera = MakeShared<EditorCamera>(90.0f, 2.0f, 0.1f, 1000.0f);
        m_EditingScene = MakeShared<Scene>();
        m_SceneRenderer = MakeShared<SceneRenderer>(m_EditingScene.get());
        m_SceneRenderer->OnViewportResize(500, 500);

        SceneManager::SetContext(m_EditingScene.get());

        m_ViewportPanel.SetContext(m_EditingScene);
        m_ViewportPanel.SetRenderer(m_SceneRenderer);
        m_ViewportPanel.SetCamera(m_EditorCamera);

        m_ToolbarPanel.SetPlayCallback([this]() { OnScenePlay();});
        m_ToolbarPanel.SetStopCallback([this]() { OnSceneStop();});

        m_MenuBarPanel.SetSaveCallback([this]() { SaveScene(); });
        m_MenuBarPanel.SetOpenCallback([this]() { OpenScene(); });

        m_Shortcuts[KEY_W] = []() { GizmoManager::SetOperation(ImGuizmo::OPERATION::TRANSLATE); };
        m_Shortcuts[KEY_R] = []() { GizmoManager::SetOperation(ImGuizmo::OPERATION::ROTATE); };
        m_Shortcuts[KEY_D] = []() { GizmoManager::SetOperation(ImGuizmo::OPERATION::SCALE); };
        m_Shortcuts[KEY_F1] = [this]() { 
            if (SceneManager::GetSceneState() == SceneState::Edit) OnScenePlay(); 
            else OnSceneStop(); 
        };
        m_Shortcuts[KEY_F5] = [this]() { 
            ScriptEngine::ReloadScripts();
        };
    };

    EditorLayer::~EditorLayer()
    {
        EventBroker::Get().UnsubscribeAll(this);
    };

    void EditorLayer::OnAttach() {
        std::cout << "EditorLayer attached" << std::endl;
        EventBroker::Get().Subscribe<KeyboardPressEvent>(BIND_FN(OnKeyEvent), this);
    };

    void EditorLayer::OnDetach() {
        std::cout << "EditorLayer detached" << std::endl;
        EventBroker::Get().UnsubscribeAll(this);
    };

    void EditorLayer::OnUpdate(TimeStep ts)
    {
        switch (SceneManager::GetSceneState()) {
            case Engine::SceneState::Edit:
            {
                m_EditorCamera->OnUpdate();
                m_SceneRenderer->RenderEditor(m_EditorCamera->GetViewProjection(), m_EditorCamera->GetPosition());
                break;
            }
            case Engine::SceneState::Play:
            {
                m_RuntimeScene->OnUpdate(ts);
                m_SceneRenderer->RenderRuntime();
                break;
            }
        }
    };

    void EditorLayer::OnImGuiRender()
    {
        ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport());

        m_MenuBarPanel.OnImGuiRender();
        m_StatsPanel.OnImGuiRender();
        m_ToolbarPanel.OnImGuiRender();
        m_ShaderPanel.OnImGuiRender();

        m_ViewportPanel.OnImGuiRender();
        m_SceneHierarchyPanel.OnImGuiRender();
        m_ContentBrowserPanel.OnImGuiRender();
        m_SettingsPanel.OnImGuiRender();

        Renderer::ResetStatistics();
    }

    bool EditorLayer::OnKeyEvent(KeyboardPressEvent &e)
    {
        if (m_Shortcuts.find(e.getKeyCode()) != m_Shortcuts.end()) {
            m_Shortcuts[e.getKeyCode()]();
            return true;
        }
        return false;
    };

    void EditorLayer::OpenScene()
    {
        std::string filepath = FileDialogs::OpenFile("*.pmblend");
        if (!filepath.empty())
        {
            m_EditingScene = MakeShared<Scene>();
            SceneManager::SetContext(m_EditingScene.get());
            SceneSerializer serializer(m_EditingScene);
            serializer.Deserialize(filepath);

            m_SceneRenderer->SetScene(m_EditingScene.get());
        }
    };

    void EditorLayer::SaveScene()
    {
        std::string filepath = FileDialogs::SaveFile("*.pmblend");
        if (!filepath.empty())
        {
            SceneSerializer serializer(m_EditingScene);
            serializer.Serialize(filepath);
        }
    };

    void EditorLayer::OnScenePlay()
    {
        SceneManager::SetSceneState(SceneState::Play);

        m_RuntimeScene = Scene::Copy(m_EditingScene);
        m_SceneRenderer->SetScene(m_RuntimeScene.get());
        m_RuntimeScene->OnRuntimeStart();
        Application::Get().GetWindow().DisableCursor();
    }

    void EditorLayer::OnSceneStop()
    {
        SceneManager::SetSceneState(SceneState::Edit);

        m_RuntimeScene->OnRuntimeStop();
        m_SceneRenderer->SetScene(m_EditingScene.get());
        m_RuntimeScene = nullptr;
        Application::Get().GetWindow().EnableCursor();
    }

};
