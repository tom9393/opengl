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

namespace Engine
{

    struct ModelScanResult {
        std::vector<AssetMetadata> MeshHooks;
        std::vector<AssetMetadata> MaterialHooks;
    };

    static TextureType AssimpToTextureType(aiTextureType type)
    {
        if (type == aiTextureType_DIFFUSE || type == aiTextureType_BASE_COLOR) return TextureType::Diffuse;
        if (type == aiTextureType_SPECULAR) return TextureType::Specular;
        if (type == aiTextureType_NORMALS || type == aiTextureType_HEIGHT) return TextureType::Normal;
        if (type == aiTextureType_METALNESS) return TextureType::Metallic;
        if (type == aiTextureType_AMBIENT_OCCLUSION) return TextureType::AO;

        return TextureType::None;
    }

    static aiTextureType TypeToAssimpType(TextureType type)
    {
        if (type == TextureType::Diffuse) return aiTextureType_DIFFUSE;
        if (type == TextureType::Specular) return aiTextureType_SPECULAR;
        if (type == TextureType::Normal) return aiTextureType_NORMALS;
        if (type == TextureType::Metallic) return aiTextureType_METALNESS;
        if (type == TextureType::AO) return aiTextureType_AMBIENT_OCCLUSION;

        return aiTextureType_NONE;
    }

    static glm::mat4 AssimpToGLMMatrix(const aiMatrix4x4& matrix)
    {
        glm::mat4 result;
        // Assimp is row-major, GLM is column-major. We transpose it here.
        result[0][0] = matrix.a1; result[1][0] = matrix.a2; result[2][0] = matrix.a3; result[3][0] = matrix.a4;
        result[0][1] = matrix.b1; result[1][1] = matrix.b2; result[2][1] = matrix.b3; result[3][1] = matrix.b4;
        result[0][2] = matrix.c1; result[1][2] = matrix.c2; result[2][2] = matrix.c3; result[3][2] = matrix.c4;
        result[0][3] = matrix.d1; result[1][3] = matrix.d2; result[2][3] = matrix.d3; result[3][3] = matrix.d4;
        return result;
    }

    class AssimpLoader
    {
        public:
        static ModelScanResult ScanModel(const std::filesystem::path& path, Shared<Model>& outModel)
        {
            ModelScanResult resultHooks;
            Assimp::Importer importer;
            
            // Pass 0 as flags! We only read the structure/headers, NOT the heavy vertex data.
            const aiScene* scene = importer.ReadFile(path.string(), 0);

            if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
                std::cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
                return resultHooks;
            }

            // 1. Extract Materials
            outModel->Materials.reserve(scene->mNumMaterials);
            for (uint32_t i = 0; i < scene->mNumMaterials; i++) {
                UUID matHandle = UUID();
                outModel->Materials.push_back(matHandle);

                AssetMetadata meta;
                meta.Handle = matHandle;
                meta.Type = AssetType::Material;
                meta.FilePath = path.string();
                meta.Index = i; // Reusing this field to store Assimp's Material Index
                
                resultHooks.MaterialHooks.push_back(meta);
            }

            // 2. Extract Meshes
            outModel->Meshes.reserve(scene->mNumMeshes);
            for (uint32_t i = 0; i < scene->mNumMeshes; i++) {
                UUID meshHandle = UUID();
                outModel->Meshes.push_back(meshHandle);

                AssetMetadata meta;
                meta.Handle = meshHandle;
                meta.Type = AssetType::Mesh;
                meta.FilePath = path.string();
                meta.Index = i;
                outModel->MeshMaterialIndices.push_back(scene->mMeshes[i]->mMaterialIndex);
                
                resultHooks.MeshHooks.push_back(meta);
            }

            return resultHooks;
        }

        static Shared<Mesh> LoadSubMesh(const std::filesystem::path& path, uint32_t submeshIndex)
        {
            Assimp::Importer importer;
            
            // We use heavy processing flags here because we actually need the geometry
            const aiScene* scene = importer.ReadFile(path.string(), 
                aiProcess_Triangulate | aiProcess_GenSmoothNormals | 
                aiProcess_CalcTangentSpace | aiProcess_OptimizeMeshes);

            if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
                std::cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
                return nullptr;
            }

            if (submeshIndex >= scene->mNumMeshes) {
                std::cout << "ERROR::ASSIMP:: Submesh index out of bounds!" << std::endl;
                return nullptr;
            }

            return ProcessAiMesh(scene->mMeshes[submeshIndex]);
        }

        static Shared<Material> LoadMaterial(const std::filesystem::path& path, uint32_t materialIndex)
        {
            Assimp::Importer importer;
            
            // Pass 0 as flags! We only need to read the material string names, not geometry.
            const aiScene* scene = importer.ReadFile(path.string(), 0);

            if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
                std::cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
                return nullptr;
            }

            if (materialIndex >= scene->mNumMaterials) {
                std::cout << "ERROR::ASSIMP:: Material index out of bounds!" << std::endl;
                return nullptr;
            }

            // Extract the directory from the filepath so we know where to look for textures
            std::string directory = path.parent_path().string();

            return ProcessAiMaterial(scene->mMaterials[materialIndex], directory);
        }

    private:
        static Shared<Mesh> ProcessAiMesh(aiMesh* aiMesh)
        {
            glm::vec3 minExtents(std::numeric_limits<float>::max());
            glm::vec3 maxExtents(std::numeric_limits<float>::lowest());

            Shared<Mesh> mesh = MakeShared<Mesh>();
            std::vector<float>& vertices = mesh->GetVertices();

            BufferLayout meshLayout = {
                { ShaderDataType::Float3, "a_Position" },
                { ShaderDataType::Float3, "a_Normal" },
                { ShaderDataType::Float2, "a_TexCoord" },
                { ShaderDataType::Float3, "a_Tangent" },
                { ShaderDataType::Float3, "a_Bitangent" }
            };

            const size_t floatsPerVertex = 14; 
            vertices.reserve(aiMesh->mNumVertices * floatsPerVertex);

            bool hasNormals = aiMesh->HasNormals();
            bool hasUVs = aiMesh->mTextureCoords[0] != nullptr;
            bool hasTangents = aiMesh->HasTangentsAndBitangents();

            for(unsigned int i = 0; i < aiMesh->mNumVertices; i++)
            {
                glm::vec3 pos(aiMesh->mVertices[i].x, aiMesh->mVertices[i].y, aiMesh->mVertices[i].z);
                minExtents = glm::min(minExtents, pos);
                maxExtents = glm::max(maxExtents, pos);

                vertices.emplace_back(pos.x); vertices.emplace_back(pos.y); vertices.emplace_back(pos.z);

                if (hasNormals) {
                    vertices.emplace_back(aiMesh->mNormals[i].x); vertices.emplace_back(aiMesh->mNormals[i].y); vertices.emplace_back(aiMesh->mNormals[i].z);
                } else {
                    vertices.emplace_back(0.0f); vertices.emplace_back(1.0f); vertices.emplace_back(0.0f); 
                }

                if (hasUVs) {
                    vertices.emplace_back(aiMesh->mTextureCoords[0][i].x); vertices.emplace_back(aiMesh->mTextureCoords[0][i].y);
                } else {
                    vertices.emplace_back(0.0f); vertices.emplace_back(0.0f);
                }

                if (hasTangents) {
                    vertices.emplace_back(aiMesh->mTangents[i].x); vertices.emplace_back(aiMesh->mTangents[i].y); vertices.emplace_back(aiMesh->mTangents[i].z);
                    vertices.emplace_back(aiMesh->mBitangents[i].x); vertices.emplace_back(aiMesh->mBitangents[i].y); vertices.emplace_back(aiMesh->mBitangents[i].z);
                } else {
                    vertices.emplace_back(1.0f); vertices.emplace_back(0.0f); vertices.emplace_back(0.0f); 
                    vertices.emplace_back(0.0f); vertices.emplace_back(0.0f); vertices.emplace_back(1.0f); 
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

            mesh->SetLayout(meshLayout);
            mesh->SetAABB({ minExtents, maxExtents });

            mesh->Prepare();

            return mesh;
        }

        static Shared<Material> ProcessAiMaterial(aiMaterial* mat, const std::string& directory)
        {
            Shared<Material> material = MakeShared<Material>();

            std::vector<aiTextureType> textureTypes = { 
                aiTextureType_DIFFUSE,          // 1  Often used as Albedo in legacy files
                aiTextureType_BASE_COLOR,       // 12 Modern Albedo
                aiTextureType_NORMALS,          // 6  PBR Normals
                aiTextureType_HEIGHT,           // 5  Sometimes used for Normals in OBJ
                aiTextureType_METALNESS,        // 15 PBR Metallic
                aiTextureType_DIFFUSE_ROUGHNESS,// 16 PBR Roughness
                aiTextureType_AMBIENT_OCCLUSION,// 17 PBR AO
                aiTextureType_SHININESS         // 7  Shininess
            };

            for (auto& type : textureTypes)
            {
                for(unsigned int i = 0; i < mat->GetTextureCount(type); i++)
                {
                    aiString str;
                    mat->GetTexture(type, i, &str);

                    std::string filename = std::string(str.C_Str());
                    std::string fullPath = directory + "/" + filename;
                    
                    // Fallback to .ktx / .ktx2 format handling
                    size_t lastindex = fullPath.find_last_of("."); 
                    std::string ktxExtensionPath = fullPath.substr(0, lastindex) + ".ktx";
                    std::string ktxExtensionPath2 = fullPath.substr(0, lastindex) + ".ktx2";

                    std::string finalLoadPath = fullPath;
                    if (std::filesystem::exists(ktxExtensionPath))       finalLoadPath = ktxExtensionPath;
                    else if (std::filesystem::exists(ktxExtensionPath2)) finalLoadPath = ktxExtensionPath2;

                    // Load the texture asset. 
                    // Note: If you fully migrated to UUIDs for textures too, this would be:
                    UUID texHandle = AssetManager::ImportAsset(finalLoadPath);
                    Shared<Texture2D> texture = AssetManager::GetAsset<Texture2D>(texHandle);
                    
                    TextureType texType = AssimpToTextureType(type);
                    if (texType == TextureType::Diffuse)  material->AlbedoMap = texture;
                    if (texType == TextureType::Normal)   material->NormalMap = texture;
                    if (texType == TextureType::Metallic) material->MetallicMap = texture;
                    if (texType == TextureType::AO)       material->AOMap = texture;
                }
            }

            return material;
        }
    };

};
