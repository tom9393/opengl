#pragma once

#include "../pch.h"
#include <assimp/Importer.hpp>
#include <assimp/material.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "AssetManager.h"
#include "Mesh.h"
#include "BufferLayout.h"
#include "ModelLoader.h"
#include "Texture.h"

namespace Engine
{

    class AssimpLoader : public ModelLoader
    {
        public:
        Shared<Model> Load(const char* path, int id) override
        {
            m_Meshes.clear();
            m_Path = std::string(path);
            m_EntityID = id;
            stbi_set_flip_vertically_on_load(true);
            auto clock_start = std::chrono::system_clock::now();
            LoadModel();
            auto clock_now = std::chrono::system_clock::now();
            auto res = clock_now - clock_start;
            std::cout << "Time in ms to load model: " << res.count() / 1000000 << std::endl;
            return MakeShared<Model>(m_Meshes, m_Path);
        }
        
        private:
        void LoadModel()
        {
            Assimp::Importer importer;
            const aiScene* scene = importer.ReadFile(m_Path, aiProcess_Triangulate | aiProcess_SortByPType | aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace | aiProcess_JoinIdenticalVertices | aiProcess_OptimizeGraph | aiProcess_OptimizeMeshes | aiProcess_ImproveCacheLocality);

            if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
                std::cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
                return;
            }

            m_Directory = m_Path.substr(0, m_Path.find_last_of('/'));
            ProcessNode(scene->mRootNode, scene);
        }

        void ProcessNode(aiNode *node, const aiScene *scene)
        {
            for(unsigned int i = 0; i < node->mNumMeshes; i++)
            {
                aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
                m_Meshes.emplace_back(ProcessMesh(mesh, scene));
            }

            for(unsigned int i = 0; i < node->mNumChildren; i++)
                ProcessNode(node->mChildren[i], scene);

        }

        Shared<Mesh> ProcessMesh(aiMesh *mesh, const aiScene *scene)
        {
            VertexLayout vertexLayout;
            TextureLayout textureLayout;
            std::vector<float> vertices;
            std::vector<unsigned int> indices;

            vertices.reserve(mesh->mNumVertices * 10);
            indices.reserve(mesh->mNumFaces * 3); // Three indices per face
            
            LoadVertices(vertices, mesh, scene, vertexLayout);
            LoadIndices(indices, mesh);
            LoadMaterialTextures(scene->mMaterials[mesh->mMaterialIndex], textureLayout);

            BufferLayout bufferLayout(vertexLayout.GetElements());

            return MakeShared<Mesh>(std::move(vertices), std::move(indices), bufferLayout, textureLayout);
        }

        void LoadVertices(std::vector<float> &vertices, aiMesh* mesh, const aiScene* scene, VertexLayout &vertexLayout)
        {
            aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
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

                aiColor4D diffuseColor(1.0f, 1.0f, 1.0f, 1.0f);
                if (AI_SUCCESS == aiGetMaterialColor(material, AI_MATKEY_COLOR_DIFFUSE, &diffuseColor)) {
                    vertexLayout.AddVertexAttrib(VertexAttribute::Color, ShaderDataType::Float4, "a_Color");
                    vertices.emplace_back(diffuseColor.r);
                    vertices.emplace_back(diffuseColor.g);
                    vertices.emplace_back(diffuseColor.b);
                    vertices.emplace_back(diffuseColor.a);
                }

                vertexLayout.AddVertexAttrib(VertexAttribute::ID, ShaderDataType::Float, "a_EntityID");
                vertices.emplace_back(m_EntityID);
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

        private:
        std::vector<Shared<Mesh>> m_Meshes;
        std::string m_Directory;
        std::string m_Path;
        int m_EntityID;
    };

};
