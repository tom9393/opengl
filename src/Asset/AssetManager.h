#pragma once

#include "Asset.h"
#include "../Core/UUID.h"

namespace Engine {

    class AssetManager
    {
        public:
        static Shared<Asset> LoadAsset(const std::filesystem::path &path);

        private:
        static Shared<Asset> GetByType(const std::filesystem::path &path);
        static Shared<Mesh> GetSubMesh(const std::filesystem::path &path, int subMeshIndex);
        static bool IsLoaded(const std::string &path);
        //static bool IsLoaded(UUID id);

        private:
        static inline std::unordered_map<std::string, Shared<Asset>> m_Loaded; // For now, later on make the system use UUIDs
        // static inline std::unordered_map<UUID, Shared<Asset>> m_Loaded;
        // static inline std::unordered_map<UUID, std::string> m_Paths;
    };
}
