#pragma once

#include "../../lib/entt/single_include/entt/entt.hpp"
#include "../Core/TimeStep.h"
#include "SceneSerializer.h"
#include "../Physics/PhysicsEngine.h"
#include "../Core/UUID.h"
#include "../Events/MouseEvent.h"

namespace Engine
{

    class Entity;

    class Scene : public Asset
    {
        public:
        static Shared<Scene> Copy(Shared<Scene> other);

        void OnUpdate(TimeStep ts);

        void OnRuntimeStart();
        void OnRuntimeStop();

        Entity CreateEntity(const std::string &name = "Entity");
        Entity CreateEntityWithUUID(UUID uuid, const std::string &name = "Entity");
        void DestroyEntity(Entity entity);

        entt::registry& GetRegistry() { return m_Registry; }

        private:
        void OnMousePress(MouseButtonPressedEvent &e);

        private:
        entt::registry m_Registry;

        friend class SceneRenderer;
        friend class Entity;
        friend class SceneSerializer;
        friend class SceneHierarchyPanel;
        friend class NativeScript;
        friend class PhysicsEngine;
        friend class ScriptEngine;
    };

};
