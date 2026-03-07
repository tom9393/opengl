#pragma once

#include "../../lib/entt/single_include/entt/entt.hpp"
#include "../TimeStep.h"
#include "SceneSerializer.h"
#include "../EditorCamera.h"
#include "../PhysicsWorld.h"
#include "../UUID.h"

namespace Engine
{

    class Entity;

    class Scene : public Asset
    {
        public:
        static Shared<Scene> Copy(Shared<Scene> other);

        void OnUpdate(TimeStep ts);
        void OnUpdateEditor(TimeStep ts, Shared<EditorCamera> &camera);

        void OnRuntimeStart();
        void OnRuntimeStop();

        void OnViewportResize(unsigned int width, unsigned int height);

        Entity CreateEntity(const std::string &name = "Entity");
        Entity CreateEntityWithUUID(UUID uuid, const std::string &name = "Entity");
        void DestroyEntity(Entity entity);

        void SyncPhysicsTransforms();

        private:
        void RenderScene(const glm::mat4 &viewProjection);

        public:
        unsigned int m_ViewportWidth = 0;
        unsigned int m_ViewportHeight = 0;
        entt::registry m_Registry;
        PhysicsWorld m_Physicsworld = PhysicsWorld(0, -9.81f, 0);

        friend class Entity;
        friend class SceneSerializer;
        friend class SceneHierarchyPanel;
    };

};
