#pragma once

#include <optional>
#include <tuple>

#include "pch.h"
#include "Scene/Components.h"
#include "Scene/Entity.h"
#include "Scene/Scene.h"

namespace Engine {

    namespace Detail {
        template <typename... Component>
        std::tuple<std::optional<Component>...> SnapshotTupleFor(ComponentGroup<Component...>);
    }

    // A copy of an entity's non-structural components (one optional per entry in
    // AllComponents). Identity (IDComponent) and hierarchy (Parent/Children) are
    // deliberately excluded: identity never changes, and hierarchy is managed by
    // the structural commands so restoring a snapshot never disturbs an entity's
    // place in the tree.
    struct EntitySnapshot {
        decltype(Detail::SnapshotTupleFor(AllComponents{})) Components;

        template <typename T>
        std::optional<T>& Get() { return std::get<std::optional<T>>(Components); }

        template <typename T>
        const std::optional<T>& Get() const { return std::get<std::optional<T>>(Components); }
    };

    inline Entity FindEntityByUUID(Scene* scene, UUID id)
    {
        if (!scene) return {};
        auto view = scene->GetRegistry().view<IDComponent>();
        for (auto e : view)
            if ((uint64_t)view.get<IDComponent>(e).id == (uint64_t)id)
                return Entity{ e, scene };
        return {};
    }

    namespace Detail {

        template <typename T>
        inline void CaptureOptional(Entity entity, std::optional<T>& out)
        {
            if (entity.HasComponent<T>()) out = entity.GetComponent<T>();
            else out.reset();
        }

        template <typename T>
        inline void ApplyOptional(entt::registry& registry, entt::entity handle, const std::optional<T>& opt)
        {
            if (opt) registry.emplace_or_replace<T>(handle, *opt);
            else if (registry.all_of<T>(handle)) registry.remove<T>(handle);
        }

        // "Editable equality" per component: compares only the fields a user can
        // change in the inspector, ignoring runtime/derived state (world matrices,
        // runtime bodies, script environments). Used solely to decide whether an
        // interaction changed anything worth recording on the undo stack.

        inline bool EditablyEqual(const TagComponent& a, const TagComponent& b)
        {
            return a.tag == b.tag;
        }

        inline bool EditablyEqual(const TransformComponent& a, const TransformComponent& b)
        {
            return a.translation == b.translation &&
                   a.rotation    == b.rotation &&
                   a.scale       == b.scale;
        }

        inline bool EditablyEqual(const MeshComponent& a, const MeshComponent& b)
        {
            return (uint64_t)a.MeshHandle == (uint64_t)b.MeshHandle;
        }

        inline bool EditablyEqual(const TextureComponent& a, const TextureComponent& b)
        {
            return (uint64_t)a.TextureHandle == (uint64_t)b.TextureHandle;
        }

        inline bool EditablyEqual(const MaterialComponent& a, const MaterialComponent& b)
        {
            return (uint64_t)a.MaterialHandle == (uint64_t)b.MaterialHandle;
        }

        inline bool EditablyEqual(const SceneCameraComponent& a, const SceneCameraComponent& b)
        {
            return a.primary          == b.primary &&
                   a.fixedAspectRatio == b.fixedAspectRatio;
        }

        inline bool EditablyEqual(const RigidBodyComponent& a, const RigidBodyComponent& b)
        {
            return a.type    == b.type &&
                   a.mass    == b.mass &&
                   a.inertia == b.inertia;
        }

        // No comparable state (bound via std::function); presence is tracked, values are not.
        inline bool EditablyEqual(const NativeScriptComponent&, const NativeScriptComponent&)
        {
            return true;
        }

        inline bool EditablyEqual(const LuaScriptComponent& a, const LuaScriptComponent& b)
        {
            return (uint64_t)a.ScriptHandle == (uint64_t)b.ScriptHandle;
        }

        inline bool EditablyEqual(const LightComponent& a, const LightComponent& b)
        {
            return a.Type        == b.Type &&
                   a.Color       == b.Color &&
                   a.Intensity   == b.Intensity &&
                   a.CastShadows == b.CastShadows &&
                   a.InnerCutOff == b.InnerCutOff &&
                   a.OuterCutOff == b.OuterCutOff;
        }

        // Runtime (the live particle pool) is intentionally ignored.
        inline bool EditablyEqual(const ParticleEmitterComponent& a, const ParticleEmitterComponent& b)
        {
            return a.Looping       == b.Looping &&
                   a.Rate          == b.Rate &&
                   a.BurstCount    == b.BurstCount &&
                   a.MaxParticles  == b.MaxParticles &&
                   a.Lifetime      == b.Lifetime &&
                   a.Direction     == b.Direction &&
                   a.Spread        == b.Spread &&
                   a.Speed         == b.Speed &&
                   a.Gravity       == b.Gravity &&
                   a.StartSize     == b.StartSize &&
                   a.EndSize       == b.EndSize &&
                   a.StartColor    == b.StartColor &&
                   a.EndColor      == b.EndColor &&
                   a.Additive      == b.Additive &&
                   (uint64_t)a.TextureHandle == (uint64_t)b.TextureHandle;
        }

        template <typename T>
        inline bool OptionalDiffers(const std::optional<T>& a, const std::optional<T>& b)
        {
            if (a.has_value() != b.has_value()) return true;
            return a && !EditablyEqual(*a, *b);
        }

        template <typename... Component>
        inline EntitySnapshot CaptureEntity(Entity entity, ComponentGroup<Component...>)
        {
            EntitySnapshot snap;
            (CaptureOptional<Component>(entity, snap.Get<Component>()), ...);
            return snap;
        }

        template <typename... Component>
        inline void RestoreEntity(entt::registry& registry, entt::entity handle, const EntitySnapshot& snap, ComponentGroup<Component...>)
        {
            (ApplyOptional<Component>(registry, handle, snap.Get<Component>()), ...);
        }

        template <typename... Component>
        inline bool SameComponentSet(const EntitySnapshot& a, const EntitySnapshot& b, ComponentGroup<Component...>)
        {
            return ((a.Get<Component>().has_value() == b.Get<Component>().has_value()) && ...);
        }

        template <typename... Component>
        inline bool SnapshotsDiffer(const EntitySnapshot& a, const EntitySnapshot& b, ComponentGroup<Component...>)
        {
            return (OptionalDiffers(a.Get<Component>(), b.Get<Component>()) || ...);
        }

    }

    inline EntitySnapshot CaptureEntity(Entity entity)
    {
        return Detail::CaptureEntity(entity, AllComponents{});
    }

    // Writes the snapshot onto the entity, adding/removing components so the set
    // matches what was captured. Operates on the registry directly (bypassing the
    // logging/guards in Entity::Add/RemoveComponent).
    inline void RestoreEntity(Scene* scene, UUID id, const EntitySnapshot& snap)
    {
        Entity entity = FindEntityByUUID(scene, id);
        if (!entity) return;

        Detail::RestoreEntity(scene->GetRegistry(), entity.GetHandle(), snap, AllComponents{});
    }

    // True if both snapshots have exactly the same set of components present (ignoring
    // values). A differing set means a component was added/removed, which is recorded
    // explicitly at the action site rather than by the inspector value tracker.
    inline bool SameComponentSet(const EntitySnapshot& a, const EntitySnapshot& b)
    {
        return Detail::SameComponentSet(a, b, AllComponents{});
    }

    // True if the user-editable state differs between two snapshots of one entity.
    // Used to discard no-op inspector interactions so they don't pollute the stack.
    inline bool SnapshotsDiffer(const EntitySnapshot& a, const EntitySnapshot& b)
    {
        return Detail::SnapshotsDiffer(a, b, AllComponents{});
    }

}
