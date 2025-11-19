#pragma once

#include "../../lib/entt/single_include/entt/entt.hpp"
#include "Scene.h"

namespace Engine
{

    class Entity
    {
        public:
        Entity() = default;
        Entity(entt::entity entity, Scene* scene);
        Entity(const Entity &other) = default;

        entt::entity GetHandle() const { return m_Handle; };

        template <typename T, typename... Args>
        T& AddComponent(Args&&... args)
        {
            if (HasComponent<T>())
            {
                std::cout << "Already has component" << std::endl;
                return GetComponent<T>();
            }
            T& component = m_Scene->m_Registry.emplace<T>(m_Handle, std::forward<Args>(args)...);
            m_Scene->OnComponentAdded<T>(*this, component);
            return component;
        }

        template <typename T>
        void RemoveComponent()
        {
            if (!HasComponent<T>())
            {
                std::cout << "Does not have component to remove" << std::endl;
                return;
            }
            m_Scene->m_Registry.remove<T>(m_Handle);
        }

        template <typename T>
        T& GetComponent()
        {
            return m_Scene->m_Registry.get<T>(m_Handle);
        }
        
        template <typename T>
        bool HasComponent()
        {
            return m_Scene->m_Registry.all_of<T>(m_Handle);
        }

        bool operator==(const Entity &other) const { return m_Handle == other.GetHandle() && m_Scene == other.m_Scene; };
        operator bool() const { return m_Handle != entt::null; }
        operator entt::entity() const { return m_Handle; }

        private:
        entt::entity m_Handle { entt::null };
        Scene* m_Scene = nullptr;
    };

};
