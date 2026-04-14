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
#include "../Render/Renderer.h"

namespace Engine
{

    struct GeometrySpecification
    {
        bool CalculateNormals;
        bool CalculateTangents;
        bool CalculateBitangents;
        bool ImportTextures;
        bool ImportAnimations;
    };

    static constexpr unsigned int s_AssimpFlags = aiProcess_Triangulate | aiProcess_SortByPType | aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace | aiProcess_JoinIdenticalVertices | aiProcess_OptimizeGraph | aiProcess_OptimizeMeshes | aiProcess_ImproveCacheLocality;
    //static constexpr unsigned int s_AssimpFlags = aiProcess_Triangulate | aiProcess_SortByPType | aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace | aiProcess_JoinIdenticalVertices | aiProcess_ImproveCacheLocality;

    static TextureType AssimpToTextureType(aiTextureType type)
    {
        // Albedo / Diffuse
        if (type == aiTextureType_DIFFUSE || type == aiTextureType_BASE_COLOR) 
            return TextureType::Diffuse;
        
        // Specular
        if (type == aiTextureType_SPECULAR) 
            return TextureType::Specular;

        // Normals / Height (OBJ files often use Height for Normals)
        if (type == aiTextureType_NORMALS || type == aiTextureType_HEIGHT || type == aiTextureType_NORMAL_CAMERA) 
            return TextureType::Normal;

        // Metallic / Shininess
        if (type == aiTextureType_METALNESS || type == aiTextureType_SHININESS) 
            return TextureType::Metallic;

        // Roughness
        if (type == aiTextureType_DIFFUSE_ROUGHNESS || type == aiTextureType_MAYA_SPECULAR_ROUGHNESS) 
            return TextureType::Roughness;

        // Ambient Occlusion / Lightmaps
        if (type == aiTextureType_AMBIENT_OCCLUSION || type == aiTextureType_LIGHTMAP || type == aiTextureType_AMBIENT) 
            return TextureType::AO;

        // Emissive
        if (type == aiTextureType_EMISSIVE || type == aiTextureType_EMISSION_COLOR) 
            return TextureType::Emissive;

        // Transparency / Opacity
        if (type == aiTextureType_OPACITY) 
            return TextureType::Transparency;

        return TextureType::None;
    }

    static aiTextureType TypeToAssimpType(TextureType type)
    {
        switch (type)
        {
            case TextureType::Diffuse:      return aiTextureType_BASE_COLOR;
            case TextureType::Specular:     return aiTextureType_SPECULAR;
            case TextureType::Normal:       return aiTextureType_NORMALS;
            case TextureType::Metallic:     return aiTextureType_METALNESS;
            case TextureType::Roughness:    return aiTextureType_DIFFUSE_ROUGHNESS;
            case TextureType::AO:           return aiTextureType_AMBIENT_OCCLUSION;
            case TextureType::Emissive:     return aiTextureType_EMISSION_COLOR;
            case TextureType::Transparency: return aiTextureType_OPACITY;
            default:                        return aiTextureType_NONE;
        }
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
        static Shared<Model> ScanModel(const std::filesystem::path& path)
        {
            Assimp::Importer importer;

            const aiScene* scene = importer.ReadFile(path.string(), s_AssimpFlags);

            if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
                std::cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
                return nullptr;
            }

            Shared<Model> model = MakeShared<Model>();

            for (uint32_t i = 0; i < scene->mNumMeshes; i++) {
                ModelMesh modelMesh;
                modelMesh.Handle = UUID();
                modelMesh.Index = i;
                modelMesh.Path = path.string();
                modelMesh.Name = scene->mMeshes[i]->mName.C_Str();

                if (modelMesh.Name.length() > 20)
                {
                    modelMesh.Name = modelMesh.Name.substr(0, 17) + "...";
                }

                uint32_t materialIndex = scene->mMeshes[i]->mMaterialIndex;

                if (materialIndex < scene->mNumMaterials) {
                    ModelMaterial modelMat;
                    modelMat.Handle = UUID();
                    modelMat.Index = materialIndex;
                    modelMat.Path = path.string();

                    aiMaterial* ai_mat = scene->mMaterials[materialIndex];
                    aiString matName;
                    ai_mat->Get(AI_MATKEY_NAME, matName);
                    if (ai_mat->Get(AI_MATKEY_NAME, matName) == AI_SUCCESS && matName.length > 0) {
                        modelMat.Name = matName.C_Str();
                    } else {
                        modelMat.Name = "Material_" + std::to_string(materialIndex);
                    }

                    modelMesh.Materials.emplace_back(modelMat);
                }
                model->Meshes.emplace_back(modelMesh);
            }

            return model;
        }

        static Shared<Mesh> LoadSubMesh(const std::filesystem::path& path, uint32_t submeshIndex)
        {
            Assimp::Importer importer;

            // We use heavy processing flags here because we actually need the geometry
            const aiScene* scene = importer.ReadFile(path, s_AssimpFlags);

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
            const aiScene* scene = importer.ReadFile(path.string(), s_AssimpFlags);

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

            bool hasPositions = aiMesh->HasPositions();
            bool hasNormals = aiMesh->HasNormals();
            bool hasUVs = aiMesh->mTextureCoords[0] != nullptr;
            bool hasTangents = aiMesh->HasTangentsAndBitangents();

            std::vector<BufferElement> layoutElements;
            size_t floatsPerVertex = 0;

            if (hasPositions) {
                layoutElements.push_back({ ShaderDataType::Float3, "a_Position" });
                floatsPerVertex += 3;
            }
            if (hasNormals) {
                layoutElements.push_back({ ShaderDataType::Float3, "a_Normal" });
                floatsPerVertex += 3;
            }
            if (hasUVs) {
                layoutElements.push_back({ ShaderDataType::Float2, "a_TexCoord" });
                floatsPerVertex += 2;
            }
            if (hasTangents) {
                layoutElements.push_back({ ShaderDataType::Float3, "a_Tangent" });
                layoutElements.push_back({ ShaderDataType::Float3, "a_Bitangent" });
                floatsPerVertex += 6;
            }

            // Create the dynamic BufferLayout
            BufferLayout meshLayout(layoutElements);

            // 3. Pre-allocate the exact amount of memory needed
            vertices.reserve(aiMesh->mNumVertices * floatsPerVertex);

            // 4. Extract data (No dummy values needed anymore)
            for(unsigned int i = 0; i < aiMesh->mNumVertices; i++)
            {
                if (hasPositions) {
                    glm::vec3 pos(aiMesh->mVertices[i].x, aiMesh->mVertices[i].y, aiMesh->mVertices[i].z);
                    minExtents = glm::min(minExtents, pos);
                    maxExtents = glm::max(maxExtents, pos);

                    vertices.emplace_back(pos.x); 
                    vertices.emplace_back(pos.y); 
                    vertices.emplace_back(pos.z);
                }

                if (hasNormals) {
                    vertices.emplace_back(aiMesh->mNormals[i].x); 
                    vertices.emplace_back(aiMesh->mNormals[i].y); 
                    vertices.emplace_back(aiMesh->mNormals[i].z);
                } 

                if (hasUVs) {
                    vertices.emplace_back(aiMesh->mTextureCoords[0][i].x); 
                    vertices.emplace_back(aiMesh->mTextureCoords[0][i].y);
                } 

                if (hasTangents) {
                    vertices.emplace_back(aiMesh->mTangents[i].x); 
                    vertices.emplace_back(aiMesh->mTangents[i].y); 
                    vertices.emplace_back(aiMesh->mTangents[i].z);
                    
                    vertices.emplace_back(aiMesh->mBitangents[i].x); 
                    vertices.emplace_back(aiMesh->mBitangents[i].y); 
                    vertices.emplace_back(aiMesh->mBitangents[i].z);
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

            Shared<VertexArray> VAO = VertexArray::Create();
            VAO->Bind();

            Shared<VertexBuffer> VBO = VertexBuffer::Create(&vertices[0], vertices.size() * sizeof(float));
            VBO->SetLayout(meshLayout);
            VAO->AddVertexBuffer(VBO);

            Shared<IndexBuffer> EBO = IndexBuffer::Create(&indices[0], indices.size());
            VAO->SetIndexBuffer(EBO);

            VAO->Unbind();

            std::vector<Shared<VertexBuffer>> VBOs = { VBO };

            mesh->SetVAO(VAO);
            mesh->SetVBOs(VBOs);
            mesh->SetEBO(EBO);
            mesh->SetAABB({ minExtents, maxExtents });

            return mesh;
        }

        static Shared<Material> ProcessAiMaterial(aiMaterial* mat, const std::string& directory)
        {
            Shared<Material> material = MakeShared<Material>();

            std::vector<aiTextureType> textureTypes = { 
                aiTextureType_DIFFUSE, aiTextureType_BASE_COLOR,
                aiTextureType_NORMALS, aiTextureType_HEIGHT, aiTextureType_NORMAL_CAMERA,
                aiTextureType_METALNESS, aiTextureType_SHININESS,
                aiTextureType_DIFFUSE_ROUGHNESS, aiTextureType_MAYA_SPECULAR_ROUGHNESS,
                aiTextureType_AMBIENT_OCCLUSION, aiTextureType_LIGHTMAP,
                aiTextureType_EMISSIVE, aiTextureType_EMISSION_COLOR,
                aiTextureType_OPACITY, aiTextureType_REFLECTION,
                aiTextureType_SHININESS
            };

            // --- 1 & 2. Kd & d: Diffuse Color and Opacity ---
            glm::vec4 albedoTint = glm::vec4(1.0f);
            aiColor4D color;
            if (AI_SUCCESS == aiGetMaterialColor(mat, AI_MATKEY_COLOR_DIFFUSE, &color)) {
                albedoTint.r = color.r; albedoTint.g = color.g; albedoTint.b = color.b;
            }
            float opacity = 1.0f;
            if (AI_SUCCESS == aiGetMaterialFloat(mat, AI_MATKEY_OPACITY, &opacity)) {
                albedoTint.a = opacity;
            }
            material->SetVec4("pbr.albedoTint", albedoTint);

            // --- 3. Ke: Emissive ---
            float emission = 0.0f;
            if (AI_SUCCESS == aiGetMaterialColor(mat, AI_MATKEY_COLOR_EMISSIVE, &color)) {
                // Average the RGB values to get a scalar emission intensity
                emission = (color.r + color.g + color.b) / 3.0f;
            }
            material->SetFloat("pbr.emission", emission);

            // --- 4. Ns: Shininess -> Roughness Conversion ---
            float roughness = 0.5f; // Safe default
            float shininess = 0.0f;
            if (AI_SUCCESS == aiGetMaterialFloat(mat, AI_MATKEY_SHININESS, &shininess)) {
                // Prevent division by zero. 
                // Math: High shininess (e.g., 714) = very smooth = low roughness.
                roughness = glm::sqrt(2.0f / (shininess + 2.0f));
            }
            material->SetFloat("pbr.roughnessMultiplier", roughness);

            // --- 5. Ks: Specular -> Metallic Guessing (Optional hack) ---
            float metallic = 0.0f;
            if (AI_SUCCESS == aiGetMaterialColor(mat, AI_MATKEY_COLOR_SPECULAR, &color)) {
                // In legacy formats, highly reflective grey/colored specular usually implies metal.
                float specAverage = (color.r + color.g + color.b) / 3.0f;
                if (specAverage > 0.8f) {
                    metallic = 1.0f; 
                }
            }
            material->SetFloat("pbr.metallicMultiplier", metallic);


            material->SetBool("pbr.useAlbedoMap", false);
            material->SetBool("pbr.useNormalMap", false);
            material->SetBool("pbr.useMetallicMap", false);
            material->SetBool("pbr.useRoughnessMap", false);
            material->SetBool("pbr.useAOMap", false);

            // --- 6. Textures ---
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
                    // if (std::filesystem::exists(ktxExtensionPath))       finalLoadPath = ktxExtensionPath;
                    // else if (std::filesystem::exists(ktxExtensionPath2)) finalLoadPath = ktxExtensionPath2;

                    // Load the texture asset. 
                    UUID texHandle = AssetManager::ImportAsset(finalLoadPath);
                    Shared<Texture2D> texture = AssetManager::GetAsset<Texture2D>(texHandle);

                    TextureType texType = AssimpToTextureType(type);
                    
                    // Route the texture to the appropriate string key expected by the shader
                    if (texType == TextureType::Diffuse) {
                        material->SetTexture("pbr.albedoMap", texture);
                        material->SetBool("pbr.useAlbedoMap", true);
                    }
                    if (texType == TextureType::Normal) {
                        material->SetTexture("pbr.normalMap", texture);
                        material->SetBool("pbr.useNormalMap", true);
                    }
                    if (texType == TextureType::Metallic) {
                        material->SetTexture("pbr.metallicMap", texture);
                        material->SetBool("pbr.useMetallicMap", true);
                    }
                    if (texType == TextureType::Roughness) {
                        material->SetTexture("pbr.roughnessMap", texture);
                        material->SetBool("pbr.useRoughnessMap", true);
                    }
                    if (texType == TextureType::AO) {
                        material->SetTexture("pbr.aoMap", texture);
                        material->SetBool("pbr.useAOMap", true);
                    }
                }
            }

            material->TargetShader = ShaderLibrary::Get("Texture");

            return material;
        }
    };

};
