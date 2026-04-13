#pragma once

#include "../Scene/Scene.h"
#include "../Scene/Entity.h"

namespace Engine
{

    class SceneHierarchyPanel
    {
        public:
        SceneHierarchyPanel() = default;
        SceneHierarchyPanel(const Shared<Scene> &context);
        void OnImGuiRender();

        private:
        void DrawEntityNode(Entity entity);
        void DrawComponents(Entity entity);

        private:
        bool m_Deleted = false;
    };

};
