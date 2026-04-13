#include "../pch.h"
#include "Asset/AssetManager.h"
#include "Core/MouseButtonCodes.h"
#include "EditorLayer.h"
#include "Render/Renderer.h"
#include "Core/KeyCodes.h"
#include "Core/Application.h"
#include "Scene/Entity.h"
#include "Scene/SceneSerializer.h"
#include "Utils/PlatformUtils.h"
#include "Events/EventBroker.h"
#include "Scripting/ScriptEngine.h"
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
        m_ViewportPanel.SetSceneDroppedCallback([this](const std::string& path) { this->OpenScene(path.c_str()); });

        m_ToolbarPanel.SetPlayCallback([this]() { OnScenePlay();});
        m_ToolbarPanel.SetStopCallback([this]() { OnSceneStop();});
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
        PROFILE_FUNCTION();

        switch (SceneManager::GetSceneState()) {
            case Engine::SceneState::Edit:
            {
                m_EditorCamera->OnUpdate(ts);
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
        switch (e.getKeyCode())
        {
            case KEY_W:
                GizmoManager::SetOperation(ImGuizmo::OPERATION::TRANSLATE);
                break;
            case KEY_R:
                GizmoManager::SetOperation(ImGuizmo::OPERATION::ROTATE);
                break;
            case KEY_D:
                GizmoManager::SetOperation(ImGuizmo::OPERATION::SCALE);
                break;
            case KEY_F1:
                if (SceneManager::GetSceneState() == SceneState::Edit) OnScenePlay();
                else OnSceneStop();
                break;
            case KEY_F5:
                ScriptEngine::ReloadScripts();
                break;
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

    void EditorLayer::OpenScene(const char* path)
    {
        if (path != nullptr)
        {
            m_EditingScene = AssetManager::GetAsset<Scene>(AssetManager::ImportAsset(path));
            SceneManager::SetContext(m_EditingScene.get());

            m_SceneRenderer->SetScene(m_EditingScene.get());
        }
    }

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
