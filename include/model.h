#pragma once

#include <assimp/Importer.hpp>
#include <assimp/material.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "../pch.h"
#include "../include/mesh.h"
#include "../include/Shader.h"
#include "../include/preprocessor.h"
#include "BufferLayout.h"
#include "Texture.h"
#include "VertexArray.h"
#include "stb_image.h"
#include "Transformable.h"
#include "Vertex.h"
#include "Renderer.h"

class Model
{
public:
    // model data 
    std::vector<std::string> textures_loaded;	// stores all the textures loaded so far, optimization to make sure textures aren't loaded more than once.
    std::vector<Mesh>    meshes;
    std::string directory;
    bool gammaCorrection;

    // constructor, expects a filepath to a 3D model.
    Model(std::string const &path, bool gamma = false) : gammaCorrection(gamma)
    {
        stbi_set_flip_vertically_on_load(true);  
        loadModel(path);
    }

    // draws the model, and thus all its meshes
    void Draw(std::shared_ptr<Shader> shader)
    {
        glm::mat4 pos = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f));
        for(unsigned int i = 0; i < meshes.size(); i++)
        {
            uint32_t slot = 0;
            std::shared_ptr<VertexArray> VAO = meshes[i].GetVao();
            std::vector<std::shared_ptr<Texture2D>> textures = meshes[i].GetTextures();
            for (auto &texture : textures) // Work in progress
            {
                texture->Bind(slot++);
                // retrieve texture number (the N in diffuse_textureN)
                string number;
                string name = textures[i].type;
                if(name == "texture_diffuse") {
                    number = std::to_string(diffuseNr++);
                    glUniform1i(glGetUniformLocation(shader.GetProgram(), "material.diffuse"), i);
                }
                else if(name == "texture_specular") {
                    number = std::to_string(specularNr++); // transfer unsigned int to string
                    glUniform1i(glGetUniformLocation(shader.GetProgram(), "material.specular"), i);
                }
                else if(name == "texture_normal")
                    number = std::to_string(normalNr++); // transfer unsigned int to string
                else if(name == "texture_height")
                    number = std::to_string(heightNr++); // transfer unsigned int to string

                // now set the sampler to the correct texture unit
                // glUniform1i(glGetUniformLocation(shader.GetProgram(), (name + number).c_str()), i);
                // and finally bind the texture
                glBindTexture(GL_TEXTURE_2D, textures[i].id);
            }
            Renderer::Submit(VAO, shader, pos);
        }
    }
    
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
        directory = path.substr(0, path.find_last_of('/'));

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
            meshes.emplace_back(processMesh(mesh, scene));
        }
        // after we've processed all of the meshes (if any) we then recursively process each of the children nodes
        for(unsigned int i = 0; i < node->mNumChildren; i++)
        {
            processNode(node->mChildren[i], scene);
        }

    }

    Mesh processMesh(aiMesh *mesh, const aiScene *scene)
    {
        // data to fill
        std::vector<float> vertices;
        vertices.reserve(mesh->mNumVertices);

        std::vector<unsigned int> indices;
        indices.reserve(mesh->mNumFaces * 3); // Each triangle has 3 faces
        std::vector<std::shared_ptr<Texture2D>> textures;
        std::vector<BufferElement> bufferElements;

        bufferElements.emplace_back(ShaderDataType::Float3, "a_Position");

        if (mesh->HasNormals()) bufferElements.emplace_back(ShaderDataType::Float3, "a_Normal");
        if(mesh->mTextureCoords[0])
        {
            bufferElements.emplace_back(ShaderDataType::Float2, "a_Texture");
            bufferElements.emplace_back(ShaderDataType::Float3, "a_Tangent");
            bufferElements.emplace_back(ShaderDataType::Float3, "a_Bitangent");
        }
        else
        {
            bufferElements.emplace_back(ShaderDataType::Float2, "a_Texture");
        }

        // walk through each of the mesh's vertices
        for(unsigned int i = 0; i < mesh->mNumVertices; i++)
        {
            // positions
            vertices.emplace_back(mesh->mVertices[i].x);
            vertices.emplace_back(mesh->mVertices[i].y);
            vertices.emplace_back(mesh->mVertices[i].z);
            // normals
            if (mesh->HasNormals())
            {
                vertices.emplace_back(mesh->mNormals[i].x);
                vertices.emplace_back(mesh->mNormals[i].y);
                vertices.emplace_back(mesh->mNormals[i].z);
            }
            // texture coordinates
            if(mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
            {
                // a vertex can contain up to 8 different texture coordinates. We thus make the assumption that we won't 
                // use models where a vertex can have multiple texture coordinates so we always take the first set (0).
                vertices.emplace_back(mesh->mTextureCoords[0][i].x);
                vertices.emplace_back(mesh->mTextureCoords[0][i].y);

                vertices.emplace_back(mesh->mTangents[i].x);
                vertices.emplace_back(mesh->mTangents[i].y);
                vertices.emplace_back(mesh->mTangents[i].z);

                vertices.emplace_back(mesh->mBitangents[i].x);
                vertices.emplace_back(mesh->mBitangents[i].y);
                vertices.emplace_back(mesh->mBitangents[i].z);
            }
            else
            {
                vertices.emplace_back(0.0f);
                vertices.emplace_back(0.0f);
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

        // 1. diffuse maps
        std::vector<std::shared_ptr<Texture2D>> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
        // 2. specular maps
        std::vector<std::shared_ptr<Texture2D>> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
        textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
        // 3. normal maps
        std::vector<std::shared_ptr<Texture2D>> normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal");
        textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
        // 4. height maps
        std::vector<std::shared_ptr<Texture2D>> heightMaps = loadMaterialTextures(material, aiTextureType_AMBIENT, "texture_height");
        textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());

        BufferLayout layout(bufferElements);

        return Mesh(vertices, indices, textures, layout);
    }

    // checks all material textures of a given type and loads the textures if they're not loaded yet.
    // the required info is returned as a Texture struct.
    std::vector<std::shared_ptr<Texture2D>> loadMaterialTextures(aiMaterial *mat, aiTextureType type, std::string typeName)
    {

        std::vector<std::shared_ptr<Texture2D>> textures;
        for(unsigned int i = 0; i < mat->GetTextureCount(type); i++)
        {
            aiString str;
            mat->GetTexture(type, i, &str);
            TextureType txtType;
            if (type == aiTextureType_DIFFUSE) txtType = TextureType::Diffuse;
            if (type == aiTextureType_SPECULAR) txtType = TextureType::Specular;
            if (type == aiTextureType_HEIGHT) txtType = TextureType::Normal;
            if (type == aiTextureType_AMBIENT) txtType = TextureType::Height;

            std::shared_ptr<Texture2D> txt = Texture2D::Create(directory, txtType);
            textures.push_back(txt);
            textures_loaded.emplace_back(str.C_Str());  // store it as texture loaded for entire model, to ensure we won't unnecessary load duplicate textures.
        }
        return textures;
    }
};

