#pragma once

#include "../../../pch.h"
#include "../../Asset/Texture.h"
#include "../../Asset/AssetManager.h"
#include "Renderer.h"

namespace Engine {

    struct Entry
    {
        std::filesystem::path Path;
        std::string Name;
        bool IsDirectory;
    };

    class Directory
    {
        public:
        Directory();
        void OnImGuiRender();

        private:
        void RefreshCache();

        std::filesystem::path m_CurrentDirectory;
        std::vector<Entry> m_EditorCache;
        Shared<Texture2D> m_DirIcon;
        Shared<Texture2D> m_FileIcon;
    };
}