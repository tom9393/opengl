#pragma once
#include "Buffer.h"
#include "CameraController.h"
#include "EditorCamera.h"
#include "Layer.h"
#include "Ray.h"
#include "AABB.h"
#include "Scene/Scene.h"
#include "Texture.h"
#include "TimeStep.h"
#include "Shader.h"
#include "Events/MouseEvent.h"
#include "model.h"
#include "Panels/SceneHierarchyPanel.h"

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
    bool OnMouseButtonPressed(MouseButtonPressedEvent &e);

    Ray CastRayFromMouse(float x, float y, float width, float height, std::shared_ptr<Camera> camera);
    bool RayIntersectsAABB(const Ray& ray, const AABB& box, float& t);
    void TrySelectMesh(float mouseX, float mouseY, float screenWidth, float screenHeight, std::shared_ptr<Camera> camera, std::shared_ptr<Model> model);

    private:
    void DrawStats();
    void OpenScene();
    void SaveScene();

    private:
    std::shared_ptr<Model> m_Model;
    bool m_ViewportFocused = false;
    bool m_ViewportHovered = false;
    glm::vec2 m_ViewportSize = {0.0f, 0.0f};
    glm::vec2 m_ViewportBounds[2];
    float m_Previous = 0;

    Shared<Scene> m_Scene;
    Shared<EditorCamera> m_EditorCamera;

    std::shared_ptr<Texture2D> m_Texture;
    std::shared_ptr<Mesh> m_SelectedMesh;
    std::shared_ptr<Framebuffer> m_Framebuffer;

    // Panels

    SceneHierarchyPanel m_SceneHierarchyPanel;
};
