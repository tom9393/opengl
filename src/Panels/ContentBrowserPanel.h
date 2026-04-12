#pragma once
#include "Subpanels/Directory.h"
#include "Subpanels/LoadedAssets.h"

namespace Engine
{

    enum class Tab
    {
        Directory,
        LoadedAssets
    };

    class ContentBrowserPanel
    {
        public:
        ContentBrowserPanel();
        void OnImGuiRender();

        private:
        Directory m_Directory;
        LoadedAssets m_LoadedAssets;

        Tab m_Tab = Tab::LoadedAssets;
    };

};
