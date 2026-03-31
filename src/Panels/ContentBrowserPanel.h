#pragma once
#include "../../pch.h"
#include "../Asset/Texture.h"
#include "../Core/Preprocessor.h"

namespace Engine
{

    class ContentBrowserPanel
    {
        public:

        ContentBrowserPanel();

        void OnImGuiRender();

        private:
        std::filesystem::path m_CurrentDirectory;
        Shared<Texture2D> m_DirIcon;
        Shared<Texture2D> m_FileIcon;

    };

};
