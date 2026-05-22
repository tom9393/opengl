#pragma once

#include "pch.h"
#include "Core/TimeStep.h"
#include "Core/UUID.h"
#include "Events/MouseEvent.h"
#include "Physics/PhysicsEngine.h"
#include "Render/EnvironmentMap.h"
#include "SceneSerializer.h"

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
        void DestroyEntity(Entity& entity);

        entt::registry& GetRegistry() { return m_Registry; }

        Entity FindRootEntity();

        // --- Environment / IBL ---
        // Load and bake an equirectangular .hdr (GL-thread only). Empty path clears it.
        void SetEnvironmentPath(const std::string& path);
        const std::string& GetEnvironmentPath() const { return m_EnvironmentPath; }
        Shared<EnvironmentMap> GetEnvironment() const { return m_Environment; }
        float& EnvironmentIntensity() { return m_EnvironmentIntensity; }
        float GetEnvironmentIntensity() const { return m_EnvironmentIntensity; }
        void SetEnvironmentIntensity(float intensity) { m_EnvironmentIntensity = intensity; }

        // --- Post processing ---
        PostProcessSettings& GetPostSettings() { return m_PostSettings; }
        const PostProcessSettings& GetPostSettings() const { return m_PostSettings; }

        private:
        void OnMousePress(MouseButtonPressedEvent &e);

        private:
        entt::registry m_Registry;

        Shared<EnvironmentMap> m_Environment;
        std::string m_EnvironmentPath;
        float m_EnvironmentIntensity = 1.0f;
        PostProcessSettings m_PostSettings;

        friend class SceneRenderer;
        friend class Entity;
        friend class SceneSerializer;
        friend class SceneHierarchyPanel;
        friend class NativeScript;
        friend class PhysicsEngine;
        friend class ScriptEngine;
        friend class LuaBindings;
    };

};
