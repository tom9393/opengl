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
            
            const aiScene* assimpScene = importer.ReadFile(s_Path, 
                aiProcess_Triangulate | aiProcess_SortByPType | aiProcess_GenSmoothNormals | 
                aiProcess_CalcTangentSpace | aiProcess_JoinIdenticalVertices | 
                aiProcess_OptimizeGraph | aiProcess_OptimizeMeshes | aiProcess_ImproveCacheLocality);

            if(!assimpScene || assimpScene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !assimpScene->mRootNode) {
                std::cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
                return nullptr;
            }

            s_Directory = s_Path.substr(0, s_Path.find_last_of('/'));
            Shared<Scene> scene = MakeShared<Scene>();

            std::cout << "Loading: " << path << std::endl;

            // Start the recursive node processing from the root
            Entity nullParent; // Empty entity evaluates to false in operator bool()
            ProcessNode(assimpScene->mRootNode, assimpScene, scene, nullParent);

            std::cout << "Done" << std::endl;
            return scene;
        }

        private:

        // --- NEW: Assimp Matrix to GLM Matrix Conversion ---
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

        // --- NEW: Recursive Node Traversal ---
        static void ProcessNode(aiNode* node, const aiScene* assimpScene, Shared<Scene>& scene, Entity parentEntity)
        {
            // 1. Create the entity for this node
            Entity entity = scene->CreateEntity(node->mName.C_Str());

            // 2. Set Hierarchy
            if (parentEntity)
            {
                entity.SetParent(parentEntity);
            }

            // 3. Apply Local Transform
            if (!entity.HasComponent<TransformComponent>())
                entity.AddComponent<TransformComponent>();
                
            auto& transform = entity.GetComponent<TransformComponent>();
            transform.ApplyMatrix(AssimpToGLMMatrix(node->mTransformation));

            // 4. Handle Meshes
            if (node->mNumMeshes == 1)
            {
                // Single mesh: Attach directly to this node's entity
                aiMesh* aiMesh = assimpScene->mMeshes[node->mMeshes[0]];
                LoadMesh(entity, aiMesh, assimpScene);
                LoadMaterialTextures(entity, assimpScene->mMaterials[aiMesh->mMaterialIndex]);
            }
            else if (node->mNumMeshes > 1)
            {
                // Multiple meshes: Create sub-entities because our ECS expects 1 mesh per entity
                for (unsigned int i = 0; i < node->mNumMeshes; i++)
                {
                    aiMesh* aiMesh = assimpScene->mMeshes[node->mMeshes[i]];
                    std::string childName = std::string(node->mName.C_Str()) + "_Mesh" + std::to_string(i);
                    Entity meshEntity = scene->CreateEntity(childName);
                    
                    meshEntity.SetParent(entity);
                    LoadMesh(meshEntity, aiMesh, assimpScene);
                    LoadMaterialTextures(meshEntity, assimpScene->mMaterials[aiMesh->mMaterialIndex]);
                }
            }

            // 5. Recursively process children
            for (unsigned int i = 0; i < node->mNumChildren; i++)
            {
                ProcessNode(node->mChildren[i], assimpScene, scene, entity);
            }
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

            entity.AddComponent<MeshComponent>(mesh);
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

            if (!entity.HasComponent<MaterialComponent>())
                entity.AddComponent<MaterialComponent>();

            Shared<Material> material = MakeShared<Material>();

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

                    if (std::filesystem::exists(ktxExtensionPath))
                    {
                        Shared<Texture2D> texture = std::static_pointer_cast<Texture2D>(AssetManager::LoadAsset(ktxExtensionPath));
                        TextureType texType = AssimpToTextureType(type);
                        if (texType == TextureType::Diffuse)  material->AlbedoMap = texture;
                        if (texType == TextureType::Normal)   material->NormalMap = texture;
                        if (texType == TextureType::Metallic) material->MetallicMap = texture;
                        if (texType == TextureType::AO)       material->AOMap = texture;
                    }
                    else if (std::filesystem::exists(ktxExtensionPath2))
                    {
                        Shared<Texture2D> texture = std::static_pointer_cast<Texture2D>(AssetManager::LoadAsset(ktxExtensionPath2));
                        TextureType texType = AssimpToTextureType(type);
                        if (texType == TextureType::Diffuse)  material->AlbedoMap = texture;
                        if (texType == TextureType::Normal)   material->NormalMap = texture;
                        if (texType == TextureType::Metallic) material->MetallicMap = texture;
                        if (texType == TextureType::AO)       material->AOMap = texture;
                    }
                    else
                    {
                        Shared<Texture2D> texture = std::static_pointer_cast<Texture2D>(AssetManager::LoadAsset(filename));
                        TextureType texType = AssimpToTextureType(type);
                        if (texType == TextureType::Diffuse)  material->AlbedoMap = texture;
                        if (texType == TextureType::Normal)   material->NormalMap = texture;
                        if (texType == TextureType::Metallic) material->MetallicMap = texture;
                        if (texType == TextureType::AO)       material->AOMap = texture;
                    };
                }
            }
            auto& materialComp = entity.GetComponent<MaterialComponent>();
        }

        private:
        inline static std::string s_Directory;
        inline static std::string s_Path;
    };
};
