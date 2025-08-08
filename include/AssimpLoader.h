#pragma once

#include <assimp/Importer.hpp>
#include <assimp/material.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "../pch.h"
#include "../include/mesh.h"
#include "../include/preprocessor.h"
#include "BufferLayout.h"
#include "ModelLoader.h"
#include "Texture.h"
#include "stb_image.h"

class AssimpLoader : public ModelLoader
{
public:
    std::vector<std::shared_ptr<Mesh>> m_Meshes;
    std::string m_Directory;
    std::string m_Path;

    std::shared_ptr<Model> Load(const char* path) override
    {
        m_Path = std::string(path);
        stbi_set_flip_vertically_on_load(true);  
        loadModel();
        return std::make_shared<Model>(m_Meshes);
    }
    
private:

    void loadModel()
    {
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(m_Path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace);
        // check for errors
        if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // if is Not Zero
        {
            std::cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
            return;
        }
        // retrieve the directory path of the filepath
        m_Directory = m_Path.substr(0, m_Path.find_last_of('/'));

        // process ASSIMP's root node recursively
        processNode(scene->mRootNode, scene);
    }

    // processes a node in a recursive fashion. Processes each individual mesh located at the node and repeats this process on its children nodes (if any).
    void processNode(aiNode *node, const aiScene *scene)
    {
        // process each mesh located at the current node
        for(unsigned int i = 0; i < node->mNumMeshes; i++)
        {
            // the node object only contains indices to index the actual objects in the scene. 
            // the scene contains all the data, node is just to keep stuff organized (like relations between nodes).
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            m_Meshes.emplace_back(processMesh(mesh, scene));
        }
        // after we've processed all of the meshes (if any) we then recursively process each of the children nodes
        for(unsigned int i = 0; i < node->mNumChildren; i++)
        {
            processNode(node->mChildren[i], scene);
        }

    }

    std::shared_ptr<Mesh> processMesh(aiMesh *mesh, const aiScene *scene)
    {
        std::vector<float> vertices;
        std::vector<unsigned int> indices;
        vertices.reserve(mesh->mNumVertices * 3);
        indices.reserve(mesh->mNumFaces * 3); // Each triangle has 3 faces
        
        glm::vec3 min;
        glm::vec3 max;

        for(unsigned int i = 0; i < mesh->mNumVertices; i++)
        {
            vertices.push_back(mesh->mVertices[i].x);
            vertices.push_back(mesh->mVertices[i].y);
            vertices.push_back(mesh->mVertices[i].z);
            glm::vec3 tmp = {mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z};
            min = glm::min(min, tmp);
            max = glm::max(max, tmp);

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
        }

        for(unsigned int i = 0; i < mesh->mNumFaces; i++)
        {
            aiFace face = mesh->mFaces[i];
            for(unsigned int j = 0; j < face.mNumIndices; j++)
                indices.push_back(face.mIndices[j]);        
        }

        std::vector<aiTextureType> textureTypes = { aiTextureType_DIFFUSE, aiTextureType_SPECULAR, aiTextureType_HEIGHT, aiTextureType_AMBIENT };
        std::vector<std::shared_ptr<Texture2D>> textures;
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];    

        for (auto &textureType : textureTypes)
        {
            std::vector<std::shared_ptr<Texture2D>> tmp = loadMaterialTextures(material, textureType);
            textures.insert(textures.end(), tmp.begin(), tmp.end());
        }

        TextureLayout textureLayout(textures);

        BufferLayout bufferLayout = {
            {ShaderDataType::Float3, "a_Position"},
            {ShaderDataType::Float3, "a_Normal"},
            {ShaderDataType::Float2, "a_TexCoord"},
            {ShaderDataType::Float3, "a_Tangent"},
            {ShaderDataType::Float3, "a_Bitangent"}
        };

        return std::make_shared<Mesh>(vertices, indices, bufferLayout, textureLayout, min, max);
    }

    std::vector<std::shared_ptr<Texture2D>> loadMaterialTextures(aiMaterial *mat, aiTextureType type)
    {
        std::vector<std::shared_ptr<Texture2D>> textures;
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

            textures.emplace_back(Texture2D::Create(filename, txtType));
        }
        return textures;
    }
};

