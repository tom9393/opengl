#include "TextureImporter.h"
#include "AssetUploadQueue.h"
#include "Core/Application.h"
#include "AssetManager.h"

namespace Engine {

    Shared<Texture2D> TextureImporter::LoadTexture2D(const AssetMetadata &metadata)
    {
        return Texture2D::Create(metadata.FilePath);
    }

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

        // Assimp stores uncompressed data as bgra, we must convert to rgba
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
            // --- THIS RUNS ON A WORKER THREAD ---
            TextureLoadData data;
            // e.g., using stbi_load
            stbi_set_flip_vertically_on_load(1);
            data.Pixels = stbi_load(metadata.FilePath.c_str(), &data.Width, &data.Height, &data.Channels, 4);

            if (!data.Pixels) {
                std::cerr << "Failed to load texture: " << metadata.FilePath << std::endl;
                return; // Handle failure state appropriately
            }

            // 2. Submit the GPU upload back to the main thread
            AssetUploadQueue::Submit([metadata, data, cb]() {
                // --- THIS RUNS ON THE MAIN THREAD ---
                
                // Now it is safe to make OpenGL calls!
                Shared<Texture2D> texture = Texture2D::Create(data);
                
                // Clean up the CPU memory now that it's on the GPU
                stbi_image_free(data.Pixels);

                Shared<Asset> cast = texture;

                // Register the finalized asset
                AssetManager::RegisterAsset(metadata, cast); 

                if (cb) cb(texture);
            });
        });
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
