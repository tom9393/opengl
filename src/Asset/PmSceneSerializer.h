#pragma once

#include "pch.h"
#include "ParsedScene.h"

namespace Engine
{
    class Prefab;

    // Reads/writes the binary .pmscene format: a self-contained bake of an imported model.
    // It embeds geometry, lights, cameras, hierarchy and materials, references external
    // textures by path, and embeds in-model (file-less) textures' pixel bytes. A .pmscene is
    // a binary serialization of a ParsedScene, so loading reuses SceneBuilder::BuildPrefab.
    class PmSceneSerializer
    {
        public:
        // Bakes a live (loaded) prefab to disk.
        static void Serialize(const Shared<Prefab>& prefab, const std::string& path);
        // Reads a .pmscene back into a ParsedScene ready for SceneBuilder::BuildPrefab.
        static std::optional<ParsedScene> Deserialize(const std::string& path);
    };
}
