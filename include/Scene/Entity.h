#pragma once

#include "../../lib/entt.hpp"
#include "Scene.h"
#include <memory>

class Entity
{
    public:
    Entity() = default;
    Entity(entt::entity entity, Scene* scene);
    Entity(const Entity &other) = default;

    template <typename T, typename... Args>
    T& AddComponent(Args&&... args)
    {
        if (HasComponent<T>()) std::cout << "Already has component" << std::endl;
        return m_Scene->m_Registry.emplace<T>(m_Handle, std::forward<Args>(args)...);
    }

    template <typename T>
    void RemoveComponent()
    {
        if (!HasComponent<T>()) std::cout << "Does not have component to remove" << std::endl;
        m_Scene->m_Registry.remove<T>(m_Handle);
    }

    template <typename T>
    T& GetComponent()
    {
        if (!HasComponent<T>()) std::cout << "Does not have component" << std::endl;
        return m_Scene->m_Registry.get<T>(m_Handle);
    }
    
    template <typename T>
    bool HasComponent()
    {
        return m_Scene->m_Registry.all_of<T>(m_Handle);
    }

    operator bool() const { return m_Handle != entt::null; }

    private:
    entt::entity m_Handle { entt::null };
    Scene* m_Scene = nullptr;
};
