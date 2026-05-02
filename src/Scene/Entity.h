#pragma once

#include "pch.h"
#include "Core/UUID.h"
#include "Components.h"
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

        template <typename T>
        entt::entity GetRootWithComponent()
        {
            Entity current = *this;
            entt::entity highest = entt::null;

            // 1. Check if the starting entity itself has the component
            if (current.HasComponent<T>())
            {
                highest = current.GetHandle();
            }

            // 2. Climb up the tree to the absolute root
            while (current.HasParent())
            {
                entt::entity parent = current.GetParent();
                if (parent == entt::null) break;

                current = Entity{ parent, m_Scene };
                
                // 3. Every time we find the component, overwrite our record 
                // so we end up with the one highest in the tree.
                if (current.HasComponent<T>())
                {
                    highest = current.GetHandle();
                }
            }

            return highest;
        }

        template <typename T>
        entt::entity GetClosestParentWithComponent()
        {
            Entity current = *this;

            if (current.HasComponent<T>()) return current;

            while (current.HasParent())
            {
                current = Entity{ current.GetParent(), m_Scene };
                
                // Stop and return immediately upon finding the first match
                if (current.HasComponent<T>())
                {
                    return current.GetHandle();
                }
            }

            return entt::null;
        }

        template <typename T>
        void GetAllChildrenWithComponent(std::vector<Entity>& outEntities)
        {
            // 1. Get the immediate children of this entity
            std::vector<entt::entity> children = GetChildren();

            for (entt::entity childHandle : children)
            {
                // Wrap the raw handle in an Entity object
                Entity childEntity = { childHandle, m_Scene };

                // 2. Check if this child has the component
                if (childEntity.HasComponent<T>())
                {
                    outEntities.push_back(childEntity);
                }

                // 3. Recurse: Ask this child to find its children with component T
                childEntity.GetAllChildrenWithComponent<T>(outEntities);
            }
        }

        UUID GetUUID() { return GetComponent<IDComponent>().id; };

        bool operator==(const Entity &other) const { return m_Handle == other.GetHandle() && m_Scene == other.m_Scene; };
        bool operator!=(const Entity &other) const { return !operator==(other); };
        operator bool() const { return m_Handle != entt::null; }
        operator entt::entity() const { return m_Handle; }

        Scene* GetScene() { return m_Scene; }


	    static Entity CloneEntity(Entity sourceEntity, Scene* targetScene, Entity targetParent);

        void SetParent(Entity& parent);
        entt::entity GetParent();
        std::vector<entt::entity> GetChildren();
        bool HasParent();

        private:
        entt::entity m_Handle { entt::null };
        Scene* m_Scene = nullptr;
    };

};
