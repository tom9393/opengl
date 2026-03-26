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
                aiMesh* mesh = flatMeshes[i];
                
                results.push_back(tp.Enqueue([this, mesh, scene, i]() {
                    VertexLayout vertexLayout;
                    std::vector<float> vertices;
                    std::vector<unsigned int> indices;

                    // This is the heavy math/parsing part, safe for threads
                    LoadVertices(vertices, mesh, scene, vertexLayout);
                    LoadIndices(indices, mesh);

                    BufferLayout bufferLayout(vertexLayout.GetElements());
                    TextureLayout emptyTextureLayout;

                    m_Meshes[i] = MakeShared<Mesh>(vertices, indices, bufferLayout, emptyTextureLayout);
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

        void LoadVertices(std::vector<float> &vertices, aiMesh* mesh, const aiScene* scene, VertexLayout &vertexLayout)
        {
            aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
            aiColor4D diffuseColor(1.0f, 1.0f, 1.0f, 1.0f);
            aiReturn col = aiGetMaterialColor(material, AI_MATKEY_COLOR_DIFFUSE, &diffuseColor);
            for(unsigned int i = 0; i < mesh->mNumVertices; i++)
            {
                vertexLayout.AddVertexAttrib(VertexAttribute::Vertex, ShaderDataType::Float3, "a_Position");
                vertices.emplace_back(mesh->mVertices[i].x);
                vertices.emplace_back(mesh->mVertices[i].y);
                vertices.emplace_back(mesh->mVertices[i].z);

                if (mesh->HasNormals())
                {
                    vertexLayout.AddVertexAttrib(VertexAttribute::Normal, ShaderDataType::Float3, "a_Normal");
                    vertices.emplace_back(mesh->mNormals[i].x);
                    vertices.emplace_back(mesh->mNormals[i].y);
                    vertices.emplace_back(mesh->mNormals[i].z);
                }
                if (mesh->mTextureCoords[0])
                {
                    vertexLayout.AddVertexAttrib(VertexAttribute::Texture, ShaderDataType::Float2, "a_TexCoord");
                    vertices.emplace_back(mesh->mTextureCoords[0][i].x);
                    vertices.emplace_back(mesh->mTextureCoords[0][i].y);

                    if (mesh->HasTangentsAndBitangents())
                    {
                        vertexLayout.AddVertexAttrib(VertexAttribute::Tangent, ShaderDataType::Float3, "a_Tangent");
                        vertices.emplace_back(mesh->mTangents[i].x);
                        vertices.emplace_back(mesh->mTangents[i].y);
                        vertices.emplace_back(mesh->mTangents[i].z);

                        vertexLayout.AddVertexAttrib(VertexAttribute::Bitangent, ShaderDataType::Float3, "a_Bitangent");
                        vertices.emplace_back(mesh->mBitangents[i].x);
                        vertices.emplace_back(mesh->mBitangents[i].y);
                        vertices.emplace_back(mesh->mBitangents[i].z);
                    }
                }
                if (AI_SUCCESS == col) {
                    vertexLayout.AddVertexAttrib(VertexAttribute::Color, ShaderDataType::Float4, "a_Color");
                    vertices.emplace_back(diffuseColor.r);
                    vertices.emplace_back(diffuseColor.g);
                    vertices.emplace_back(diffuseColor.b);
                    vertices.emplace_back(diffuseColor.a);
                }
            }
        }

        void LoadIndices(std::vector<unsigned int> &indices, aiMesh* mesh)
        {
            for(unsigned int i = 0; i < mesh->mNumFaces; i++)
            {
                aiFace face = mesh->mFaces[i];
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
                aiTextureType_DIFFUSE,          // Often used as Albedo in legacy files
                aiTextureType_BASE_COLOR,       // Modern Albedo
                aiTextureType_NORMALS,          // PBR Normals
                aiTextureType_HEIGHT,           // Sometimes used for Normals in OBJ
                aiTextureType_METALNESS,        // PBR Metallic
                aiTextureType_DIFFUSE_ROUGHNESS,// PBR Roughness
                aiTextureType_AMBIENT_OCCLUSION // PBR AO
            };

            for (auto &type : textureTypes)
            {
                for(unsigned int i = 0; i < mat->GetTextureCount(type); i++)
                {
                    aiString str;
                    mat->GetTexture(type, i, &str);

                    std::string filename = std::string(str.C_Str());
                    filename = m_Directory + "/" + filename;

                    Shared<Texture2D> texture = std::static_pointer_cast<Texture2D>(AssetManager::LoadAsset(filename));
                    layout.AddTexture(AssimpToTextureType(type), texture);
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
