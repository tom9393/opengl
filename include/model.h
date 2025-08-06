#pragma once

#include <assimp/Importer.hpp>
#include <assimp/material.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "../pch.h"
#include "../include/mesh.h"
#include "../include/preprocessor.h"
#include "BufferLayout.h"
#include "Texture.h"
#include "stb_image.h"
#include "Transformable.h"

class Model
{
public:
    // model data 
    std::vector<std::shared_ptr<Mesh>> m_Meshes;
    std::string m_Directory;
    bool m_GammaCorrection;

    // constructor, expects a filepath to a 3D model.
    Model(std::string const &path, bool gamma = false) : m_GammaCorrection(gamma)
    {
        loadModel(path);
    }

    std::vector<std::shared_ptr<Mesh>> GetMeshes() { return m_Meshes; };
    
private:
    // loads a model with supported ASSIMP extensions from file and stores the resulting meshes in the meshes vector.
    void loadModel(std::string const &path)
    {
        // read file via ASSIMP
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace);
        // check for errors
        if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // if is Not Zero
        {
            std::cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
            return;
        }
        // retrieve the directory path of the filepath
        m_Directory = path.substr(0, path.find_last_of('/'));

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
        // data to fill
        std::vector<float> vertices;
        std::vector<unsigned int> indices;
        vertices.reserve(mesh->mNumVertices * 3);
        indices.reserve(mesh->mNumFaces * 3); // Each triangle has 3 faces
        
        glm::vec3 min;
        glm::vec3 max;

        // walk through each of the mesh's vertices
        for(unsigned int i = 0; i < mesh->mNumVertices; i++)
        {
            // positions
            vertices.push_back(mesh->mVertices[i].x);
            vertices.push_back(mesh->mVertices[i].y);
            vertices.push_back(mesh->mVertices[i].z);
            glm::vec3 tmp = {mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z};
            min = glm::min(min, tmp);
            max = glm::max(max, tmp);

            // normals
            if (mesh->HasNormals())
            {
                vertices.push_back(mesh->mNormals[i].x);
                vertices.push_back(mesh->mNormals[i].y);
                vertices.push_back(mesh->mNormals[i].z);
            }
            // texture coordinates
            if(mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
            {
                // a vertex can contain up to 8 different texture coordinates. We thus make the assumption that we won't 
                // use models where a vertex can have multiple texture coordinates so we always take the first set (0).
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
        // now wak through each of the mesh's faces (a face is a mesh its triangle) and retrieve the corresponding vertex indices.
        for(unsigned int i = 0; i < mesh->mNumFaces; i++)
        {
            aiFace face = mesh->mFaces[i];
            // retrieve all indices of the face and store them in the indices vector
            for(unsigned int j = 0; j < face.mNumIndices; j++)
                indices.push_back(face.mIndices[j]);        
        }
        // process materials
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];    
        // we assume a convention for sampler names in the shaders. Each diffuse texture should be named
        // as 'texture_diffuseN' where N is a sequential number ranging from 1 to MAX_SAMPLER_NUMBER. 
        // Same applies to other texture as the following list summarizes:
        // diffuse: texture_diffuseN
        // specular: texture_specularN
        // normal: texture_normalN
        //

        std::vector<std::shared_ptr<Texture2D>> textures;
        // 1. diffuse maps
        std::vector<std::shared_ptr<Texture2D>> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE);
        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
        // 2. specular maps
        std::vector<std::shared_ptr<Texture2D>> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR);
        textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
        // 3. normal maps
        std::vector<std::shared_ptr<Texture2D>> normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT);
        textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
        // 4. height maps
        std::vector<std::shared_ptr<Texture2D>> heightMaps = loadMaterialTextures(material, aiTextureType_AMBIENT);
        textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());

        BufferLayout layout = {
            {ShaderDataType::Float3, "a_Position"},
            {ShaderDataType::Float3, "a_Normal"},
            {ShaderDataType::Float2, "a_TexCoord"},
            {ShaderDataType::Float3, "a_Tangent"},
            {ShaderDataType::Float3, "a_Bitangent"}
        };

        return std::make_shared<Mesh>(vertices, indices, textures, layout, min, max);
    }

    // checks all material textures of a given type and loads the textures if they're not loaded yet.
    // the required info is returned as a Texture struct.
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

