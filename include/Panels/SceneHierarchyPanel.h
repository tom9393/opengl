#pragma once

#include "../Scene/Scene.h"
#include "../Scene/Entity.h"

class SceneHierarchyPanel
{
    public:
    SceneHierarchyPanel() = default;
    SceneHierarchyPanel(const Shared<Scene> &context);
    void SetContext(const Shared<Scene> &context);
    void OnImGuiRender();

    private:
    void DrawEntityNode(Entity entity);
    void DrawComponents(Entity entity);

    private:
    Shared<Scene> m_Context;
    Entity m_SelectionContext;
};
