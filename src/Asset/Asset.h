#pragma once

#include "Core/UUID.h"

namespace Engine {

    enum class AssetType
    {
        None = 0,
        Texture2D,
        Texture3D,
        Scene,
        Prefab,
        Mesh,
        Material,
        LuaScript,
        Audio
    };

    inline AssetType AssetTypeFromFilepath(const std::filesystem::path& path)
    {
        std::string ext = path.extension().string();

        if (ext == ".pmblend") return AssetType::Scene;
        if (ext == ".pmscene") return AssetType::Scene;
        if (ext == ".png")     return AssetType::Texture2D;
        if (ext == ".jpg")     return AssetType::Texture2D;
        if (ext == ".jpeg")    return AssetType::Texture2D;
        if (ext == ".ktx")     return AssetType::Texture2D;
        if (ext == ".ktx2")    return AssetType::Texture2D;
        if (ext == ".assbin")  return AssetType::Scene;
        if (ext == ".obj")     return AssetType::Scene;
        if (ext == ".fbx")     return AssetType::Scene;
        if (ext == ".gltf")    return AssetType::Scene;
        if (ext == ".prefab")  return AssetType::Prefab;
        if (ext == ".lua")     return AssetType::LuaScript;
        if (ext == ".wav")     return AssetType::Audio;
        if (ext == ".mp3")     return AssetType::Audio;
        if (ext == ".flac")    return AssetType::Audio;

        return AssetType::None;
    }

    inline std::string AssetTypeToString(AssetType type)
	{
		switch (type)
		{
			case AssetType::None:      return "AssetType::None";
			case AssetType::Scene:     return "AssetType::Scene";
			case AssetType::Mesh: return "AssetType::Mesh";
			case AssetType::Texture2D: return "AssetType::Texture2D";
			case AssetType::Material: return "AssetType::Material";
			case AssetType::Prefab: return "AssetType::Prefab";
			case AssetType::LuaScript: return "AssetType::LuaScript";
			case AssetType::Audio: return "AssetType::Audio";
		}

		return "AssetType::<Invalid>";
	}

	inline AssetType AssetTypeFromString(std::string_view assetType)
	{
		if (assetType == "AssetType::None")      return AssetType::None;
		if (assetType == "AssetType::Scene")     return AssetType::Scene;
		if (assetType == "AssetType::Mesh") return AssetType::Mesh;
		if (assetType == "AssetType::Texture2D") return AssetType::Texture2D;
		if (assetType == "AssetType::Material") return AssetType::Material;
		if (assetType == "AssetType::Prefab") return AssetType::Prefab;
		if (assetType == "AssetType::LuaScript") return AssetType::LuaScript;
		if (assetType == "AssetType::Audio") return AssetType::Audio;

		return AssetType::None;
	}

    class Asset
    {
        public:
        UUID Handle;
        AssetType GetType() { return m_Type; };

        protected:
        bool m_Loaded = false;
        AssetType m_Type;
    };

    struct AssetMetadata {
        UUID Handle = 0;
        AssetType Type = AssetType::None;
        bool IsMemoryAsset = false;
        std::string FilePath;
        std::string Name = "";
        
        bool IsValid() const { return Handle != 0; }
    };

}
