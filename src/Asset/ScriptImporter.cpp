#include "ScriptImporter.h"

namespace Engine {

    Shared<AssetMetadata> ScriptImporter::ImportLuaScript(const std::filesystem::path &path)
    {
        Shared<AssetMetadata> metadata = MakeShared<AssetMetadata>();
        metadata->Handle = UUID();
        metadata->FilePath = path.string();
        metadata->Name = path.stem().string();
        metadata->Type = AssetType::LuaScript;

        return metadata;
    }
}
