#pragma once

#include "../../../pch.h"
#include "../../Asset/Texture.h"
#include "../../Asset/AssetManager.h"
#include "Renderer.h"

namespace Engine {

    class Directory
    {
        public:
        Directory();
        void OnImGuiRender();

        private:
        std::filesystem::path m_CurrentDirectory;
        Shared<Texture2D> m_DirIcon;
        Shared<Texture2D> m_FileIcon;
    };
}