#include "ModelImporter.h"
#include "AssimpLoader.h"

namespace Engine {

    Shared<Scene> ModelImporter::LoadModel(const std::filesystem::path &path)
    {
        return AssimpLoader::Load(path);
    }
}
