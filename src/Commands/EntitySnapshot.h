#pragma once

#include <optional>

#include "pch.h"
#include "Scene/Components.h"
#include "Scene/Entity.h"
#include "Scene/Scene.h"

namespace Engine {

    // A copy of an entity's non-structural components. Identity (IDComponent) and
    // hierarchy (Parent/Children) are deliberately excluded: identity never changes,
    // and hierarchy is managed by the structural commands so restoring a snapshot
    // never disturbs an entity's place in the tree.
    struct EntitySnapshot {
        std::optional<TagComponent>          Tag;
        std::optional<TransformComponent>    Transform;
        std::optional<MeshComponent>         Mesh;
        std::optional<TextureComponent>      Texture;
        std::optional<MaterialComponent>     Material;
        std::optional<SceneCameraComponent>  Camera;
        std::optional<RigidBodyComponent>    RigidBody;
        std::optional<NativeScriptComponent> NativeScript;
        std::optional<LuaScriptComponent>    Lua;
        std::optional<LightComponent>        Light;
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
    }

    inline EntitySnapshot CaptureEntity(Entity entity)
    {
        EntitySnapshot snap;
        Detail::CaptureOptional(entity, snap.Tag);
        Detail::CaptureOptional(entity, snap.Transform);
        Detail::CaptureOptional(entity, snap.Mesh);
        Detail::CaptureOptional(entity, snap.Texture);
        Detail::CaptureOptional(entity, snap.Material);
        Detail::CaptureOptional(entity, snap.Camera);
        Detail::CaptureOptional(entity, snap.RigidBody);
        Detail::CaptureOptional(entity, snap.NativeScript);
        Detail::CaptureOptional(entity, snap.Lua);
        Detail::CaptureOptional(entity, snap.Light);
        return snap;
    }

    // Writes the snapshot onto the entity, adding/removing components so the set
    // matches what was captured. Operates on the registry directly (bypassing the
    // logging/guards in Entity::Add/RemoveComponent).
    inline void RestoreEntity(Scene* scene, UUID id, const EntitySnapshot& snap)
    {
        Entity entity = FindEntityByUUID(scene, id);
        if (!entity) return;

        entt::registry& reg = scene->GetRegistry();
        entt::entity handle = entity.GetHandle();

        Detail::ApplyOptional(reg, handle, snap.Tag);
        Detail::ApplyOptional(reg, handle, snap.Transform);
        Detail::ApplyOptional(reg, handle, snap.Mesh);
        Detail::ApplyOptional(reg, handle, snap.Texture);
        Detail::ApplyOptional(reg, handle, snap.Material);
        Detail::ApplyOptional(reg, handle, snap.Camera);
        Detail::ApplyOptional(reg, handle, snap.RigidBody);
        Detail::ApplyOptional(reg, handle, snap.NativeScript);
        Detail::ApplyOptional(reg, handle, snap.Lua);
        Detail::ApplyOptional(reg, handle, snap.Light);
    }

    // True if both snapshots have exactly the same set of components present (ignoring
    // values). A differing set means a component was added/removed, which is recorded
    // explicitly at the action site rather than by the inspector value tracker.
    inline bool SameComponentSet(const EntitySnapshot& a, const EntitySnapshot& b)
    {
        return a.Tag.has_value()          == b.Tag.has_value()          &&
               a.Transform.has_value()    == b.Transform.has_value()    &&
               a.Mesh.has_value()         == b.Mesh.has_value()         &&
               a.Texture.has_value()      == b.Texture.has_value()      &&
               a.Material.has_value()     == b.Material.has_value()     &&
               a.Camera.has_value()       == b.Camera.has_value()       &&
               a.RigidBody.has_value()    == b.RigidBody.has_value()    &&
               a.NativeScript.has_value() == b.NativeScript.has_value() &&
               a.Lua.has_value()          == b.Lua.has_value()          &&
               a.Light.has_value()        == b.Light.has_value();
    }

    // True if the user-editable state differs between two snapshots of one entity.
    // Used to discard no-op inspector interactions so they don't pollute the stack.
    inline bool SnapshotsDiffer(const EntitySnapshot& a, const EntitySnapshot& b)
    {
        if (a.Tag.has_value()          != b.Tag.has_value())          return true;
        if (a.Transform.has_value()    != b.Transform.has_value())    return true;
        if (a.Mesh.has_value()         != b.Mesh.has_value())         return true;
        if (a.Texture.has_value()      != b.Texture.has_value())      return true;
        if (a.Material.has_value()     != b.Material.has_value())     return true;
        if (a.Camera.has_value()       != b.Camera.has_value())       return true;
        if (a.RigidBody.has_value()    != b.RigidBody.has_value())    return true;
        if (a.NativeScript.has_value() != b.NativeScript.has_value()) return true;
        if (a.Lua.has_value()          != b.Lua.has_value())          return true;
        if (a.Light.has_value()        != b.Light.has_value())        return true;

        if (a.Tag && a.Tag->tag != b.Tag->tag) return true;

        if (a.Transform && (a.Transform->translation != b.Transform->translation ||
                            a.Transform->rotation    != b.Transform->rotation ||
                            a.Transform->scale       != b.Transform->scale))
            return true;

        if (a.Mesh && (uint64_t)a.Mesh->MeshHandle != (uint64_t)b.Mesh->MeshHandle) return true;
        if (a.Texture && (uint64_t)a.Texture->TextureHandle != (uint64_t)b.Texture->TextureHandle) return true;
        if (a.Material && (uint64_t)a.Material->MaterialHandle != (uint64_t)b.Material->MaterialHandle) return true;

        if (a.Camera && (a.Camera->primary           != b.Camera->primary ||
                         a.Camera->fixedAspectRatio  != b.Camera->fixedAspectRatio))
            return true;

        if (a.RigidBody && (a.RigidBody->type    != b.RigidBody->type ||
                            a.RigidBody->mass    != b.RigidBody->mass ||
                            a.RigidBody->inertia != b.RigidBody->inertia))
            return true;

        if (a.Light && (a.Light->Type        != b.Light->Type ||
                        a.Light->Color       != b.Light->Color ||
                        a.Light->Intensity   != b.Light->Intensity ||
                        a.Light->CastShadows != b.Light->CastShadows ||
                        a.Light->InnerCutOff != b.Light->InnerCutOff ||
                        a.Light->OuterCutOff != b.Light->OuterCutOff))
            return true;

        if (a.Lua && (uint64_t)a.Lua->ScriptHandle != (uint64_t)b.Lua->ScriptHandle) return true;

        return false;
    }

}
