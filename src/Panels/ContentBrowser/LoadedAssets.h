#pragma once
#include "Renderer.h"

namespace Engine {

    class LoadedAssets
    {
        public:
        LoadedAssets();
        void OnImGuiRender();

        private:
        Shared<Texture2D> m_FileIcon;
    };
}