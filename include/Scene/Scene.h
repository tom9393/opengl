#pragma once

#include "../../lib/entt/single_include/entt/entt.hpp"
#include "../TimeStep.h"
#include "SceneSerializer.h"
#include "../ModelLibrary.h"
#include "../EditorCamera.h"

namespace Engine
{

    class Entity;

    class Scene
    {
        public:
        Scene();
        ~Scene();

        void OnUpdate(TimeStep ts);
        void OnUpdateEditor(TimeStep ts, Shared<EditorCamera> &camera);

        void OnViewportResize(unsigned int width, unsigned int height);

        Entity CreateEntity(const std::string &name = "Entity");
        void DestroyEntity(Entity entity);

        private:
        template <typename T>
        void OnComponentAdded(Entity entity, T &component);

        private:
        entt::registry m_Registry;
        unsigned int m_ViewportWidth = 0;
        unsigned int m_ViewportHeight = 0;
        ModelLibrary m_ModelLibrary;

        friend class Entity;
        friend class SceneSerializer;
        friend class SceneHierarchyPanel;
    };

};
