#pragma once

#include "pch.h"
#include "Asset/Texture.h"

namespace Engine {

    class EditorIcons {
    public:
        static void Init();
        static void Shutdown();

        static const Shared<Texture2D>& File()   { return s_File; }
        static const Shared<Texture2D>& Folder() { return s_Folder; }
        static const Shared<Texture2D>& Lua()    { return s_Lua; }
        static const Shared<Texture2D>& Audio()  { return s_Audio; }

    private:
        static Shared<Texture2D> Load(const std::filesystem::path& path);

        static Shared<Texture2D> s_File;
        static Shared<Texture2D> s_Folder;
        static Shared<Texture2D> s_Lua;
        static Shared<Texture2D> s_Audio;
    };

}
