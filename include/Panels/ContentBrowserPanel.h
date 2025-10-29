#pragma once
#include <filesystem>
#include "../Texture.h"
#include "../preprocessor.h"

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
