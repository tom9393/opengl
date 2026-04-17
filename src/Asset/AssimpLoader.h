#pragma once

#include "../pch.h"
#include "../Core/Application.h"
#include "Model.h"
#include <assimp/Importer.hpp>
#include <assimp/material.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "AssetManager.h"
#include "Mesh.h"
#include "../Render/BufferLayout.h"
#include "Texture.h"
#include "../Scene/Scene.h"
#include "../Scene/Entity.h"
#include "../Render/Renderer.h"
#include <glm/gtc/quaternion.hpp>

#include "YamlArchive.h"
#include <yaml-cpp/yaml.h>
#include <fstream>

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

    static TextureType AssimpToTextureType(aiTextureType type)
    {
        // Albedo / Diffuse
        if (type == aiTextureType_DIFFUSE || type == aiTextureType_BASE_COLOR) 
            return TextureType::Diffuse;
        
        // Specular
        if (type == aiTextureType_SPECULAR) 
            return TextureType::Specular;

        // Normals / Height (OBJ files often use Height for Normals)
        if (type == aiTextureType_NORMALS || type == aiTextureType_HEIGHT || type == aiTextureType_NORMAL_CAMERA) 
            return TextureType::Normal;

        // Metallic / Shininess
        if (type == aiTextureType_METALNESS || type == aiTextureType_SHININESS) 
            return TextureType::Metallic;

        // Roughness
        if (type == aiTextureType_DIFFUSE_ROUGHNESS || type == aiTextureType_MAYA_SPECULAR_ROUGHNESS) 
            return TextureType::Roughness;

        // Ambient Occlusion / Lightmaps
        if (type == aiTextureType_AMBIENT_OCCLUSION || type == aiTextureType_LIGHTMAP || type == aiTextureType_AMBIENT) 
            return TextureType::AO;

        // Emissive
        if (type == aiTextureType_EMISSIVE || type == aiTextureType_EMISSION_COLOR) 
            return TextureType::Emissive;

        // Transparency / Opacity
        if (type == aiTextureType_OPACITY) 
            return TextureType::Transparency;

        return TextureType::None;
    }

    static aiTextureType TypeToAssimpType(TextureType type)
    {
        switch (type)
        {
            case TextureType::Diffuse:      return aiTextureType_BASE_COLOR;
            case TextureType::Specular:     return aiTextureType_SPECULAR;
            case TextureType::Normal:       return aiTextureType_NORMALS;
            case TextureType::Metallic:     return aiTextureType_METALNESS;
            case TextureType::Roughness:    return aiTextureType_DIFFUSE_ROUGHNESS;
            case TextureType::AO:           return aiTextureType_AMBIENT_OCCLUSION;
            case TextureType::Emissive:     return aiTextureType_EMISSION_COLOR;
            case TextureType::Transparency: return aiTextureType_OPACITY;
            default:                        return aiTextureType_NONE;
        }
    }

    class AssimpLoader
    {
        public:
        static Shared<Scene> LoadModel(const AssetMetadata &metadata)
        {
            Assimp::Importer importer;
            std::filesystem::path path = metadata.FilePath;
            const aiScene* scene = importer.ReadFile(path, s_AssimpFlags);

            if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
                std::cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
                return nullptr;
            }

            std::string modelName = metadata.Name;
            Shared<Scene> targetScene = MakeShared<Scene>();
            Entity rootEntity = targetScene->CreateEntity(modelName);
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
            ProcessAiNodeToEntity(scene->mRootNode, scene, targetScene, rootEntity, meshHandles, materialHandles);

            Shared<Asset> castedScene = targetScene;
            AssetManager::RegisterAsset(metadata, castedScene);

            std::cout << "Successfully loaded and registered entire model: " << path.string() << std::endl;
            return targetScene;
        }

    private:
        static void ProcessAiNodeToEntity(aiNode* node, const aiScene* scene, Shared<Scene>& targetScene, Entity parentEntity, const std::vector<UUID>& meshHandles, const std::vector<UUID>& materialHandles)
        {
            std::string nodeName = node->mName.C_Str();
            if (nodeName.empty()) nodeName = "Unnamed_Node";

            Entity currentEntity = targetScene->CreateEntity(nodeName);
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
                Entity meshEntity = (node->mNumMeshes == 1) ? currentEntity : targetScene->CreateEntity(aiMesh->mName.C_Str());
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
                ProcessAiNodeToEntity(node->mChildren[i], scene, targetScene, currentEntity, meshHandles, materialHandles);
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
            Shared<Material> material = MakeShared<Material>();

            std::vector<aiTextureType> textureTypes = { 
                aiTextureType_DIFFUSE, aiTextureType_BASE_COLOR,
                aiTextureType_NORMALS, aiTextureType_HEIGHT, aiTextureType_NORMAL_CAMERA,
                aiTextureType_METALNESS, aiTextureType_SHININESS,
                aiTextureType_DIFFUSE_ROUGHNESS, aiTextureType_MAYA_SPECULAR_ROUGHNESS,
                aiTextureType_AMBIENT_OCCLUSION, aiTextureType_LIGHTMAP,
                aiTextureType_EMISSIVE, aiTextureType_EMISSION_COLOR,
                aiTextureType_OPACITY, aiTextureType_REFLECTION,
                aiTextureType_SHININESS
            };

            // --- 1 & 2. Kd & d: Diffuse Color and Opacity ---
            glm::vec4 albedoTint = glm::vec4(1.0f);
            aiColor4D color;
            if (AI_SUCCESS == aiGetMaterialColor(mat, AI_MATKEY_COLOR_DIFFUSE, &color)) {
                albedoTint.r = color.r; albedoTint.g = color.g; albedoTint.b = color.b;
            }
            float opacity = 1.0f;
            if (AI_SUCCESS == aiGetMaterialFloat(mat, AI_MATKEY_OPACITY, &opacity)) {
                albedoTint.a = opacity;
            }
            material->SetVec4("pbr.albedoTint", albedoTint);

            // --- 3. Ke: Emissive ---
            float emission = 0.0f;
            if (AI_SUCCESS == aiGetMaterialColor(mat, AI_MATKEY_COLOR_EMISSIVE, &color)) {
                // Average the RGB values to get a scalar emission intensity
                emission = (color.r + color.g + color.b) / 3.0f;
            }
            material->SetFloat("pbr.emission", emission);

            // --- 4. Ns: Shininess -> Roughness Conversion ---
            float roughness = 0.5f; // Safe default
            float shininess = 0.0f;
            if (AI_SUCCESS == aiGetMaterialFloat(mat, AI_MATKEY_SHININESS, &shininess)) {
                // Prevent division by zero. 
                // Math: High shininess (e.g., 714) = very smooth = low roughness.
                roughness = glm::sqrt(2.0f / (shininess + 2.0f));
            }
            material->SetFloat("pbr.roughnessMultiplier", roughness);

            // --- 5. Ks: Specular -> Metallic Guessing (Optional hack) ---
            float metallic = 0.0f;
            if (AI_SUCCESS == aiGetMaterialColor(mat, AI_MATKEY_COLOR_SPECULAR, &color)) {
                // In legacy formats, highly reflective grey/colored specular usually implies metal.
                float specAverage = (color.r + color.g + color.b) / 3.0f;
                if (specAverage > 0.8f) {
                    metallic = 1.0f; 
                }
            }
            material->SetFloat("pbr.metallicMultiplier", metallic);


            material->SetBool("pbr.useAlbedoMap", false);
            material->SetBool("pbr.useNormalMap", false);
            material->SetBool("pbr.useMetallicMap", false);
            material->SetBool("pbr.useRoughnessMap", false);
            material->SetBool("pbr.useAOMap", false);

            for (auto& type : textureTypes)
            {
                for(unsigned int i = 0; i < mat->GetTextureCount(type); i++)
                {
                    aiString str;
                    mat->GetTexture(type, i, &str);

                    std::string filename = std::string(str.C_Str());
                    std::string fullPath = directory + "/" + filename;
                    
                    size_t lastindex = fullPath.find_last_of("."); 
                    std::string ktxExtensionPath = fullPath.substr(0, lastindex) + ".ktx";
                    std::string ktxExtensionPath2 = fullPath.substr(0, lastindex) + ".ktx2";

                    std::string finalLoadPath = fullPath;
                    // if (std::filesystem::exists(ktxExtensionPath))       finalLoadPath = ktxExtensionPath;
                    // else if (std::filesystem::exists(ktxExtensionPath2)) finalLoadPath = ktxExtensionPath2;

                    Shared<Texture2D> texture;

                    if (const aiTexture* embeddedTexture = scene->GetEmbeddedTexture(str.C_Str())) 
                    {
                        // --- EMBEDDED TEXTURE HANDLING ---

                        if (embeddedTexture->mHeight == 0) 
                        {
                            // Compressed embedded texture
                            void* buffer = embeddedTexture->pcData;
                            size_t bufferSize = embeddedTexture->mWidth;
                            texture = Texture2D::Create(buffer, bufferSize); 
                        } 
                        else 
                        {
                            // Raw embedded texture
                            void* buffer = embeddedTexture->pcData;
                            uint32_t width = embeddedTexture->mWidth;
                            uint32_t height = embeddedTexture->mHeight;
                            texture = Texture2D::Create(buffer, width, height);
                        }

                        // Use the callback instead of hardcoded AssetManager
                        AssetMetadata metadata;
                        metadata.Name = std::string(str.C_Str());
                        metadata.Type = AssetType::Texture2D;
                        metadata.IsMemoryAsset = true;

                        Shared<Asset> texCast = texture;
                        UUID texHandle = AssetManager::RegisterAsset(metadata, texCast);
                        
                    } else {
                        const aiTexture* text = scene->GetEmbeddedTexture(str.C_Str());

                        // Load the texture asset. 
                        UUID texHandle = AssetManager::ImportAsset(finalLoadPath);
                        texture = AssetManager::GetAsset<Texture2D>(texHandle);
                    }

                    if (!texture)
                    {
                        std::cout << "Warning: Failed to load texture: " << filename << std::endl;
                        continue;
                    }

                    TextureType texType = AssimpToTextureType(type);
                    
                    // Route the texture to the appropriate string key expected by the shader
                    if (texType == TextureType::Diffuse) {
                        material->SetTexture("pbr.albedoMap", texture);
                        material->SetBool("pbr.useAlbedoMap", true);
                    }
                    if (texType == TextureType::Normal) {
                        material->SetTexture("pbr.normalMap", texture);
                        material->SetBool("pbr.useNormalMap", true);
                    }
                    if (texType == TextureType::Metallic) {
                        material->SetTexture("pbr.metallicMap", texture);
                        material->SetBool("pbr.useMetallicMap", true);
                    }
                    if (texType == TextureType::Roughness) {
                        material->SetTexture("pbr.roughnessMap", texture);
                        material->SetBool("pbr.useRoughnessMap", true);
                    }
                    if (texType == TextureType::AO) {
                        material->SetTexture("pbr.aoMap", texture);
                        material->SetBool("pbr.useAOMap", true);
                    }
                }
            }

            material->TargetShader = ShaderLibrary::Get("GBufferShader");

            return material;
        }
    };

};
