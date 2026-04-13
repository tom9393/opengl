#pragma once

#include "../Scene/Scene.h"
#include "../Scene/Entity.h"
#include "../Scene/SceneRenderer.h"
#include "../Render/EditorCamera.h"
#include "../Utils/SceneManager.h"
#include <functional>
#include <glm/glm.hpp>

namespace Engine {

    class ViewportPanel {
    public:
        ViewportPanel();
        ~ViewportPanel();

        void SetContext(Shared<Scene> scene);
        void SetRenderer(Shared<SceneRenderer> renderer);
        void SetCamera(Shared<EditorCamera> camera);

        // Callbacks for events the panel shouldn't handle directly
        void SetSceneDroppedCallback(const std::function<void(const std::string&)>& callback);

        // Called every frame to draw the ImGui window
        void OnImGuiRender();

        // State getters
        bool IsHovered() const { return m_IsHovered; }
        bool IsFocused() const { return m_IsFocused; }
        glm::vec2 GetSize() const { return m_ViewportSize; }
        
        // Handles raycasting and returns the selected entity ID (or entt::null)
        entt::entity RaycastSelection();

    private:
        void DrawDragAndDrop();
        void DrawContextMenu();
        void UpdateSelectedEntityGizmo();
        bool OnMouseButtonPressed(MouseButtonPressedEvent &e);

    private:
        Shared<Scene> m_Context;
        Shared<SceneRenderer> m_SceneRenderer;
        Shared<EditorCamera> m_EditorCamera;

        bool m_IsHovered = false;
        bool m_IsFocused = false;
        glm::vec2 m_ViewportSize = { 0.0f, 0.0f };
        glm::vec2 m_ViewportBounds[2] = { {0.0f, 0.0f}, {0.0f, 0.0f} };
        glm::vec2 m_MousePos = { 0.0f, 0.0f };

        std::function<void(const std::string&)> m_SceneDroppedCallback;
    };

}