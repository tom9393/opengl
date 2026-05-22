#pragma once

#include "pch.h"
#include "Command.h"
#include "CommandHistory.h"
#include "EntitySnapshot.h"
#include "Scene/Components.h"
#include "Scene/Entity.h"
#include "Scene/Scene.h"
#include "Utils/SceneManager.h"

namespace Engine {

    // ---- Subtree memento (an entity plus all its descendants) -------------------
    //
    // Captures everything needed to rebuild an entity hierarchy identically: UUIDs,
    // every non-structural component, and the parent / sibling position of the root.
    // Used by Create (undo destroys, redo rebuilds) and Delete (undo rebuilds, redo
    // destroys). UUIDs are preserved so cross-references and repeated undo/redo stay
    // stable; the entt handles are rebuilt fresh on restore.

    struct SubtreeNode {
        UUID id;
        EntitySnapshot components;
        std::vector<SubtreeNode> children;
    };

    struct SubtreeMemento {
        SubtreeNode root;
        UUID parent = 0;       // 0 == root has no parent
        int  siblingIndex = -1;
    };

    namespace Detail {

        inline SubtreeNode CaptureNode(Entity entity)
        {
            SubtreeNode node;
            node.id = entity.GetUUID();
            node.components = CaptureEntity(entity);
            for (entt::entity childHandle : entity.GetChildren())
                node.children.push_back(CaptureNode(Entity{ childHandle, entity.GetScene() }));
            return node;
        }

        inline void ReorderChild(Entity parent, entt::entity child, int index)
        {
            if (index < 0 || !parent.HasComponent<ChildrenComponent>()) return;
            auto& children = parent.GetComponent<ChildrenComponent>().Children;
            auto it = std::find(children.begin(), children.end(), child);
            if (it == children.end()) return;
            children.erase(it);
            int clamped = std::min(index, (int)children.size());
            children.insert(children.begin() + clamped, child);
        }

        inline Entity RestoreNode(Scene* scene, const SubtreeNode& node)
        {
            Entity entity = scene->CreateEntityWithUUID(node.id);
            RestoreEntity(scene, node.id, node.components);
            for (const SubtreeNode& child : node.children)
            {
                Entity childEntity = RestoreNode(scene, child);
                childEntity.SetParent(entity);
            }
            return entity;
        }

    }

    inline SubtreeMemento CaptureSubtree(Entity entity)
    {
        SubtreeMemento memento;
        memento.root = Detail::CaptureNode(entity);

        entt::entity parentHandle = entity.GetParent();
        if (parentHandle != entt::null)
        {
            Entity parent{ parentHandle, entity.GetScene() };
            memento.parent = parent.GetUUID();
            if (parent.HasComponent<ChildrenComponent>())
            {
                auto& children = parent.GetComponent<ChildrenComponent>().Children;
                auto it = std::find(children.begin(), children.end(), entity.GetHandle());
                if (it != children.end())
                    memento.siblingIndex = (int)std::distance(children.begin(), it);
            }
        }
        return memento;
    }

    inline void RestoreSubtree(Scene* scene, const SubtreeMemento& memento)
    {
        Entity root = Detail::RestoreNode(scene, memento.root);
        if ((uint64_t)memento.parent != 0)
        {
            Entity parent = FindEntityByUUID(scene, memento.parent);
            if (parent)
            {
                root.SetParent(parent);
                Detail::ReorderChild(parent, root.GetHandle(), memento.siblingIndex);
            }
        }
    }

    inline void DestroySubtree(Scene* scene, UUID rootId)
    {
        Entity entity = FindEntityByUUID(scene, rootId);
        if (entity) scene->DestroyEntity(entity);
    }

    // ---- Commands ---------------------------------------------------------------

    // In-place edit of one entity's components (field edits, component add/remove).
    class ModifyEntityCommand : public Command {
    public:
        ModifyEntityCommand(UUID entity, EntitySnapshot before, EntitySnapshot after)
            : m_Entity(entity), m_Before(std::move(before)), m_After(std::move(after)) {}

        void Undo() override { RestoreEntity(SceneManager::GetContext(), m_Entity, m_Before); }
        void Redo() override { RestoreEntity(SceneManager::GetContext(), m_Entity, m_After); }
        const char* Name() const override { return "Modify Entity"; }

    private:
        UUID m_Entity;
        EntitySnapshot m_Before;
        EntitySnapshot m_After;
    };

    // Records the creation of an entity that has already been built in the scene.
    // Push (not Execute) this so the live entity is left as-is.
    class CreateEntityCommand : public Command {
    public:
        explicit CreateEntityCommand(Entity created) : m_Memento(CaptureSubtree(created)) {}

        void Undo() override { DestroySubtree(SceneManager::GetContext(), m_Memento.root.id); }
        void Redo() override { RestoreSubtree(SceneManager::GetContext(), m_Memento); }
        const char* Name() const override { return "Create Entity"; }

    private:
        SubtreeMemento m_Memento;
    };

    // Deletes an entity subtree. The constructor captures the subtree, so build this
    // and Execute it (which performs the deletion) rather than deleting separately.
    class DeleteEntityCommand : public Command {
    public:
        explicit DeleteEntityCommand(Entity entity) : m_Memento(CaptureSubtree(entity)) {}

        void Undo() override { RestoreSubtree(SceneManager::GetContext(), m_Memento); }
        void Redo() override { DestroySubtree(SceneManager::GetContext(), m_Memento.root.id); }
        const char* Name() const override { return "Delete Entity"; }

    private:
        SubtreeMemento m_Memento;
    };

    // Re-parents an entity. Built by RecordReparent after the move has been applied.
    class ReparentCommand : public Command {
    public:
        ReparentCommand(UUID child, UUID oldParent, int oldIndex, UUID newParent, int newIndex)
            : m_Child(child), m_OldParent(oldParent), m_OldIndex(oldIndex),
              m_NewParent(newParent), m_NewIndex(newIndex) {}

        void Undo() override { Apply(m_OldParent, m_OldIndex); }
        void Redo() override { Apply(m_NewParent, m_NewIndex); }
        const char* Name() const override { return "Reparent Entity"; }

    private:
        void Apply(UUID parentId, int index)
        {
            Scene* scene = SceneManager::GetContext();
            Entity child = FindEntityByUUID(scene, m_Child);
            if (!child) return;

            if ((uint64_t)parentId != 0)
            {
                Entity parent = FindEntityByUUID(scene, parentId);
                child.SetParent(parent);
                Detail::ReorderChild(parent, child.GetHandle(), index);
            }
            else
            {
                Entity none{};
                child.SetParent(none);
            }
        }

        UUID m_Child;
        UUID m_OldParent; int m_OldIndex;
        UUID m_NewParent; int m_NewIndex;
    };

    // ---- Recording helpers ------------------------------------------------------

    // Runs a component mutation (add/remove/edit) on an entity and records a single
    // ModifyEntityCommand if it changed anything. Use for discrete inspector actions.
    template <typename Fn>
    inline void RecordEntityEdit(Entity entity, Fn&& mutate)
    {
        if (!entity) { mutate(); return; }

        UUID id = entity.GetUUID();
        EntitySnapshot before = CaptureEntity(entity);
        mutate();
        EntitySnapshot after = CaptureEntity(entity);

        if (SnapshotsDiffer(before, after))
            CommandHistory::Global().Push(
                MakeUnique<ModifyEntityCommand>(id, std::move(before), std::move(after)));
    }

    inline int SiblingIndexOf(Entity entity)
    {
        entt::entity parentHandle = entity.GetParent();
        if (parentHandle == entt::null) return -1;
        Entity parent{ parentHandle, entity.GetScene() };
        if (!parent.HasComponent<ChildrenComponent>()) return -1;
        auto& children = parent.GetComponent<ChildrenComponent>().Children;
        auto it = std::find(children.begin(), children.end(), entity.GetHandle());
        return it != children.end() ? (int)std::distance(children.begin(), it) : -1;
    }

    // Applies a re-parent and records it. `newParent` may be a null entity to detach.
    inline void RecordReparent(Entity child, Entity newParent)
    {
        UUID childId = child.GetUUID();

        entt::entity oldParentHandle = child.GetParent();
        UUID oldParent = oldParentHandle != entt::null
            ? Entity{ oldParentHandle, child.GetScene() }.GetUUID() : UUID(0);
        int oldIndex = SiblingIndexOf(child);

        child.SetParent(newParent);

        UUID newParentId = newParent ? newParent.GetUUID() : UUID(0);
        int newIndex = SiblingIndexOf(child);

        CommandHistory::Global().Push(
            MakeUnique<ReparentCommand>(childId, oldParent, oldIndex, newParentId, newIndex));
    }

}
