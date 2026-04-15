#include "ScriptImporter.h"

namespace Engine {

    AssetMetadata ScriptImporter::ImportLuaScript(const std::filesystem::path &path)
    {
        AssetMetadata metadata;
        metadata.Handle = UUID();
        metadata.FilePath = path.string();
        metadata.Name = path.stem().string();
        metadata.Type = AssetType::LuaScript;

        return metadata;
    }
}
