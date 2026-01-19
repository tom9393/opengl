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
            // check for errors
            if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // if is Not Zero
            {
                std::cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
                return;
            }
            // retrieve the directory path of the filepath
            m_Directory = m_Path.substr(0, m_Path.find_last_of('/'));

            // process ASSIMP's root node recursively
            ProcessNode(scene->mRootNode, scene);
        }

        // processes a node in a recursive fashion. Processes each individual mesh located at the node and repeats this process on its children nodes (if any).
        void ProcessNode(aiNode *node, const aiScene *scene)
        {
            // process each mesh located at the current node
            for(unsigned int i = 0; i < node->mNumMeshes; i++)
            {
                // the node object only contains indices to index the actual objects in the scene. 
                // the scene contains all the data, node is just to keep stuff organized (like relations between nodes).
                aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
                m_Meshes.emplace_back(ProcessMesh(mesh, scene));
            }
            // after we've processed all of the meshes (if any) we then recursively process each of the children nodes
            for(unsigned int i = 0; i < node->mNumChildren; i++)
            {
                ProcessNode(node->mChildren[i], scene);
            }

        }

        Shared<Mesh> ProcessMesh(aiMesh *mesh, const aiScene *scene)
        {
            std::vector<float> vertices;
            std::vector<unsigned int> indices;
            vertices.reserve(mesh->mNumVertices * 10);
            indices.reserve(mesh->mNumFaces * 3); // Each triangle has 3 faces
            
            for(unsigned int i = 0; i < mesh->mNumVertices; i++)
            {
                vertices.push_back(mesh->mVertices[i].x);
                vertices.push_back(mesh->mVertices[i].y);
                vertices.push_back(mesh->mVertices[i].z);

                if (mesh->HasNormals())
                {
                    vertices.push_back(mesh->mNormals[i].x);
                    vertices.push_back(mesh->mNormals[i].y);
                    vertices.push_back(mesh->mNormals[i].z);
                }
                if(mesh->mTextureCoords[0])
                {
                    vertices.push_back(mesh->mTextureCoords[0][i].x);
                    vertices.push_back(mesh->mTextureCoords[0][i].y);

                    vertices.push_back(mesh->mTangents[i].x);
                    vertices.push_back(mesh->mTangents[i].y);
                    vertices.push_back(mesh->mTangents[i].z);

                    vertices.push_back(mesh->mBitangents[i].x);
                    vertices.push_back(mesh->mBitangents[i].y);
                    vertices.push_back(mesh->mBitangents[i].z);
                }
                else
                {
                    vertices.push_back(0.0f);
                    vertices.push_back(0.0f);
                }
                vertices.push_back(m_EntityID);
            }

            for(unsigned int i = 0; i < mesh->mNumFaces; i++)
            {
                aiFace face = mesh->mFaces[i];
                for(unsigned int j = 0; j < face.mNumIndices; j++)
                    indices.push_back(face.mIndices[j]);        
            }

            std::vector<aiTextureType> textureTypes = { aiTextureType_DIFFUSE, aiTextureType_SPECULAR, aiTextureType_HEIGHT, aiTextureType_AMBIENT };
            std::vector<Shared<Texture2D>> textures;
            aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];    

            for (auto &textureType : textureTypes)
            {
                std::vector<Shared<Texture2D>> tmp = LoadMaterialTextures(material, textureType);
                textures.insert(textures.end(), tmp.begin(), tmp.end());
            }

            TextureLayout textureLayout(textures);

            BufferLayout bufferLayout = {
                {ShaderDataType::Float3, "a_Position"},
                {ShaderDataType::Float3, "a_Normal"},
                {ShaderDataType::Float2, "a_TexCoord"},
                {ShaderDataType::Float3, "a_Tangent"},
                {ShaderDataType::Float3, "a_Bitangent"},
                {ShaderDataType::Float, "a_EntityID"}
            };

            return MakeShared<Mesh>(std::move(vertices), std::move(indices), bufferLayout, textureLayout);
        }

        std::vector<Shared<Texture2D>> LoadMaterialTextures(aiMaterial *mat, aiTextureType type)
        {
            std::vector<Shared<Texture2D>> textures;
            for(unsigned int i = 0; i < mat->GetTextureCount(type); i++)
            {
                aiString str;
                mat->GetTexture(type, i, &str);

                std::string filename = std::string(str.C_Str());
                filename = m_Directory + "/" + filename;

                TextureType txtType;
                if (type == aiTextureType_DIFFUSE) txtType = TextureType::Diffuse;
                if (type == aiTextureType_SPECULAR) txtType = TextureType::Specular;
                if (type == aiTextureType_HEIGHT) txtType = TextureType::Normal;
                if (type == aiTextureType_AMBIENT) txtType = TextureType::Height;

                Shared<Texture2D> texture = std::static_pointer_cast<Texture2D>(AssetManager::LoadAsset(filename));
                texture->SetType(txtType);

                textures.emplace_back(texture);
            }
            return textures;
        }

        private:
        std::vector<Shared<Mesh>> m_Meshes;
        std::string m_Directory;
        std::string m_Path;
        int m_EntityID;
    };

};
