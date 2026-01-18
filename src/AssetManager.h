#pragma once

#include "Asset.h"
#include "UUID.h"

namespace Engine {

    class AssetManager
    {
        public:
        static Shared<Asset> LoadAsset(const std::filesystem::path &path);

        private:
        static Shared<Asset> GetByType(const std::filesystem::path &path);
        static bool IsLoaded(UUID id);

        private:
        static inline std::unordered_map<UUID, Shared<Asset>> m_Loaded;
        static inline std::unordered_map<UUID, std::string> m_Paths;
    };
}
