#include "TextureImporter.h"
#include "AssetManager.h"
#include "AssetUploadQueue.h"
#include "Core/Application.h"

namespace Engine {

    // For external textures i.e. files
    Shared<Texture2D> TextureImporter::LoadTexture2D(const AssetMetadata &metadata)
    {
        TextureLoadData loadData;
        stbi_set_flip_vertically_on_load(1);
        loadData.Pixels = stbi_load(metadata.FilePath.c_str(), &loadData.Width, &loadData.Height, &loadData.Channels, 4);

        if (!loadData.Pixels) {
            std::cerr << "Failed to load texture: " << metadata.FilePath << std::endl;
            return nullptr;
        }

        Shared<Texture2D> texture = Texture2D::Create(loadData);
        stbi_image_free(loadData.Pixels);
        return texture;
    }

    // For embedded textures
    TextureLoadData TextureImporter::DecodeTextureFromMemory(const void* data, size_t length) {
        TextureLoadData loadData;
        stbi_set_flip_vertically_on_load(1);
        loadData.Pixels = stbi_load_from_memory((const stbi_uc*)data, (int)length, &loadData.Width, &loadData.Height, &loadData.Channels, 4);
        return loadData;
    }

    // Decodes uncompressed embedded textures (Raw Arrays)
    TextureLoadData TextureImporter::DecodeTextureUncompressed(const void* data, uint32_t width, uint32_t height) {
        TextureLoadData loadData;
        loadData.Width = width;
        loadData.Height = height;
        loadData.Channels = 4;
        
        size_t size = width * height * 4;
        loadData.Pixels = (unsigned char*)malloc(size);

        // Assimp stores uncompressed data as bgra, convert to rgba
        struct aiTexel { unsigned char b, g, r, a; };
        const aiTexel* texels = (const aiTexel*)data;

        for (size_t i = 0; i < width * height; i++) {
            loadData.Pixels[i*4 + 0] = texels[i].r;
            loadData.Pixels[i*4 + 1] = texels[i].g;
            loadData.Pixels[i*4 + 2] = texels[i].b;
            loadData.Pixels[i*4 + 3] = texels[i].a;
        }
        return loadData;
    }

    void TextureImporter::FreeTextureData(const TextureLoadData& data) {
        if (data.Pixels) {
            stbi_image_free(data.Pixels); 
        }
    }

    void TextureImporter::LoadTexture2DAsync(const AssetMetadata metadata, std::function<void(Shared<Asset>)> cb)
    {
        PROFILE_FUNCTION();
        Application::Get().GetThreadPool().Enqueue([metadata, cb]() {
            TextureLoadData loadData;
            stbi_set_flip_vertically_on_load(1);
            loadData.Pixels = stbi_load(metadata.FilePath.c_str(), &loadData.Width, &loadData.Height, &loadData.Channels, 4);

            if (!loadData.Pixels) {
                std::cerr << "Failed to load texture: " << metadata.FilePath << std::endl;
                return;
            }

            AssetUploadQueue::Submit([metadata, loadData, cb]() {
                Shared<Texture2D> texture = Texture2D::Create(loadData);
                stbi_image_free(loadData.Pixels);
                Shared<Asset> cast = texture;

                AssetManager::RegisterAsset(metadata, cast); 

                if (cb) cb(texture);
            });
        });
    }

    // HDR i.e. skybox
    Shared<Texture2D> TextureImporter::LoadHDR(const std::string& path)
    {
        stbi_set_flip_vertically_on_load(true);
        int width, height, channels;
        float* data = stbi_loadf(path.c_str(), &width, &height, &channels, 3);
        if (!data)
            return nullptr;

        TextureLoadData hdrData;
        hdrData.HDRPixels = data;
        hdrData.Width     = width;
        hdrData.Height    = height;
        hdrData.Channels  = 3;
        hdrData.clamp     = true;
        Shared<Texture2D> texture = Texture2D::Create(hdrData);
        stbi_image_free(data);

        return texture;
    }

    Shared<AssetMetadata> TextureImporter::ImportTexture2D(const std::filesystem::path &path)
    {
        Shared<AssetMetadata> metadata = MakeShared<AssetMetadata>();
        metadata->Handle = UUID();
        metadata->FilePath = path.string();
        metadata->Name = path.stem().string();
        metadata->Type = AssetType::Texture2D;

        return metadata;
    }
}
