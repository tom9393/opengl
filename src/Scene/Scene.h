#pragma once

#include "../../lib/entt/single_include/entt/entt.hpp"
#include "../TimeStep.h"
#include "Components.h"
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

        void OnUpdate(TimeStep ts);
        void OnUpdateEditor(TimeStep ts, Shared<EditorCamera> &camera);

        void OnRuntimeStart();
        void OnRuntimeStop();

        void OnViewportResize(unsigned int width, unsigned int height);

        Entity CreateEntity(const std::string &name = "Entity");
        Entity CreateEntityWithUUID(UUID uuid, const std::string &name = "Entity");
        void DestroyEntity(Entity entity);

        void SyncPhysicsTransforms();
        SceneCameraComponent* GetPrimaryCamera();

        private:
        template <typename T>
        void OnComponentAdded(Entity entity, T &component);
        
        void RenderScene(const glm::mat4 &viewProjection);

        private:
        entt::registry m_Registry;
        unsigned int m_ViewportWidth = 0;
        unsigned int m_ViewportHeight = 0;

        PhysicsWorld* m_PhysicsWorld = nullptr;

        friend class Entity;
        friend class SceneSerializer;
        friend class SceneHierarchyPanel;
    };

};
