#pragma once

#include "pch.h"
#include "Asset.h"

namespace Engine {
    
    class ScriptImporter
    {
        public:
        static Shared<AssetMetadata> ImportLuaScript(const std::filesystem::path &path);
    };
}