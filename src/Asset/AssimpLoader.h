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
#include <glm/gtc/quaternion.hpp>

#include "YamlArchive.h"
#include <yaml-cpp/yaml.h>
#include <fstream>

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

    //static constexpr unsigned int s_AssimpFlags = aiProcess_Triangulate | aiProcess_SortByPType | aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace | aiProcess_JoinIdenticalVertices | aiProcess_OptimizeGraph | aiProcess_OptimizeMeshes | aiProcess_ImproveCacheLocality;
    static constexpr unsigned int s_AssimpFlags = aiProcess_Triangulate | aiProcess_SortByPType | aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace | aiProcess_JoinIdenticalVertices | aiProcess_ImproveCacheLocality;

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
        static AssetMetadata ImportModel(const std::filesystem::path& path)
        {
            Assimp::Importer importer;

            // Load the model using the same flags you defined for geometry processing
            const aiScene* scene = importer.ReadFile(path.string(), s_AssimpFlags);

            if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
                std::cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
                return AssetMetadata{};
            }

            YAML::Emitter out;
            out << YAML::BeginMap;
            out << YAML::Key << "Model" << YAML::Value << YAML::BeginMap;
            out << YAML::Key << "Name" << YAML::Value << path.stem().string();
            out << YAML::Key << "SourceFilePath" << YAML::Value << path.string();
            out << YAML::Key << "Handle" << YAML::Value << (uint64_t)UUID();

            out << YAML::Key << "Hierarchy";
            ProcessNode(scene->mRootNode, scene, out);

            out << YAML::EndMap;
            out << YAML::EndMap;

            std::filesystem::path assetPath = path;
            assetPath.replace_extension(".asset");

            std::ofstream fout(assetPath);
            fout << out.c_str();

            std::cout << "Successfully generated hierarchical asset file: " << assetPath.string() << std::endl;

            AssetMetadata metadata;
            metadata.Handle = UUID();
            metadata.FilePath = assetPath.string();
            metadata.Name = path.stem().string();
            metadata.Type = AssetType::Model;

            return metadata;
        }

        static Shared<Scene> LoadSceneFromAsset(const AssetMetadata &metadata)
        {
            YAML::Node data;
            try {
                data = YAML::LoadFile(metadata.FilePath);
            } catch (YAML::ParserException& e) {
                std::cout << "ERROR::YAML:: Failed to load .asset file: " << metadata.FilePath << " (" << e.what() << ")\n";
                return nullptr;
            }

            if (!data["Model"]) {
                std::cout << "ERROR::YAML:: Invalid .asset file format! Root 'Model' node missing.\n";
                return nullptr;
            }

            YAML::Node modelNode = data["Model"];
            std::string modelName = modelNode["Name"].as<std::string>();
            std::string sourceFilePath = modelNode["SourceFilePath"].as<std::string>();

            Shared<Scene> scene = MakeShared<Scene>();
            Entity rootEntity = scene->CreateEntity(modelName);
            
            rootEntity.AddComponent<TransformComponent>(); 

            YAML::Node hierarchyNode = modelNode["Hierarchy"];
            if (hierarchyNode) {
                ProcessYamlNode(hierarchyNode, scene, rootEntity, sourceFilePath);
            }

            std::cout << "Successfully generated hierarchical Scene from: " << metadata.FilePath << std::endl;

            return scene;
        }

        static Shared<Mesh> LoadSubMesh(const AssetMetadata &metadata)
        {
            Assimp::Importer importer;

            const aiScene* scene = importer.ReadFile(metadata.FilePath, s_AssimpFlags);

            if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
                std::cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
                return nullptr;
            }

            if (metadata.Index >= scene->mNumMeshes) {
                std::cout << "ERROR::ASSIMP:: Submesh index out of bounds!" << std::endl;
                return nullptr;
            }

            return ProcessAiMesh(scene->mMeshes[metadata.Index]);
        }

        static Shared<Material> LoadMaterial(const AssetMetadata &metadata)
        {
            Assimp::Importer importer;

            const aiScene* scene = importer.ReadFile(metadata.FilePath, s_AssimpFlags);

            if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
                std::cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
                return nullptr;
            }

            if (metadata.Index >= scene->mNumMaterials) {
                std::cout << "ERROR::ASSIMP:: Material index out of bounds!" << std::endl;
                return nullptr;
            }

            std::filesystem::path path = metadata.FilePath;
            std::string directory = path.parent_path().string();

            return ProcessAiMaterial(scene->mMaterials[metadata.Index], directory, scene);
        }

    private:
        static void ProcessYamlNode(YAML::Node yamlNode, Shared<Scene> scene, Entity parentEntity, const std::string& sourceFilePath)
        {
            std::string nodeName = yamlNode["Node"].as<std::string>();
            
            Entity currentEntity = scene->CreateEntity(nodeName);
            
            currentEntity.SetParent(parentEntity);

            if (auto transformNode = yamlNode["Transform"]) 
            {
                auto& transform = currentEntity.AddComponent<TransformComponent>();
                transform.translation = transformNode["Translation"].as<glm::vec3>();
                transform.rotation = transformNode["Rotation"].as<glm::vec3>();
                transform.scale = transformNode["Scale"].as<glm::vec3>();
            }

            if (auto meshesNode = yamlNode["Meshes"]) 
            {
                for (auto meshNode : meshesNode) 
                {
                    std::string meshName = meshNode["Name"].as<std::string>();
                    
                    Entity meshEntity = scene->CreateEntity(meshName);
                    
                    meshEntity.SetParent(currentEntity);

                    auto& meshComp = meshEntity.AddComponent<MeshComponent>();
                    meshComp.MeshHandle = meshNode["Handle"].as<uint64_t>();

                    if (auto matNode = meshNode["Material"]) {
                        auto& materialComp = meshEntity.AddComponent<MaterialComponent>();
                        materialComp.MaterialHandle = matNode["Handle"].as<uint64_t>();
                    }
                }
            }

            if (auto childrenNode = yamlNode["Children"]) 
            {
                for (auto childNode : childrenNode) 
                {
                    ProcessYamlNode(childNode, scene, currentEntity, sourceFilePath);
                }
            }
        }

        static void ProcessNode(aiNode* node, const aiScene* scene, YAML::Emitter& out)
        {
            out << YAML::BeginMap;
            
            std::string nodeName = node->mName.C_Str();
            if (nodeName.empty()) nodeName = "Unnamed_Node";
            
            out << YAML::Key << "Node" << YAML::Value << nodeName;

            aiVector3D aiScale, aiPos;
            aiQuaternion aiRot;
            node->mTransformation.Decompose(aiScale, aiRot, aiPos);

            glm::vec3 translation(aiPos.x, aiPos.y, aiPos.z);
            glm::vec3 scale(aiScale.x, aiScale.y, aiScale.z);
            glm::quat q(aiRot.w, aiRot.x, aiRot.y, aiRot.z);
            glm::vec3 euler = glm::eulerAngles(q);
            glm::vec3 rotation(euler.x, euler.y, euler.z);

            out << YAML::Key << "Transform" << YAML::Value << YAML::BeginMap;
            out << YAML::Key << "Translation" << YAML::Value << translation;
            out << YAML::Key << "Rotation" << YAML::Value << rotation;
            out << YAML::Key << "Scale" << YAML::Value << scale;
            out << YAML::EndMap;

            if (node->mNumMeshes > 0)
            {
                out << YAML::Key << "Meshes" << YAML::Value << YAML::BeginSeq;
                for (uint32_t i = 0; i < node->mNumMeshes; i++)
                {
                    uint32_t meshIndex = node->mMeshes[i];
                    aiMesh* mesh = scene->mMeshes[meshIndex];

                    out << YAML::BeginMap;
                    out << YAML::Key << "Name" << YAML::Value << mesh->mName.C_Str();
                    out << YAML::Key << "Index" << YAML::Value << meshIndex;
                    out << YAML::Key << "Handle" << YAML::Value << (uint64_t)UUID();

                    out << YAML::Key << "VertexCount" << YAML::Value << mesh->mNumVertices;
                    out << YAML::Key << "IndexCount" << YAML::Value << (mesh->mNumFaces * 3);
                    out << YAML::Key << "HasBones" << YAML::Value << mesh->HasBones();

                    glm::vec3 aabbMin(mesh->mAABB.mMin.x, mesh->mAABB.mMin.y, mesh->mAABB.mMin.z);
                    glm::vec3 aabbMax(mesh->mAABB.mMax.x, mesh->mAABB.mMax.y, mesh->mAABB.mMax.z);
                    out << YAML::Key << "AABB" << YAML::Value << YAML::BeginMap;
                    out << YAML::Key << "Min" << YAML::Value << aabbMin;
                    out << YAML::Key << "Max" << YAML::Value << aabbMax;
                    out << YAML::EndMap;

                    uint32_t materialIndex = mesh->mMaterialIndex;
                    out << YAML::Key << "Material" << YAML::Value << YAML::BeginMap;
                    if (materialIndex < scene->mNumMaterials) {
                        aiMaterial* ai_mat = scene->mMaterials[materialIndex];
                        aiString matName;
                        std::string finalMatName = "Material_" + std::to_string(materialIndex);
                        
                        if (ai_mat->Get(AI_MATKEY_NAME, matName) == AI_SUCCESS && matName.length > 0) {
                            finalMatName = matName.C_Str();
                        }
                        
                        out << YAML::Key << "Name" << YAML::Value << finalMatName;
                        out << YAML::Key << "Index" << YAML::Value << materialIndex;
                        out << YAML::Key << "Handle" << YAML::Value << (uint64_t)UUID();
                    }
                    out << YAML::EndMap;
                    
                    out << YAML::EndMap;
                }
                out << YAML::EndSeq;
            }

            if (node->mNumChildren > 0)
            {
                out << YAML::Key << "Children" << YAML::Value << YAML::BeginSeq;
                for (uint32_t i = 0; i < node->mNumChildren; i++)
                {
                    ProcessNode(node->mChildren[i], scene, out);
                }
                out << YAML::EndSeq;
            }

            out << YAML::EndMap;
        }

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

            BufferLayout meshLayout(layoutElements);

            vertices.reserve(aiMesh->mNumVertices * floatsPerVertex);

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

        static Shared<Material> ProcessAiMaterial(aiMaterial* mat, const std::string& directory, const aiScene* scene)
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

            for (auto& type : textureTypes)
            {
                for(unsigned int i = 0; i < mat->GetTextureCount(type); i++)
                {
                    aiString str;
                    mat->GetTexture(type, i, &str);

                    std::string filename = std::string(str.C_Str());
                    std::string fullPath = directory + "/" + filename;
                    
                    size_t lastindex = fullPath.find_last_of("."); 
                    std::string ktxExtensionPath = fullPath.substr(0, lastindex) + ".ktx";
                    std::string ktxExtensionPath2 = fullPath.substr(0, lastindex) + ".ktx2";

                    std::string finalLoadPath = fullPath;
                    // if (std::filesystem::exists(ktxExtensionPath))       finalLoadPath = ktxExtensionPath;
                    // else if (std::filesystem::exists(ktxExtensionPath2)) finalLoadPath = ktxExtensionPath2;

                    Shared<Texture2D> texture;

                    if (const aiTexture* embeddedTexture = scene->GetEmbeddedTexture(str.C_Str())) 
                    {
                        // --- EMBEDDED TEXTURE HANDLING ---
                        
                        if (embeddedTexture->mHeight == 0) 
                        {
                            // The texture is compressed (e.g., a PNG or JPEG embedded in the file).
                            // mWidth contains the size of the buffer in bytes.
                            void* buffer = embeddedTexture->pcData;
                            size_t bufferSize = embeddedTexture->mWidth;
                            
                            // You will need to implement a new constructor/factory in Texture2D
                            // that takes a memory buffer and uses stbi_load_from_memory()
                            texture = Texture2D::Create(buffer, bufferSize); 

                            AssetMetadata metadata;
                            metadata.Handle = UUID();
                            metadata.FilePath = "";
                            metadata.Name = "";
                            metadata.Index = 0;
                            metadata.Type = AssetType::Texture2D;
                            Shared<Asset> tmp = texture;
                            UUID texHandle = AssetManager::RegisterAsset(metadata, tmp);
                        } 
                        else 
                        {
                            // The texture is raw uncompressed pixel data (ARGB8888).
                            // mWidth and mHeight contain the actual dimensions.
                            void* buffer = embeddedTexture->pcData;
                            uint32_t width = embeddedTexture->mWidth;
                            uint32_t height = embeddedTexture->mHeight;
                            
                            // You will need a constructor that creates a texture directly from raw pixels
                            texture = Texture2D::Create(buffer, width, height);
                            AssetMetadata metadata;
                            metadata.Handle = UUID();
                            metadata.FilePath = "";
                            metadata.Name = "";
                            metadata.Index = 0;
                            metadata.Type = AssetType::Texture2D;
                            Shared<Asset> tmp = texture;
                            UUID texHandle = AssetManager::RegisterAsset(metadata, tmp);
                        }
                    } else {
                        const aiTexture* text = scene->GetEmbeddedTexture(str.C_Str());

                        // Load the texture asset. 
                        UUID texHandle = AssetManager::ImportAsset(finalLoadPath);
                        texture = AssetManager::GetAsset<Texture2D>(texHandle);
                    }

                    if (!texture)
                    {
                        std::cout << "Warning: Failed to load texture: " << filename << std::endl;
                        continue;
                    }

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
