#pragma once
#include "../../pch.h"
#include "../Texture.h"
#include "../Preprocessor.h"

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
