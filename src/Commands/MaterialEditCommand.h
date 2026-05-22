#pragma once

#include "pch.h"
#include "Command.h"
#include "CommandHistory.h"
#include "Asset/AssetManager.h"
#include "Asset/Material.h"

namespace Engine {

    // The user-editable state of a Material asset. Bindings and the shader are set at
    // load time and not touched by the inspector, so they are not snapshotted.
    struct MaterialState {
        BlendMode mode = BlendMode::Opaque;
        bool DepthWrite = true;
        bool DoubleSided = false;
        std::unordered_map<MaterialType, PropertyValue> properties;
    };

    inline MaterialState CaptureMaterial(const Shared<Material>& material)
    {
        MaterialState state;
        state.mode = material->mode;
        state.DepthWrite = material->DepthWrite;
        state.DoubleSided = material->DoubleSided;
        state.properties = material->m_Properties;
        return state;
    }

    inline bool MaterialStatesDiffer(const MaterialState& a, const MaterialState& b)
    {
        return a.mode != b.mode || a.DepthWrite != b.DepthWrite ||
               a.DoubleSided != b.DoubleSided || a.properties != b.properties;
    }

    // True if both states have the same set of property keys (ignoring values). A
    // differing set means a parameter/map was added or removed, which is recorded at
    // its action site rather than by the inspector value tracker.
    inline bool MaterialKeysEqual(const MaterialState& a, const MaterialState& b)
    {
        if (a.properties.size() != b.properties.size()) return false;
        for (const auto& [key, value] : a.properties)
            if (b.properties.find(key) == b.properties.end()) return false;
        return true;
    }

    // Undo/redo for a Material asset (parameters, maps, blend settings). The material
    // is resolved by handle so the command survives asset reloads.
    class MaterialEditCommand : public Command {
    public:
        MaterialEditCommand(UUID handle, MaterialState before, MaterialState after)
            : m_Handle(handle), m_Before(std::move(before)), m_After(std::move(after)) {}

        void Undo() override { Restore(m_Before); }
        void Redo() override { Restore(m_After); }
        const char* Name() const override { return "Edit Material"; }

    private:
        void Restore(const MaterialState& state)
        {
            Shared<Material> material = AssetManager::GetAsset<Material>(m_Handle);
            if (!material) return;
            material->mode = state.mode;
            material->DepthWrite = state.DepthWrite;
            material->DoubleSided = state.DoubleSided;
            material->m_Properties = state.properties;
        }

        UUID m_Handle;
        MaterialState m_Before;
        MaterialState m_After;
    };

    // Runs a structural material change (add/remove a parameter or map) and records a
    // single MaterialEditCommand if it changed anything. Use for discrete actions; the
    // inspector value tracker handles in-place parameter edits.
    template <typename Fn>
    inline void RecordMaterialEdit(UUID handle, Fn&& mutate)
    {
        Shared<Material> material = AssetManager::GetAsset<Material>(handle);
        if (!material) { mutate(); return; }

        MaterialState before = CaptureMaterial(material);
        mutate();
        MaterialState after = CaptureMaterial(material);

        if (MaterialStatesDiffer(before, after))
            CommandHistory::Global().Push(
                MakeUnique<MaterialEditCommand>(handle, std::move(before), std::move(after)));
    }

}
