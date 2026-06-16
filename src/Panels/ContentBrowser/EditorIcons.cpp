#include "EditorIcons.h"
#include "Asset/TextureImporter.h"

namespace Engine {

    Shared<Texture2D> EditorIcons::s_File;
    Shared<Texture2D> EditorIcons::s_Folder;
    Shared<Texture2D> EditorIcons::s_Lua;
    Shared<Texture2D> EditorIcons::s_Audio;

    Shared<Texture2D> EditorIcons::Load(const std::filesystem::path& path) {
        TextureLoadData data;
        stbi_set_flip_vertically_on_load(1);
        data.Pixels = stbi_load(path.string().c_str(), &data.Width, &data.Height, &data.Channels, 4);

        if (!data.Pixels) {
            std::cerr << "Failed to load icon: " << path << std::endl;
            return nullptr;
        }

        Shared<Texture2D> texture = Texture2D::Create(data);
        stbi_image_free(data.Pixels);
        return texture;
    }

    void EditorIcons::Init() {
        // Todo; pathing
        s_File   = Load("../assets/icons/documentIcon.png");
        s_Folder = Load("../assets/icons/folderIcon.png");
        s_Lua    = Load("../assets/icons/luaIcon.png");
        s_Audio  = Load("../assets/icons/audioIcon.png");
    }

    void EditorIcons::Shutdown() {
        s_File.reset();
        s_Folder.reset();
        s_Lua.reset();
        s_Audio.reset();
    }

}
