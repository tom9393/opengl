#include "ModelImporter.h"
#include "AssimpLoader.h"

namespace Engine {

    Shared<Model> ModelImporter::LoadModel(const std::filesystem::path &path)
    {
        AssimpLoader loader; // Temp
        return loader.Load(path.c_str());
    }
}
