#pragma once

#include "../pch.h"
#include "Application.h"
#include "Model.h"
#include <assimp/Importer.hpp>
#include <assimp/material.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "AssetManager.h"
#include "Mesh.h"
#include "BufferLayout.h"
#include "Texture.h"

namespace Engine
{

    class AssimpLoader
    {
        public:
        Shared<Model> Load(const std::filesystem::path &path)
        {
            m_Meshes.clear();
            m_Path = std::string(path);
            stbi_set_flip_vertically_on_load(true);
            Assimp::Importer importer;
            const aiScene* scene = importer.ReadFile(m_Path, aiProcess_Triangulate | aiProcess_SortByPType | aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace | aiProcess_JoinIdenticalVertices | aiProcess_OptimizeGraph | aiProcess_OptimizeMeshes | aiProcess_ImproveCacheLocality);

            if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
                std::cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
                return nullptr;
            }

            m_Directory = m_Path.substr(0, m_Path.find_last_of('/'));
            std::vector<aiMesh*> flatMeshes;
            CollectMeshes(scene->mRootNode, scene, flatMeshes);
            m_Meshes.resize(flatMeshes.size());

            std::vector<std::future<void>> results;
            ThreadPool& tp = Application::Get().GetThreadPool();

            // Cpu only
            for (size_t i = 0; i < flatMeshes.size(); i++)
            {
                aiMesh* aiMesh = flatMeshes[i];
                m_Meshes[i] = MakeShared<Mesh>();
                
                results.push_back(tp.Enqueue([this, aiMesh, scene, i]() {
                    auto& mesh = m_Meshes[i];
                    // This is the heavy math/parsing part, safe for threads
                    LoadVertices(mesh, aiMesh, scene);
                    LoadIndices(mesh, aiMesh);

                    TextureLayout emptyTextureLayout;
                    mesh->SetTextureLayout(emptyTextureLayout);

                    m_Meshes[i] = mesh;
                }));
            }

            for (auto& res : results) {
                res.wait();
            }

            // OpenGL calls & gpu uploads
            for (size_t i = 0; i < flatMeshes.size(); i++)
            {
                aiMesh* mesh = flatMeshes[i];
                
                // 1. Load Textures (This calls glGenTextures)
                LoadMaterialTextures(scene->mMaterials[mesh->mMaterialIndex], m_Meshes[i]->GetTextureLayout());
                
                // 2. Upload Mesh to GPU (This calls glGenVertexArrays/Buffers)
                m_Meshes[i]->Prepare();
            }
            return MakeShared<Model>(m_Meshes, m_Path);
        }

        void LoadVertices(Shared<Mesh> &mesh, aiMesh* aiMesh, const aiScene* scene)
        {
            aiMaterial* material = scene->mMaterials[aiMesh->mMaterialIndex];
            glm::vec3 minExtents(std::numeric_limits<float>::max());
            glm::vec3 maxExtents(std::numeric_limits<float>::lowest());

            VertexLayout vertexLayout;
            std::vector<float> &vertices = mesh->GetVertices();

            // 1. FORCE A STRICT LAYOUT (No more `if (hasUVs)` here!)
            vertexLayout.AddVertexAttrib(VertexAttribute::Vertex, ShaderDataType::Float3, "a_Position");
            vertexLayout.AddVertexAttrib(VertexAttribute::Normal, ShaderDataType::Float3, "a_Normal");
            vertexLayout.AddVertexAttrib(VertexAttribute::Texture, ShaderDataType::Float2, "a_TexCoord");
            vertexLayout.AddVertexAttrib(VertexAttribute::Tangent, ShaderDataType::Float3, "a_Tangent");
            vertexLayout.AddVertexAttrib(VertexAttribute::Bitangent, ShaderDataType::Float3, "a_Bitangent");

            // A standard PBR vertex is exactly 14 floats
            const size_t floatsPerVertex = 14; 
            vertices.reserve(aiMesh->mNumVertices * floatsPerVertex);

            bool hasNormals = aiMesh->HasNormals();
            bool hasUVs = aiMesh->mTextureCoords[0] != nullptr;
            bool hasTangents = aiMesh->HasTangentsAndBitangents();

            for(unsigned int i = 0; i < aiMesh->mNumVertices; i++)
            {
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
            AABB aabb{ minExtents, maxExtents };
            BufferLayout bufferLayout(vertexLayout.GetElements());
            mesh->SetLayout(bufferLayout);
            mesh->SetAABB(aabb);
        }

        void LoadIndices(Shared<Mesh> &mesh, aiMesh* aiMesh)
        {
            std::vector<unsigned int> &indices = mesh->GetIndices();
            for(unsigned int i = 0; i < aiMesh->mNumFaces; i++)
            {
                aiFace face = aiMesh->mFaces[i];
                for(unsigned int j = 0; j < face.mNumIndices; j++)
                    indices.emplace_back(face.mIndices[j]);        
            }
        }

        TextureType AssimpToTextureType(aiTextureType type)
        {
            if (type == aiTextureType_DIFFUSE || type == aiTextureType_BASE_COLOR) return TextureType::Diffuse;
            if (type == aiTextureType_SPECULAR) return TextureType::Specular;
            if (type == aiTextureType_NORMALS || type == aiTextureType_HEIGHT) return TextureType::Normal;
            if (type == aiTextureType_METALNESS) return TextureType::Metallic;
            if (type == aiTextureType_AMBIENT_OCCLUSION) return TextureType::AO;

            return TextureType::None;
        }

        aiTextureType TypeToAssimpType(TextureType type)
        {
            if (type == TextureType::Diffuse) return aiTextureType_DIFFUSE;
            if (type == TextureType::Specular) return aiTextureType_SPECULAR;
            if (type == TextureType::Normal) return aiTextureType_NORMALS;
            if (type == TextureType::Metallic) return aiTextureType_METALNESS;
            if (type == TextureType::AO) return aiTextureType_AMBIENT_OCCLUSION;

            return aiTextureType_NONE;
        }

        void LoadMaterialTextures(aiMaterial *mat, TextureLayout &layout)
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
                    filename = m_Directory + "/" + filename;

                    size_t lastindex = filename.find_last_of("."); 

                    std::string ktxExtensionPath = filename.substr(0, lastindex).append(".ktx");
                    std::string ktxExtensionPath2 = filename.substr(0, lastindex).append(".ktx2");

                    if (std::filesystem::exists(ktxExtensionPath))
                    {
                        Shared<Texture2D> texture = std::static_pointer_cast<Texture2D>(AssetManager::LoadAsset(ktxExtensionPath));
                        layout.AddTexture(AssimpToTextureType(type), texture);
                    }
                    else if (std::filesystem::exists(ktxExtensionPath2))
                    {
                        Shared<Texture2D> texture = std::static_pointer_cast<Texture2D>(AssetManager::LoadAsset(ktxExtensionPath2));
                        layout.AddTexture(AssimpToTextureType(type), texture);
                    }
                    else
                    {
                        Shared<Texture2D> texture = std::static_pointer_cast<Texture2D>(AssetManager::LoadAsset(filename));
                        layout.AddTexture(AssimpToTextureType(type), texture);
                    };
                }
            }
        }

        void CollectMeshes(aiNode* node, const aiScene* scene, std::vector<aiMesh*>& outMeshes)
        {
            for(unsigned int i = 0; i < node->mNumMeshes; i++) {
                outMeshes.push_back(scene->mMeshes[node->mMeshes[i]]);
            }
            for(unsigned int i = 0; i < node->mNumChildren; i++) {
                CollectMeshes(node->mChildren[i], scene, outMeshes);
            }
        }

        private:
        std::vector<Shared<Mesh>> m_Meshes;
        std::string m_Directory;
        std::string m_Path;
    };
};
