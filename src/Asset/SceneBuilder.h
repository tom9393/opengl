#pragma once

#include "pch.h"
#include "AssetManager.h"
#include "Asset/Prefab.h"
#include "ParsedScene.h"
#include "MatdefLoader.h"
#include "Material.h"
#include "Mesh.h"
#include "Render/BufferLayout.h"
#include "Render/Renderer.h"
#include "Scene/Entity.h"
#include "Texture.h"
#include "TextureImporter.h"

namespace Engine
{

    // Turns a parsed scene tree into prefab; upload materials/meshes/textures to GPU and build entity hierarchy. Run on main thread
    class SceneBuilder
    {
        public:
        static Shared<Prefab> BuildPrefab(const AssetMetadata& metadata, const ParsedScene& data) {
            Shared<Prefab> prefab = MakeShared<Prefab>();

            // Materials are shared across the whole model: upload each once and reference it by its global index when building hierarchy
            std::vector<UUID> materialHandles(data.Materials.size());
            for (size_t i = 0; i < data.Materials.size(); i++)
                materialHandles[i] = CreateMaterialAsset(data.Materials[i], data);

            Entity nullParent{};
            BuildEntityHierarchy(data.RootNode, prefab, nullParent, data, materialHandles, data.Name);

            Shared<Asset> prefabCast = prefab;
            AssetManager::RegisterAsset(metadata, prefabCast);
            std::cout << "Successfully built model: " << metadata.FilePath << std::endl;
            return prefab;
        }

    private:
        static UUID CreateMaterialAsset(const ParsedMaterial& matData, const ParsedScene& data) {
            Shared<Material> material = MatdefLoader::LoadMaterial("/home/tom/Coding/c++/opengl/shaders/PBR_Standard.matdef"); // For now hardcoded

            material->Add({MaterialType::AlbedoTint, matData.AlbedoTint});
            material->Add({MaterialType::Emission, matData.Emission});
            material->Add({MaterialType::RoughnessMultiplier, matData.Roughness});
            material->Add({MaterialType::MetallicMultiplier, matData.Metallic});


            for (const auto& [type, path] : matData.TexturePaths) {
                // Imported models give paths relative to the model directory; baked .pmscene
                // files store the full path already (empty Directory).
                std::string texturePath = data.Directory.empty() ? path : (data.Directory + "/" + path);
                if (!std::filesystem::exists(texturePath)) continue;

                UUID texHandle = AssetManager::ImportAsset(texturePath);
                AssetMetadata metadata = AssetManager::GetMetadata(texHandle);

                TextureImporter::LoadTexture2DAsync(metadata, [material, type, texHandle](Shared<Asset> asset) {
                    Shared<Texture2D> img = std::static_pointer_cast<Texture2D>(asset);
                    material->Add({type, img});
                });
            }

            for (auto& [type, loadData] : matData.EmbeddedTextures) {
                Shared<Texture2D> texture = Texture2D::Create(loadData);

                TextureImporter::FreeTextureData(loadData);

                AssetMetadata metadata;
                metadata.Handle = UUID();
                metadata.Name = "EmbeddedTexture";
                metadata.FilePath = "Temp";
                metadata.Type = AssetType::Texture2D;
                metadata.IsMemoryAsset = true;

                Shared<Asset> texCast = texture;
                UUID texHandle = AssetManager::RegisterAsset(metadata, texCast);
                material->Add({type, texture});
            }

            AssetMetadata matMeta;
            matMeta.Handle = UUID(matData.Hash);
            matMeta.Name = data.Name + "_" + matData.Name;
            matMeta.Type = AssetType::Material;
            matMeta.IsMemoryAsset = true;
            Shared<Asset> assetCast = material;
            return AssetManager::RegisterAsset(matMeta, assetCast);
        }

        static UUID CreateMeshAsset(const ParsedMesh& meshData, const ParsedScene& data) {
            Shared<Mesh> mesh = MakeShared<Mesh>();
            mesh->GetVertices() = meshData.Vertices;
            mesh->GetIndices() = meshData.Indices;

            Shared<VertexArray> VAO = VertexArray::Create();
            Shared<VertexBuffer> VBO = VertexBuffer::Create((float*)meshData.Vertices.data(), meshData.Vertices.size() * sizeof(float));

            VBO->SetLayout(meshData.Layout);

            VAO->AddVertexBuffer(VBO);
            Shared<IndexBuffer> EBO = IndexBuffer::Create((unsigned int*)meshData.Indices.data(), meshData.Indices.size());
            VAO->SetIndexBuffer(EBO);

            mesh->SetVAO(VAO);
            mesh->SetVBOs({VBO});
            mesh->SetEBO(EBO);
            mesh->SetAABB({ meshData.MinExtents, meshData.MaxExtents });

            AssetMetadata meshMeta;
            meshMeta.Handle = UUID(meshData.Hash);
            meshMeta.Name = data.Name + "_" + meshData.Name;
            meshMeta.Type = AssetType::Mesh;
            meshMeta.IsMemoryAsset = true;
            Shared<Asset> assetCast = mesh;
            return AssetManager::RegisterAsset(meshMeta, assetCast);
        }

        static void BuildEntityHierarchy(const ParsedSceneNode& nodeData, Shared<Prefab>& prefab, Entity parentEntity, const ParsedScene& fullData, const std::vector<UUID>& materialHandles, const std::string& nameOverride = "") {
            Entity currentEntity = prefab->CreateEntity(nameOverride.empty() ? nodeData.Name : nameOverride);
            currentEntity.SetParent(parentEntity);

            auto& transform = currentEntity.AddComponent<TransformComponent>();
            transform.translation = nodeData.Translation;
            transform.rotation = nodeData.Rotation;
            transform.scale = nodeData.Scale;

            if (nodeData.Light) AttachLight(currentEntity, *nodeData.Light);
            if (nodeData.Camera) AttachCamera(currentEntity, *nodeData.Camera);

            for (const ParsedMesh& meshData : nodeData.Meshes) {
                Entity meshEntity = (nodeData.Meshes.size() == 1) ? currentEntity : prefab->CreateEntity(nodeData.Name + "_MeshPart");
                if (nodeData.Meshes.size() > 1) {
                    meshEntity.SetParent(currentEntity);
                    meshEntity.AddComponent<TransformComponent>();
                }

                meshEntity.AddComponent<MeshComponent>(CreateMeshAsset(meshData, fullData));

                if (meshData.MaterialIndex < materialHandles.size()) {
                    meshEntity.AddComponent<MaterialComponent>(materialHandles[meshData.MaterialIndex]);
                }
            }

            for (const auto& childNode : nodeData.Children) {
                BuildEntityHierarchy(childNode, prefab, currentEntity, fullData, materialHandles);
            }
        }

        static void AttachLight(Entity entity, const ParsedLight& data)
        {
            auto& light = entity.AddComponent<LightComponent>();
            light.Type = data.Type;
            light.Color = data.Color;
            light.Intensity = data.Intensity;
            light.CastShadows = data.CastShadows;
            light.InnerCutOff = data.InnerCutOff;
            light.OuterCutOff = data.OuterCutOff;
        }

        static void AttachCamera(Entity entity, const ParsedCamera& data)
        {
            Shared<SceneCamera> camera = MakeShared<SceneCamera>();
            if (data.Orthographic)
                camera->SetOrthographic(data.OrthographicSize, data.NearClip, data.FarClip);
            else
                camera->SetPerspective(data.VerticalFOV, data.NearClip, data.FarClip);

            auto& cameraComponent = entity.AddComponent<SceneCameraComponent>(camera);
            cameraComponent.primary = data.Primary;
        }
    };

};
