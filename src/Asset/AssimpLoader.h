#pragma once

#include <assimp/Importer.hpp>
#include <assimp/material.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "pch.h"
#include "Core/Application.h"
#include "AssetManager.h"
#include "Asset/Prefab.h"
#include "Asset/AssetUploadQueue.h"
#include "Render/BufferLayout.h"
#include "MaterialSpecification.h"
#include "MatdefLoader.h"
#include "Mesh.h"
#include "Render/Renderer.h"
#include "Scene/Entity.h"
#include "Texture.h"
#include "TextureImporter.h"

namespace Engine
{

    struct GeometrySpecification
    {
        bool CalculateNormals;
        bool CalculateTangents;
        bool CalculateBitangents;
        bool ImportTextures;
        bool ImportAnimations;
    };

    //static constexpr unsigned int s_AssimpFlags = aiProcess_Triangulate | aiProcess_SortByPType | aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace | aiProcess_JoinIdenticalVertices | aiProcess_OptimizeGraph | aiProcess_OptimizeMeshes | aiProcess_ImproveCacheLocality;
    static constexpr unsigned int s_AssimpFlags = aiProcess_Triangulate | aiProcess_SortByPType | aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace | aiProcess_JoinIdenticalVertices | aiProcess_ImproveCacheLocality;

    static std::array<aiTextureType, 16> AssimpTextureTypes = { 
        aiTextureType_DIFFUSE, aiTextureType_BASE_COLOR,
        aiTextureType_NORMALS, aiTextureType_HEIGHT, aiTextureType_NORMAL_CAMERA,
        aiTextureType_METALNESS, aiTextureType_SHININESS,
        aiTextureType_DIFFUSE_ROUGHNESS, aiTextureType_MAYA_SPECULAR_ROUGHNESS,
        aiTextureType_AMBIENT_OCCLUSION, aiTextureType_LIGHTMAP,
        aiTextureType_EMISSIVE, aiTextureType_EMISSION_COLOR,
        aiTextureType_OPACITY, aiTextureType_REFLECTION,
        aiTextureType_SHININESS
    };

    static MaterialType AssimpToTextureType(aiTextureType type)
    {
        // Albedo / Diffuse
        if (type == aiTextureType_DIFFUSE || type == aiTextureType_BASE_COLOR) 
            return MaterialType::Albedo;
        
        // Specular
        if (type == aiTextureType_SPECULAR) 
            return MaterialType::Specular;

        // Normals / Height (OBJ files often use Height for Normals)
        if (type == aiTextureType_NORMALS || type == aiTextureType_HEIGHT || type == aiTextureType_NORMAL_CAMERA) 
            return MaterialType::Normal;

        // Metallic / Shininess
        if (type == aiTextureType_METALNESS || type == aiTextureType_SHININESS) 
            return MaterialType::Metallic;

        // Roughness
        if (type == aiTextureType_DIFFUSE_ROUGHNESS || type == aiTextureType_MAYA_SPECULAR_ROUGHNESS) 
            return MaterialType::Roughness;

        // Ambient Occlusion / Lightmaps
        if (type == aiTextureType_AMBIENT_OCCLUSION || type == aiTextureType_LIGHTMAP || type == aiTextureType_AMBIENT) 
            return MaterialType::AO;

        // Emissive
        if (type == aiTextureType_EMISSIVE || type == aiTextureType_EMISSION_COLOR) 
            return MaterialType::Emissive;

        // Transparency / Opacity
        if (type == aiTextureType_OPACITY) 
            return MaterialType::Transparency;

        return MaterialType::None;
    }

    static aiTextureType TypeToAssimpType(MaterialType type)
    {
        switch (type)
        {
            case MaterialType::Diffuse:      return aiTextureType_BASE_COLOR;
            case MaterialType::Specular:     return aiTextureType_SPECULAR;
            case MaterialType::Normal:       return aiTextureType_NORMALS;
            case MaterialType::Metallic:     return aiTextureType_METALNESS;
            case MaterialType::Roughness:    return aiTextureType_DIFFUSE_ROUGHNESS;
            case MaterialType::AO:           return aiTextureType_AMBIENT_OCCLUSION;
            case MaterialType::Emissive:     return aiTextureType_EMISSION_COLOR;
            case MaterialType::Transparency: return aiTextureType_OPACITY;
            default:                         return aiTextureType_NONE;
        }
    }

    struct ParsedMesh {
        std::string Name;
        std::vector<float> Vertices;
        std::vector<unsigned int> Indices;
        uint32_t MaterialIndex = 0;
        glm::vec3 MinExtents;
        glm::vec3 MaxExtents;
    };

    struct ParsedMaterial {
        std::string Name;
        glm::vec4 AlbedoTint;
        float Emission;
        float Roughness;
        float Metallic;
        std::unordered_map<MaterialType, std::string> TexturePaths;
        std::unordered_map<MaterialType, TextureLoadData> EmbeddedTextures;
    };

    struct ParsedNode {
        std::string Name;
        glm::vec3 Translation;
        glm::vec3 Rotation;
        glm::vec3 Scale;
        std::vector<uint32_t> MeshIndices;
        std::vector<ParsedNode> Children;
    };

    struct ParsedModelData {
        std::string Name;
        std::string Directory;
        std::vector<ParsedMesh> Meshes;
        std::vector<ParsedMaterial> Materials;
        ParsedNode RootNode;
    };

    class AssimpLoader
    {
        public:
        static Shared<Prefab> LoadModel(const AssetMetadata &metadata)
        {
            Assimp::Importer importer;
            std::filesystem::path path = metadata.FilePath;
            const aiScene* scene = importer.ReadFile(path, s_AssimpFlags);

            if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
                std::cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
                return nullptr;
            }

            std::string modelName = metadata.Name;
            Shared<Prefab> prefab = MakeShared<Prefab>();
            Entity rootEntity = prefab->CreateEntity(modelName);
            rootEntity.AddComponent<TransformComponent>();

            // 1. Process and Register all Materials
            std::vector<UUID> materialHandles(scene->mNumMaterials);
            for (uint32_t i = 0; i < scene->mNumMaterials; i++) 
            {
                // Pass the callback down so embedded textures can also be registered
                Shared<Material> material = ProcessAiMaterial(scene->mMaterials[i], path.parent_path().string(), scene);
                
                aiString aiMatName;
                scene->mMaterials[i]->Get(AI_MATKEY_NAME, aiMatName);
                std::string matName = aiMatName.length > 0 ? aiMatName.C_Str() : "Material_" + std::to_string(i);

                AssetMetadata metadata;
                metadata.Handle = UUID();
                metadata.Name = modelName + "_" + matName;
                metadata.FilePath = path.string();
                metadata.Type = AssetType::Material;
                metadata.IsMemoryAsset = true;

                Shared<Asset> assetCast = material;
                materialHandles[i] = AssetManager::RegisterAsset(metadata, assetCast);
            }

            // 2. Process and Register all Meshes
            std::vector<UUID> meshHandles(scene->mNumMeshes);
            for (uint32_t i = 0; i < scene->mNumMeshes; i++) 
            {
                Shared<Mesh> mesh = ProcessAiMesh(scene->mMeshes[i]);
                
                std::string meshName = scene->mMeshes[i]->mName.C_Str();
                if (meshName.empty()) meshName = "Mesh_" + std::to_string(i);

                AssetMetadata metadata;
                metadata.Handle = UUID();
                metadata.Name = modelName + "_" + meshName;
                metadata.FilePath = path.string();
                metadata.Type = AssetType::Mesh;
                metadata.IsMemoryAsset = true;

                Shared<Asset> assetCast = mesh;
                meshHandles[i] = AssetManager::RegisterAsset(metadata, assetCast);
            }

            // 3. Process the Hierarchy directly into the Scene
            ProcessAiNodeToEntity(scene->mRootNode, scene, prefab, rootEntity, meshHandles, materialHandles);

            Shared<Asset> castedScene = prefab;
            AssetManager::RegisterAsset(metadata, castedScene);

            std::cout << "Successfully loaded and registered entire model: " << path.string() << std::endl;
            return prefab;
        }

        static void LoadModelAsync(const AssetMetadata& metadata, std::function<void(Shared<Asset>)> cb) {
            Application::Get().GetThreadPool().Enqueue([metadata, cb]() {
                
                // --- PHASE 1: WORKER THREAD ---
                Assimp::Importer importer;
                const aiScene* scene = importer.ReadFile(metadata.FilePath, s_AssimpFlags);

                if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
                    std::cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
                    return; // Note: Handle failure state properly in production
                }

                ParsedModelData modelData;
                modelData.Name = metadata.Name;
                modelData.Directory = std::filesystem::path(metadata.FilePath).parent_path().string();

                // Extract Materials
                for (uint32_t i = 0; i < scene->mNumMaterials; i++) {
                    modelData.Materials.push_back(ParseMaterialData(scene->mMaterials[i], scene));
                }

                // Extract Meshes
                for (uint32_t i = 0; i < scene->mNumMeshes; i++) {
                    modelData.Meshes.push_back(ParseMeshData(scene->mMeshes[i]));
                }

                // Extract Hierarchy
                modelData.RootNode = ParseNodeData(scene->mRootNode);

                // --- SEND TO MAIN THREAD ---
                AssetUploadQueue::Submit([metadata, modelData, cb]() {
                    Shared<Prefab> model = FinalizeModelOnMainThread(metadata, modelData);
                    if (cb) cb(model);
                });
            });
        }

    private:
        static ParsedNode ParseNodeData(aiNode* node) {
            ParsedNode parsedNode;
            parsedNode.Name = node->mName.C_Str();
            if (parsedNode.Name.empty()) parsedNode.Name = "Unnamed_Node";

            aiVector3D aiScale, aiPos;
            aiQuaternion aiRot;
            node->mTransformation.Decompose(aiScale, aiRot, aiPos);

            parsedNode.Translation = { aiPos.x, aiPos.y, aiPos.z };
            parsedNode.Rotation = glm::eulerAngles(glm::quat(aiRot.w, aiRot.x, aiRot.y, aiRot.z));
            parsedNode.Scale = { aiScale.x, aiScale.y, aiScale.z };

            for (uint32_t i = 0; i < node->mNumMeshes; i++) {
                parsedNode.MeshIndices.push_back(node->mMeshes[i]);
            }

            for (uint32_t i = 0; i < node->mNumChildren; i++) {
                parsedNode.Children.push_back(ParseNodeData(node->mChildren[i]));
            }
            return parsedNode;
        }

        static ParsedMesh ParseMeshData(aiMesh* aiMesh) {
            ParsedMesh parsedMesh;
            parsedMesh.Name = aiMesh->mName.C_Str();
            if (parsedMesh.Name.empty()) parsedMesh.Name = "Unnamed_Mesh";
            parsedMesh.MaterialIndex = aiMesh->mMaterialIndex;

            glm::vec3 minExtents(std::numeric_limits<float>::max());
            glm::vec3 maxExtents(std::numeric_limits<float>::lowest());

            bool hasPositions = aiMesh->HasPositions();
            bool hasNormals = aiMesh->HasNormals();
            bool hasUVs = aiMesh->mTextureCoords[0] != nullptr;
            bool hasTangents = aiMesh->HasTangentsAndBitangents();

            // Calculate exact floats required
            size_t floatsPerVertex = 0;
            if (hasPositions) floatsPerVertex += 3;
            if (hasNormals) floatsPerVertex += 3;
            if (hasUVs) floatsPerVertex += 2;
            if (hasTangents) floatsPerVertex += 6;

            parsedMesh.Vertices.reserve(aiMesh->mNumVertices * floatsPerVertex);

            for (unsigned int i = 0; i < aiMesh->mNumVertices; i++) {
                if (hasPositions) {
                    glm::vec3 pos(aiMesh->mVertices[i].x, aiMesh->mVertices[i].y, aiMesh->mVertices[i].z);
                    minExtents = glm::min(minExtents, pos);
                    maxExtents = glm::max(maxExtents, pos);
                    parsedMesh.Vertices.push_back(pos.x); parsedMesh.Vertices.push_back(pos.y); parsedMesh.Vertices.push_back(pos.z);
                }
                if (hasNormals) {
                    parsedMesh.Vertices.push_back(aiMesh->mNormals[i].x); parsedMesh.Vertices.push_back(aiMesh->mNormals[i].y); parsedMesh.Vertices.push_back(aiMesh->mNormals[i].z);
                }
                if (hasUVs) {
                    parsedMesh.Vertices.push_back(aiMesh->mTextureCoords[0][i].x); parsedMesh.Vertices.push_back(aiMesh->mTextureCoords[0][i].y);
                }
                if (hasTangents) {
                    parsedMesh.Vertices.push_back(aiMesh->mTangents[i].x); parsedMesh.Vertices.push_back(aiMesh->mTangents[i].y); parsedMesh.Vertices.push_back(aiMesh->mTangents[i].z);
                    parsedMesh.Vertices.push_back(aiMesh->mBitangents[i].x); parsedMesh.Vertices.push_back(aiMesh->mBitangents[i].y); parsedMesh.Vertices.push_back(aiMesh->mBitangents[i].z);
                }
            }

            parsedMesh.Indices.reserve(aiMesh->mNumFaces * 3);
            for (unsigned int i = 0; i < aiMesh->mNumFaces; i++) {
                aiFace face = aiMesh->mFaces[i];
                for (unsigned int j = 0; j < face.mNumIndices; j++) {
                    parsedMesh.Indices.push_back(face.mIndices[j]);
                }
            }

            parsedMesh.MinExtents = minExtents;
            parsedMesh.MaxExtents = maxExtents;

            return parsedMesh;
        }

        static ParsedMaterial ParseMaterialData(aiMaterial* mat, const aiScene* scene) {
            ParsedMaterial parsedMat;
            aiString aiMatName;
            mat->Get(AI_MATKEY_NAME, aiMatName);
            parsedMat.Name = aiMatName.length > 0 ? aiMatName.C_Str() : "Unnamed_Material";

            // Grab properties (Using your existing helpers)
            parsedMat.AlbedoTint = GetAlbedoTint(mat);
            parsedMat.Emission = GetEmission(mat);
            parsedMat.Roughness = GetRoughness(mat);
            parsedMat.Metallic = GetMetallic(mat);

            // Grab Texture Paths
            // Note: I stripped out `AssimpTextureTypes` array from the previous file to save space, 
            // you'll need to leave your `AssimpTextureTypes` and `AssimpToTextureType` definitions above this class!
            std::array<aiTextureType, 5> typesToCheck = { aiTextureType_DIFFUSE, aiTextureType_NORMALS, aiTextureType_METALNESS, aiTextureType_DIFFUSE_ROUGHNESS, aiTextureType_AMBIENT_OCCLUSION };
            
            for (auto type : typesToCheck) {
                if (mat->GetTextureCount(type) > 0) {
                    aiString str;
                    mat->GetTexture(type, 0, &str);
                    MaterialType matType = AssimpToTextureType(type); // Call your helper
                    if (matType != MaterialType::None) {
                        parsedMat.TexturePaths[matType] = std::string(str.C_Str());
                        // --- NEW EMBEDDED TEXTURE CHECK ---
                        if (const aiTexture* embeddedTex = scene->GetEmbeddedTexture(str.C_Str())) {
                            if (embeddedTex->mHeight == 0) {
                                parsedMat.EmbeddedTextures[matType] = TextureImporter::DecodeTextureFromMemory(embeddedTex->pcData, embeddedTex->mWidth);
                            } else {
                                parsedMat.EmbeddedTextures[matType] = TextureImporter::DecodeTextureUncompressed(embeddedTex->pcData, embeddedTex->mWidth, embeddedTex->mHeight);
                            }
                        } else {
                            // It's external, just save the path as usual.
                            parsedMat.TexturePaths[matType] = std::string(str.C_Str());
                        }
                    }
                }
            }
            return parsedMat;
        }

        static Shared<Prefab> FinalizeModelOnMainThread(const AssetMetadata& metadata, const ParsedModelData& data) {
            PROFILE_FUNCTION();
            Shared<Prefab> prefab = MakeShared<Prefab>();
            Entity rootEntity = prefab->CreateEntity(data.Name);
            rootEntity.AddComponent<TransformComponent>();

            // 1. Generate GPU Materials
            std::vector<UUID> materialHandles(data.Materials.size());
            for (size_t i = 0; i < data.Materials.size(); i++) {
                const auto& matData = data.Materials[i];
                Shared<Material> material = MatdefLoader::LoadMaterial("/home/tom/Coding/c++/opengl/shaders/PBR_Standard.matdef");

                material->Add({MaterialType::AlbedoTint, matData.AlbedoTint});
                material->Add({MaterialType::Emission, matData.Emission});
                material->Add({MaterialType::RoughnessMultiplier, matData.Roughness});
                material->Add({MaterialType::MetallicMultiplier, matData.Metallic});


                // Load Textures (Note: In a fully async system, you would trigger LoadTextureAsync here)
                for (const auto& [type, path] : matData.TexturePaths) {
                    std::string texturePath = data.Directory + "/" + path;
                    if (!std::filesystem::exists(texturePath)) continue;
                    UUID texHandle = AssetManager::ImportAsset(texturePath);
                    AssetMetadata metadata = AssetManager::GetMetadata(texHandle);

                    TextureImporter::LoadTexture2DAsync(metadata, [material, type, texHandle](Shared<Asset> asset) {
                        Shared<Texture2D> img = std::static_pointer_cast<Texture2D>(asset);
                        material->Add({type, img}); 
                    });
                }

                for (auto& [type, loadData] : matData.EmbeddedTextures) {
                    // RIGHT HERE! We use your existing method now that we are safe on the Main Thread

                    Shared<Texture2D> texture = Texture2D::Create(loadData);
                    
                    // Clean up the CPU RAM!
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
                matMeta.Handle = UUID();
                matMeta.Name = data.Name + "_" + matData.Name;
                matMeta.Type = AssetType::Material;
                matMeta.IsMemoryAsset = true;
                Shared<Asset> assetCast = material;
                materialHandles[i] = AssetManager::RegisterAsset(matMeta, assetCast);
            }

            // 2. Generate GPU Meshes
            std::vector<UUID> meshHandles(data.Meshes.size());
            for (size_t i = 0; i < data.Meshes.size(); i++) {
                const auto& meshData = data.Meshes[i];
                Shared<Mesh> mesh = MakeShared<Mesh>();
                mesh->GetVertices() = meshData.Vertices; 
                mesh->GetIndices() = meshData.Indices;

                // --- OPENGL CALLS HAPPEN HERE ---
                Shared<VertexArray> VAO = VertexArray::Create();
                Shared<VertexBuffer> VBO = VertexBuffer::Create((float*)meshData.Vertices.data(), meshData.Vertices.size() * sizeof(float));
                
                // Reconstruct Layout dynamically (Simplified layout assumption based on your original code)
                VBO->SetLayout({
                    { ShaderDataType::Float3, "a_Position" },
                    { ShaderDataType::Float3, "a_Normal" },
                    { ShaderDataType::Float2, "a_TexCoord" },
                    { ShaderDataType::Float3, "a_Tangent" },
                    { ShaderDataType::Float3, "a_Bitangent" }
                });
                
                VAO->AddVertexBuffer(VBO);
                Shared<IndexBuffer> EBO = IndexBuffer::Create((unsigned int*)meshData.Indices.data(), meshData.Indices.size());
                VAO->SetIndexBuffer(EBO);

                mesh->SetVAO(VAO);
                mesh->SetVBOs({VBO});
                mesh->SetEBO(EBO);
                mesh->SetAABB({ meshData.MinExtents, meshData.MaxExtents });

                AssetMetadata meshMeta;
                meshMeta.Handle = UUID();
                meshMeta.Name = data.Name + "_" + meshData.Name;
                meshMeta.Type = AssetType::Mesh;
                meshMeta.IsMemoryAsset = true;
                Shared<Asset> assetCast = mesh;
                meshHandles[i] = AssetManager::RegisterAsset(meshMeta, assetCast);
            }

            // 3. Build ECS Hierarchy
            BuildEntityHierarchy(data.RootNode, prefab, rootEntity, meshHandles, materialHandles, data);

            // 4. Mark Complete!
            Shared<Asset> prefabCast = prefab;
            AssetManager::RegisterAsset(metadata, prefabCast);
            std::cout << "Successfully loaded async model: " << metadata.FilePath << std::endl;
            return prefab;
        }

        static void BuildEntityHierarchy(const ParsedNode& nodeData, Shared<Prefab>& prefab, Entity parentEntity, const std::vector<UUID>& meshHandles, const std::vector<UUID>& materialHandles, const ParsedModelData& fullData) {
            Entity currentEntity = prefab->CreateEntity(nodeData.Name);
            currentEntity.SetParent(parentEntity);

            auto& transform = currentEntity.AddComponent<TransformComponent>();
            transform.translation = nodeData.Translation;
            transform.rotation = nodeData.Rotation;
            transform.scale = nodeData.Scale;

            for (uint32_t meshIndex : nodeData.MeshIndices) {
                Entity meshEntity = (nodeData.MeshIndices.size() == 1) ? currentEntity : prefab->CreateEntity(nodeData.Name + "_MeshPart");
                if (nodeData.MeshIndices.size() > 1) {
                    meshEntity.SetParent(currentEntity);
                    meshEntity.AddComponent<TransformComponent>();
                }

                meshEntity.AddComponent<MeshComponent>(meshHandles[meshIndex]);

                // Map the material index from the ParsedMesh
                uint32_t matIndex = fullData.Meshes[meshIndex].MaterialIndex;
                if (matIndex < materialHandles.size()) {
                    meshEntity.AddComponent<MaterialComponent>(materialHandles[matIndex]);
                }
            }

            for (const auto& childNode : nodeData.Children) {
                BuildEntityHierarchy(childNode, prefab, currentEntity, meshHandles, materialHandles, fullData);
            }
        }

        static void ProcessAiNodeToEntity(aiNode* node, const aiScene* scene, Shared<Prefab>& prefab, Entity parentEntity, const std::vector<UUID>& meshHandles, const std::vector<UUID>& materialHandles)
        {
            std::string nodeName = node->mName.C_Str();
            if (nodeName.empty()) nodeName = "Unnamed_Node";

            Entity currentEntity = prefab->CreateEntity(nodeName);
            currentEntity.SetParent(parentEntity);

            // Extract and set Transform
            aiVector3D aiScale, aiPos;
            aiQuaternion aiRot;
            node->mTransformation.Decompose(aiScale, aiRot, aiPos);

            auto& transform = currentEntity.AddComponent<TransformComponent>();
            transform.translation = { aiPos.x, aiPos.y, aiPos.z };
            transform.rotation = glm::eulerAngles(glm::quat(aiRot.w, aiRot.x, aiRot.y, aiRot.z));
            transform.scale = { aiScale.x, aiScale.y, aiScale.z };

            // Attach Meshes (Assimp nodes can have multiple meshes)
            for (uint32_t i = 0; i < node->mNumMeshes; i++) 
            {
                uint32_t meshIndex = node->mMeshes[i];
                aiMesh* aiMesh = scene->mMeshes[meshIndex];
                
                // If a node has multiple meshes, we create sub-entities for them to keep the 1-Mesh-per-Entity standard
                Entity meshEntity = (node->mNumMeshes == 1) ? currentEntity : prefab->CreateEntity(aiMesh->mName.C_Str());
                if (node->mNumMeshes > 1) {
                    meshEntity.SetParent(currentEntity);
                    meshEntity.AddComponent<TransformComponent>();
                }

                auto& meshComp = meshEntity.AddComponent<MeshComponent>();
                meshComp.MeshHandle = meshHandles[meshIndex];

                uint32_t materialIndex = aiMesh->mMaterialIndex;
                if (materialIndex < materialHandles.size()) {
                    auto& matComp = meshEntity.AddComponent<MaterialComponent>();
                    matComp.MaterialHandle = materialHandles[materialIndex];
                }
            }

            // Recursively process children
            for (uint32_t i = 0; i < node->mNumChildren; i++) 
            {
                ProcessAiNodeToEntity(node->mChildren[i], scene, prefab, currentEntity, meshHandles, materialHandles);
            }
        }

        static Shared<Mesh> ProcessAiMesh(aiMesh* aiMesh)
        {
            glm::vec3 minExtents(std::numeric_limits<float>::max());
            glm::vec3 maxExtents(std::numeric_limits<float>::lowest());

            Shared<Mesh> mesh = MakeShared<Mesh>();
            std::vector<float>& vertices = mesh->GetVertices();

            bool hasPositions = aiMesh->HasPositions();
            bool hasNormals = aiMesh->HasNormals();
            bool hasUVs = aiMesh->mTextureCoords[0] != nullptr;
            bool hasTangents = aiMesh->HasTangentsAndBitangents();

            std::vector<BufferElement> layoutElements;
            size_t floatsPerVertex = 0;

            if (hasPositions) {
                layoutElements.push_back({ ShaderDataType::Float3, "a_Position" });
                floatsPerVertex += 3;
            }
            if (hasNormals) {
                layoutElements.push_back({ ShaderDataType::Float3, "a_Normal" });
                floatsPerVertex += 3;
            }
            if (hasUVs) {
                layoutElements.push_back({ ShaderDataType::Float2, "a_TexCoord" });
                floatsPerVertex += 2;
            }
            if (hasTangents) {
                layoutElements.push_back({ ShaderDataType::Float3, "a_Tangent" });
                layoutElements.push_back({ ShaderDataType::Float3, "a_Bitangent" });
                floatsPerVertex += 6;
            }

            BufferLayout meshLayout(layoutElements);

            vertices.reserve(aiMesh->mNumVertices * floatsPerVertex);

            for(unsigned int i = 0; i < aiMesh->mNumVertices; i++)
            {
                if (hasPositions) {
                    glm::vec3 pos(aiMesh->mVertices[i].x, aiMesh->mVertices[i].y, aiMesh->mVertices[i].z);
                    minExtents = glm::min(minExtents, pos);
                    maxExtents = glm::max(maxExtents, pos);

                    vertices.emplace_back(pos.x); 
                    vertices.emplace_back(pos.y); 
                    vertices.emplace_back(pos.z);
                }

                if (hasNormals) {
                    vertices.emplace_back(aiMesh->mNormals[i].x); 
                    vertices.emplace_back(aiMesh->mNormals[i].y); 
                    vertices.emplace_back(aiMesh->mNormals[i].z);
                } 

                if (hasUVs) {
                    vertices.emplace_back(aiMesh->mTextureCoords[0][i].x); 
                    vertices.emplace_back(aiMesh->mTextureCoords[0][i].y);
                } 

                if (hasTangents) {
                    vertices.emplace_back(aiMesh->mTangents[i].x); 
                    vertices.emplace_back(aiMesh->mTangents[i].y); 
                    vertices.emplace_back(aiMesh->mTangents[i].z);
                    
                    vertices.emplace_back(aiMesh->mBitangents[i].x); 
                    vertices.emplace_back(aiMesh->mBitangents[i].y); 
                    vertices.emplace_back(aiMesh->mBitangents[i].z);
                } 
            }

            std::vector<unsigned int>& indices = mesh->GetIndices();
            indices.reserve(aiMesh->mNumFaces * 3);
            for(unsigned int i = 0; i < aiMesh->mNumFaces; i++)
            {
                aiFace face = aiMesh->mFaces[i];
                for(unsigned int j = 0; j < face.mNumIndices; j++)
                    indices.emplace_back(face.mIndices[j]);        
            }

            Shared<VertexArray> VAO = VertexArray::Create();
            VAO->Bind();

            Shared<VertexBuffer> VBO = VertexBuffer::Create(&vertices[0], vertices.size() * sizeof(float));
            VBO->SetLayout(meshLayout);
            VAO->AddVertexBuffer(VBO);

            Shared<IndexBuffer> EBO = IndexBuffer::Create(&indices[0], indices.size());
            VAO->SetIndexBuffer(EBO);

            VAO->Unbind();

            std::vector<Shared<VertexBuffer>> VBOs = { VBO };

            mesh->SetVAO(VAO);
            mesh->SetVBOs(VBOs);
            mesh->SetEBO(EBO);
            mesh->SetAABB({ minExtents, maxExtents });

            return mesh;
        }

        static Shared<Material> ProcessAiMaterial(aiMaterial* mat, const std::string& directory, const aiScene* scene)
        {
            Shared<Material> material = MatdefLoader::LoadMaterial("/home/tom/Coding/c++/opengl/shaders/PBR_Standard.matdef");

            material->Add({MaterialType::AlbedoTint, GetAlbedoTint(mat)});
            material->Add({MaterialType::Emission, GetEmission(mat)});
            material->Add({MaterialType::RoughnessMultiplier, GetRoughness(mat)});
            material->Add({MaterialType::MetallicMultiplier, GetMetallic(mat)});

            for (auto& type : AssimpTextureTypes)
            {
                for(unsigned int i = 0; i < mat->GetTextureCount(type); i++)
                {
                    aiString str;
                    mat->GetTexture(type, i, &str);

                    std::string filename = std::string(str.C_Str());
                    std::string fullPath = directory + "/" + filename;
                    
                    // Todo: figure out ktx loading
                    Shared<Texture2D> texture = GetTexture(scene, str, fullPath);

                    if (!texture)
                    {
                        std::cout << "Warning: Failed to load texture: " << filename << std::endl;
                        continue;
                    }

                    material->Add({AssimpToTextureType(type), texture});
                }
            }

            return material;
        }

        static glm::vec4 GetAlbedoTint(aiMaterial* mat)
        {
            glm::vec4 albedoTint = glm::vec4(1.0f);
            aiColor4D color;
            if (AI_SUCCESS == aiGetMaterialColor(mat, AI_MATKEY_COLOR_DIFFUSE, &color)) {
                albedoTint.r = color.r; albedoTint.g = color.g; albedoTint.b = color.b;
            }

            float opacity = 1.0f;
            if (AI_SUCCESS == aiGetMaterialFloat(mat, AI_MATKEY_OPACITY, &opacity)) {
                albedoTint.a = opacity;
            }
            return albedoTint;
        }

        static float GetEmission(aiMaterial* mat)
        {
            aiColor4D color;
            float emission = 0.0f;
            if (AI_SUCCESS == aiGetMaterialColor(mat, AI_MATKEY_COLOR_EMISSIVE, &color)) {
                // Average the RGB values to get a scalar emission intensity
                emission = (color.r + color.g + color.b) / 3.0f;
            }
            return emission;
        }

        static float GetRoughness(aiMaterial* mat)
        {
            float roughness = 0.5f; // Safe default
            float shininess = 0.0f;
            if (AI_SUCCESS == aiGetMaterialFloat(mat, AI_MATKEY_SHININESS, &shininess)) {
                // Prevent division by zero. 
                // Math: High shininess (e.g., 714) = very smooth = low roughness.
                roughness = glm::sqrt(2.0f / (shininess + 2.0f));
            }
            return roughness;
        }

        static float GetMetallic(aiMaterial* mat)
        {
            aiColor4D color;
            float metallic = 0.0f;
            if (AI_SUCCESS == aiGetMaterialColor(mat, AI_MATKEY_COLOR_SPECULAR, &color)) {
                // In legacy formats, highly reflective grey/colored specular usually implies metal.
                float specAverage = (color.r + color.g + color.b) / 3.0f;
                if (specAverage > 0.8f) {
                    metallic = 1.0f; 
                }
            }
            return metallic;
        }

        static Shared<Texture2D> GetTexture(const aiScene* scene, const aiString &str, const std::string &path)
        {
            if (const aiTexture* embeddedTexture = scene->GetEmbeddedTexture(str.C_Str())) 
            {
                Shared<Texture2D> texture = CreateEmbeddedTexture(embeddedTexture);

                AssetMetadata metadata;
                metadata.Name = std::string(str.C_Str());
                metadata.FilePath = str.C_Str();
                metadata.Type = AssetType::Texture2D;
                metadata.IsMemoryAsset = true;

                Shared<Asset> texCast = texture;
                UUID texHandle = AssetManager::RegisterAsset(metadata, texCast);

                return texture;
            }

            UUID texHandle = AssetManager::ImportAsset(path);
            return AssetManager::GetAsset<Texture2D>(texHandle);

        }

        static Shared<Texture2D> CreateEmbeddedTexture(const aiTexture* embeddedTexture)
        {
            // if (embeddedTexture->mHeight == 0) 
            // {
            //     // Compressed embedded texture
            //     void* buffer = embeddedTexture->pcData;
            //     size_t bufferSize = embeddedTexture->mWidth;
            //     return Texture2D::Create(buffer, bufferSize);
            // } 

            // // Raw embedded texture
            // void* buffer = embeddedTexture->pcData;
            // uint32_t width = embeddedTexture->mWidth;
            // uint32_t height = embeddedTexture->mHeight;
            // return Texture2D::Create(buffer, width, height);

            return nullptr;

        }
    };

};
