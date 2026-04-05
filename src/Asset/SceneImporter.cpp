#include "SceneImporter.h"
#include "../Asset/AssimpLoader.h"

namespace Engine {

    Shared<Scene> SceneImporter::LoadScene(const std::filesystem::path &path)
    {
        Shared<Scene> scene = MakeShared<Scene>();
        SceneSerializer serializer(scene);
        serializer.Deserialize(path);
        return scene;
    }
}
