#pragma once

namespace Engine
{

    class Scene;
    class Entity;

    class SceneHierarchyPanel
    {
        public:
        SceneHierarchyPanel() = default;
        SceneHierarchyPanel(const Shared<Scene> &context);
        void OnImGuiRender();

        private:
        void DrawEntityNode(Entity entity);

        private:
        bool m_Deleted = false;
    };

};
