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

    class AssimpLoader
    {
        public:
        static Shared<Scene> Load(const std::filesystem::path &path)
        {
            s_Path = std::string(path);
            stbi_set_flip_vertically_on_load(true);
            Assimp::Importer importer;
            const aiScene* assimpScene = importer.ReadFile(s_Path, aiProcess_Triangulate | aiProcess_SortByPType | aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace | aiProcess_JoinIdenticalVertices | aiProcess_OptimizeGraph | aiProcess_OptimizeMeshes | aiProcess_ImproveCacheLocality);

            if(!assimpScene || assimpScene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !assimpScene->mRootNode) {
                std::cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
                return nullptr;
            }

            s_Directory = s_Path.substr(0, s_Path.find_last_of('/'));
            std::vector<aiMesh*> flatMeshes;
            CollectMeshes(assimpScene->mRootNode, assimpScene, flatMeshes);

            Shared<Scene> scene = MakeShared<Scene>();

            // Cpu only
            for (size_t i = 0; i < flatMeshes.size(); i++)
            {
                aiMesh* aiMesh = flatMeshes[i];

                Entity entity = scene->CreateEntity();
                
                LoadMesh(entity, aiMesh, assimpScene);
                LoadMaterialTextures(entity, assimpScene->mMaterials[aiMesh->mMaterialIndex]);

                // TextureLayout emptyTextureLayout;
                // mesh->SetTextureLayout(emptyTextureLayout);
            }
            std::cout << "DD" << std::endl;
            return scene;
        }

        static void LoadMesh(Entity &entity, aiMesh* aiMesh, const aiScene* scene)
        {
            aiMaterial* material = scene->mMaterials[aiMesh->mMaterialIndex];
            glm::vec3 minExtents(std::numeric_limits<float>::max());
            glm::vec3 maxExtents(std::numeric_limits<float>::lowest());

            Shared<Mesh> mesh = MakeShared<Mesh>();

            std::vector<float> &vertices = mesh->GetVertices();

            BufferLayout meshLayout = {
            { ShaderDataType::Float3, "a_Position" },
            { ShaderDataType::Float3, "a_Normal" },
            { ShaderDataType::Float2, "a_TexCoord" },
            { ShaderDataType::Float3, "a_Tangent" },
            { ShaderDataType::Float3, "a_Bitangent" }
            };

            // A standard PBR vertex is exactly 14 floats
            const size_t floatsPerVertex = 14; 
            vertices.reserve(aiMesh->mNumVertices * floatsPerVertex);

            bool hasNormals = aiMesh->HasNormals();
            bool hasUVs = aiMesh->mTextureCoords[0] != nullptr;
            bool hasTangents = aiMesh->HasTangentsAndBitangents();

            for(unsigned int i = 0; i < aiMesh->mNumVertices; i++)
            {
                // Calculate the bounds for this specific vertex
                glm::vec3 pos(aiMesh->mVertices[i].x, aiMesh->mVertices[i].y, aiMesh->mVertices[i].z);
                minExtents = glm::min(minExtents, pos);
                maxExtents = glm::max(maxExtents, pos);

                // --- 1. POSITION (Location 0) ---
                vertices.emplace_back(aiMesh->mVertices[i].x);
                vertices.emplace_back(aiMesh->mVertices[i].y);
                vertices.emplace_back(aiMesh->mVertices[i].z);

                // --- 2. NORMAL (Location 1) ---
                if (hasNormals) {
                    vertices.emplace_back(aiMesh->mNormals[i].x);
                    vertices.emplace_back(aiMesh->mNormals[i].y);
                    vertices.emplace_back(aiMesh->mNormals[i].z);
                } else {
                    // Dummy normal pointing straight up
                    vertices.emplace_back(0.0f); vertices.emplace_back(1.0f); vertices.emplace_back(0.0f); 
                }

                // --- 3. UVs (Location 2) ---
                if (hasUVs) {
                    vertices.emplace_back(aiMesh->mTextureCoords[0][i].x);
                    vertices.emplace_back(aiMesh->mTextureCoords[0][i].y);
                } else {
                    // Dummy UVs
                    vertices.emplace_back(0.0f); vertices.emplace_back(0.0f);
                }

                // --- 4. TANGENT & BITANGENT (Location 3 & 4) ---
                if (hasTangents) {
                    vertices.emplace_back(aiMesh->mTangents[i].x);
                    vertices.emplace_back(aiMesh->mTangents[i].y);
                    vertices.emplace_back(aiMesh->mTangents[i].z);
                    
                    vertices.emplace_back(aiMesh->mBitangents[i].x);
                    vertices.emplace_back(aiMesh->mBitangents[i].y);
                    vertices.emplace_back(aiMesh->mBitangents[i].z);
                } else {
                    // DUMMY TANGENTS (Crucial: Must NOT be 0,0,0 to prevent NaN explosions!)
                    vertices.emplace_back(1.0f); vertices.emplace_back(0.0f); vertices.emplace_back(0.0f); // Tangent
                    vertices.emplace_back(0.0f); vertices.emplace_back(0.0f); vertices.emplace_back(1.0f); // Bitangent
                }
            }

            std::vector<unsigned int> &indices = mesh->GetIndices();
            indices.reserve(aiMesh->mNumFaces * 3);
            for(unsigned int i = 0; i < aiMesh->mNumFaces; i++)
            {
                aiFace face = aiMesh->mFaces[i];
                for(unsigned int j = 0; j < face.mNumIndices; j++)
                    indices.emplace_back(face.mIndices[j]);        
            }

            AABB aabb{ minExtents, maxExtents };
            BufferLayout bufferLayout(meshLayout);
            mesh->SetLayout(bufferLayout);
            mesh->SetAABB(aabb);
        }

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

        static void LoadMaterialTextures(Entity &entity, aiMaterial *mat)
        {
            std::vector<aiTextureType> textureTypes = { 
                aiTextureType_DIFFUSE,          // 1  Often used as Albedo in legacy files
                aiTextureType_BASE_COLOR,       // 12 Modern Albedo
                aiTextureType_NORMALS,          // 6  PBR Normals
                aiTextureType_HEIGHT,           // 5  Sometimes used for Normals in OBJ
                aiTextureType_METALNESS,        // 15 PBR Metallic
                aiTextureType_DIFFUSE_ROUGHNESS,// 16 PBR Roughness
                aiTextureType_AMBIENT_OCCLUSION, //17 PBR AO
                aiTextureType_SHININESS          // 7 Shininess
            };

            for (auto &type : textureTypes)
            {
                for(unsigned int i = 0; i < mat->GetTextureCount(type); i++)
                {
                    aiString str;
                    mat->GetTexture(type, i, &str);

                    std::string filename = std::string(str.C_Str());
                    filename = s_Directory + "/" + filename;

                    size_t lastindex = filename.find_last_of("."); 

                    std::string ktxExtensionPath = filename.substr(0, lastindex).append(".ktx");
                    std::string ktxExtensionPath2 = filename.substr(0, lastindex).append(".ktx2");

                    std::cout << type << std::endl;

                    if (std::filesystem::exists(ktxExtensionPath))
                    {
                        Shared<Texture2D> texture = std::static_pointer_cast<Texture2D>(AssetManager::LoadAsset(ktxExtensionPath));
                        //layout.AddTexture(AssimpToTextureType(type), texture);
                    }
                    else if (std::filesystem::exists(ktxExtensionPath2))
                    {
                        Shared<Texture2D> texture = std::static_pointer_cast<Texture2D>(AssetManager::LoadAsset(ktxExtensionPath2));
                        //layout.AddTexture(AssimpToTextureType(type), texture);
                    }
                    else
                    {
                        Shared<Texture2D> texture = std::static_pointer_cast<Texture2D>(AssetManager::LoadAsset(filename));
                        //layout.AddTexture(AssimpToTextureType(type), texture);
                    };
                }
            }
        }

        static void CollectMeshes(aiNode* node, const aiScene* scene, std::vector<aiMesh*>& outMeshes)
        {
            for(unsigned int i = 0; i < node->mNumMeshes; i++) {
                outMeshes.push_back(scene->mMeshes[node->mMeshes[i]]);
            }
            for(unsigned int i = 0; i < node->mNumChildren; i++) {
                CollectMeshes(node->mChildren[i], scene, outMeshes);
            }
        }

        private:
        inline static std::string s_Directory;
        inline static std::string s_Path;
    };
};
