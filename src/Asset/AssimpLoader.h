#pragma once

#include <assimp/Importer.hpp>
#include <assimp/material.h>
#include <assimp/scene.h>
#include <assimp/light.h>
#include <assimp/camera.h>
#include <assimp/postprocess.h>

#include "pch.h"
#include "Core/Application.h"
#include "AssetManager.h"
#include "Asset/Prefab.h"
#include "Asset/AssetUploadQueue.h"
#include "Render/BufferLayout.h"
#include "MaterialSpecification.h"
#include "MatdefLoader.h"
#include "Mesh.h"
#include "Render/Renderer.h"
#include "Scene/Entity.h"
#include "Texture.h"
#include "TextureImporter.h"

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

    static MaterialType AssimpToTextureType(aiTextureType type)
    {
        // Albedo / Diffuse
        if (type == aiTextureType_DIFFUSE || type == aiTextureType_BASE_COLOR) 
            return MaterialType::Albedo;
        
        // Specular
        if (type == aiTextureType_SPECULAR) 
            return MaterialType::Specular;

        // Normals / Height (OBJ files often use Height for Normals)
        if (type == aiTextureType_NORMALS || type == aiTextureType_HEIGHT || type == aiTextureType_NORMAL_CAMERA) 
            return MaterialType::Normal;

        // Metallic / Shininess
        if (type == aiTextureType_METALNESS || type == aiTextureType_SHININESS) 
            return MaterialType::Metallic;

        // Roughness
        if (type == aiTextureType_DIFFUSE_ROUGHNESS || type == aiTextureType_MAYA_SPECULAR_ROUGHNESS) 
            return MaterialType::Roughness;

        // Ambient Occlusion / Lightmaps
        if (type == aiTextureType_AMBIENT_OCCLUSION || type == aiTextureType_LIGHTMAP || type == aiTextureType_AMBIENT) 
            return MaterialType::AO;

        // Emissive
        if (type == aiTextureType_EMISSIVE || type == aiTextureType_EMISSION_COLOR) 
            return MaterialType::Emissive;

        // Transparency / Opacity
        if (type == aiTextureType_OPACITY) 
            return MaterialType::Transparency;

        return MaterialType::None;
    }

    static aiTextureType TypeToAssimpType(MaterialType type)
    {
        switch (type)
        {
            case MaterialType::Diffuse:      return aiTextureType_BASE_COLOR;
            case MaterialType::Specular:     return aiTextureType_SPECULAR;
            case MaterialType::Normal:       return aiTextureType_NORMALS;
            case MaterialType::Metallic:     return aiTextureType_METALNESS;
            case MaterialType::Roughness:    return aiTextureType_DIFFUSE_ROUGHNESS;
            case MaterialType::AO:           return aiTextureType_AMBIENT_OCCLUSION;
            case MaterialType::Emissive:     return aiTextureType_EMISSION_COLOR;
            case MaterialType::Transparency: return aiTextureType_OPACITY;
            default:                         return aiTextureType_NONE;
        }
    }

    struct ParsedMesh {
        std::string Name;
        std::vector<float> Vertices;
        std::vector<unsigned int> Indices;
        uint32_t MaterialIndex = 0;
        glm::vec3 MinExtents;
        glm::vec3 MaxExtents;
    };

    struct ParsedMaterial {
        std::string Name;
        glm::vec4 AlbedoTint;
        float Emission;
        float Roughness;
        float Metallic;
        std::unordered_map<MaterialType, std::string> TexturePaths;
        std::unordered_map<MaterialType, TextureLoadData> EmbeddedTextures;
    };

    struct ParsedNode {
        std::string Name;
        glm::vec3 Translation;
        glm::vec3 Rotation;
        glm::vec3 Scale;
        std::vector<uint32_t> MeshIndices;
        std::vector<ParsedNode> Children;
    };

    struct ParsedLight {
        std::string Name;
        LightComponent::LightType Type = LightComponent::LightType::Point;
        glm::vec3 Color = glm::vec3(1.0f);
        float Intensity = 1.0f;
        bool CastShadows = true;
        float InnerCutOff = 0.0f; // cosine of the inner cone half-angle (cone lights only)
        float OuterCutOff = 0.0f; // cosine of the outer cone half-angle (cone lights only)
    };

    struct ParsedCamera {
        std::string Name;
        bool Orthographic = false;
        float VerticalFOV = glm::radians(45.0f);
        float OrthographicSize = 10.0f;
        float NearClip = 0.01f;
        float FarClip = 1000.0f;
        bool Primary = false;
    };

    struct ParsedModelData {
        std::string Name;
        std::string Directory;
        std::vector<ParsedMesh> Meshes;
        std::vector<ParsedMaterial> Materials;
        std::vector<ParsedLight> Lights;
        std::vector<ParsedCamera> Cameras;
        ParsedNode RootNode;
    };

    class AssimpLoader
    {
        public:
        static Shared<Prefab> LoadModel(const AssetMetadata& metadata)
        {
            ParsedModelData modelData;
            if (!ParseScene(metadata, modelData))
                return nullptr;

            return FinalizeModelOnMainThread(metadata, modelData);
        }

        static void LoadModelAsync(const AssetMetadata& metadata, std::function<void(Shared<Asset>)> cb) {
            Application::Get().GetThreadPool().Enqueue([metadata, cb]() {
                ParsedModelData modelData;
                if (!ParseScene(metadata, modelData))
                    return;

                AssetUploadQueue::Submit([metadata, modelData, cb]() {
                    Shared<Prefab> model = FinalizeModelOnMainThread(metadata, modelData);
                    if (cb) cb(model);
                });
            });
        }

    private:
        static bool ParseScene(const AssetMetadata& metadata, ParsedModelData& outData)
        {
            Assimp::Importer importer;
            const aiScene* scene = importer.ReadFile(metadata.FilePath, s_AssimpFlags);

            if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
                std::cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
                return false;
            }

            outData.Name = metadata.Name;
            outData.Directory = std::filesystem::path(metadata.FilePath).parent_path().string();

            // Extract Materials
            for (uint32_t i = 0; i < scene->mNumMaterials; i++) {
                outData.Materials.push_back(ParseMaterialData(scene->mMaterials[i], scene));
            }

            // Extract Meshes
            for (uint32_t i = 0; i < scene->mNumMeshes; i++) {
                outData.Meshes.push_back(ParseMeshData(scene->mMeshes[i]));
            }

            // Extract Lights (the aiScene is freed when the importer goes out of
            // scope, so copy everything we need into POD before returning). Assimp
            // attaches lights/cameras to scene nodes by name; the node's transform
            // already places and orients them, so we only translate intrinsic params.
            for (uint32_t i = 0; i < scene->mNumLights; i++) {
                ParsedLight parsed;
                if (ParseLightData(scene->mLights[i], parsed))
                    outData.Lights.push_back(parsed);
            }

            // Extract Cameras
            for (uint32_t i = 0; i < scene->mNumCameras; i++) {
                outData.Cameras.push_back(ParseCameraData(scene->mCameras[i]));
            }

            // Extract Hierarchy
            outData.RootNode = ParseNodeData(scene->mRootNode);

            return true;
        }

        static ParsedNode ParseNodeData(aiNode* node) {
            ParsedNode parsedNode;
            parsedNode.Name = node->mName.C_Str();
            if (parsedNode.Name.empty()) parsedNode.Name = "Unnamed_Node";

            aiVector3D aiScale, aiPos;
            aiQuaternion aiRot;
            node->mTransformation.Decompose(aiScale, aiRot, aiPos);

            parsedNode.Translation = { aiPos.x, aiPos.y, aiPos.z };
            parsedNode.Rotation = glm::eulerAngles(glm::quat(aiRot.w, aiRot.x, aiRot.y, aiRot.z));
            parsedNode.Scale = { aiScale.x, aiScale.y, aiScale.z };

            for (uint32_t i = 0; i < node->mNumMeshes; i++) {
                parsedNode.MeshIndices.push_back(node->mMeshes[i]);
            }

            for (uint32_t i = 0; i < node->mNumChildren; i++) {
                parsedNode.Children.push_back(ParseNodeData(node->mChildren[i]));
            }
            return parsedNode;
        }

        static ParsedMesh ParseMeshData(aiMesh* aiMesh) {
            ParsedMesh parsedMesh;
            parsedMesh.Name = aiMesh->mName.C_Str();
            if (parsedMesh.Name.empty()) parsedMesh.Name = "Unnamed_Mesh";
            parsedMesh.MaterialIndex = aiMesh->mMaterialIndex;

            glm::vec3 minExtents(std::numeric_limits<float>::max());
            glm::vec3 maxExtents(std::numeric_limits<float>::lowest());

            bool hasPositions = aiMesh->HasPositions();
            bool hasNormals = aiMesh->HasNormals();
            bool hasUVs = aiMesh->mTextureCoords[0] != nullptr;
            bool hasTangents = aiMesh->HasTangentsAndBitangents();

            // Calculate exact floats required
            size_t floatsPerVertex = 0;
            if (hasPositions) floatsPerVertex += 3;
            if (hasNormals) floatsPerVertex += 3;
            if (hasUVs) floatsPerVertex += 2;
            if (hasTangents) floatsPerVertex += 6;

            parsedMesh.Vertices.reserve(aiMesh->mNumVertices * floatsPerVertex);

            for (unsigned int i = 0; i < aiMesh->mNumVertices; i++) {
                if (hasPositions) {
                    glm::vec3 pos(aiMesh->mVertices[i].x, aiMesh->mVertices[i].y, aiMesh->mVertices[i].z);
                    minExtents = glm::min(minExtents, pos);
                    maxExtents = glm::max(maxExtents, pos);
                    parsedMesh.Vertices.push_back(pos.x); parsedMesh.Vertices.push_back(pos.y); parsedMesh.Vertices.push_back(pos.z);
                }
                if (hasNormals) {
                    parsedMesh.Vertices.push_back(aiMesh->mNormals[i].x); parsedMesh.Vertices.push_back(aiMesh->mNormals[i].y); parsedMesh.Vertices.push_back(aiMesh->mNormals[i].z);
                }
                if (hasUVs) {
                    parsedMesh.Vertices.push_back(aiMesh->mTextureCoords[0][i].x); parsedMesh.Vertices.push_back(aiMesh->mTextureCoords[0][i].y);
                }
                if (hasTangents) {
                    parsedMesh.Vertices.push_back(aiMesh->mTangents[i].x); parsedMesh.Vertices.push_back(aiMesh->mTangents[i].y); parsedMesh.Vertices.push_back(aiMesh->mTangents[i].z);
                    parsedMesh.Vertices.push_back(aiMesh->mBitangents[i].x); parsedMesh.Vertices.push_back(aiMesh->mBitangents[i].y); parsedMesh.Vertices.push_back(aiMesh->mBitangents[i].z);
                }
            }

            parsedMesh.Indices.reserve(aiMesh->mNumFaces * 3);
            for (unsigned int i = 0; i < aiMesh->mNumFaces; i++) {
                aiFace face = aiMesh->mFaces[i];
                for (unsigned int j = 0; j < face.mNumIndices; j++) {
                    parsedMesh.Indices.push_back(face.mIndices[j]);
                }
            }

            parsedMesh.MinExtents = minExtents;
            parsedMesh.MaxExtents = maxExtents;

            return parsedMesh;
        }

        static ParsedMaterial ParseMaterialData(aiMaterial* mat, const aiScene* scene) {
            ParsedMaterial parsedMat;
            aiString aiMatName;
            mat->Get(AI_MATKEY_NAME, aiMatName);
            parsedMat.Name = aiMatName.length > 0 ? aiMatName.C_Str() : "Unnamed_Material";

            // Grab properties (Using your existing helpers)
            parsedMat.AlbedoTint = GetAlbedoTint(mat);
            parsedMat.Emission = GetEmission(mat);
            parsedMat.Roughness = GetRoughness(mat);
            parsedMat.Metallic = GetMetallic(mat);

            // Grab Texture Paths
            std::array<aiTextureType, 5> typesToCheck = { aiTextureType_DIFFUSE, aiTextureType_NORMALS, aiTextureType_METALNESS, aiTextureType_DIFFUSE_ROUGHNESS, aiTextureType_AMBIENT_OCCLUSION };
            
            for (auto type : typesToCheck) {
                if (mat->GetTextureCount(type) > 0) {
                    aiString str;
                    mat->GetTexture(type, 0, &str);
                    MaterialType matType = AssimpToTextureType(type); // Call your helper
                    if (matType != MaterialType::None) {
                        parsedMat.TexturePaths[matType] = std::string(str.C_Str());
                        // --- NEW EMBEDDED TEXTURE CHECK ---
                        if (const aiTexture* embeddedTex = scene->GetEmbeddedTexture(str.C_Str())) {
                            if (embeddedTex->mHeight == 0) {
                                parsedMat.EmbeddedTextures[matType] = TextureImporter::DecodeTextureFromMemory(embeddedTex->pcData, embeddedTex->mWidth);
                            } else {
                                parsedMat.EmbeddedTextures[matType] = TextureImporter::DecodeTextureUncompressed(embeddedTex->pcData, embeddedTex->mWidth, embeddedTex->mHeight);
                            }
                        } else {
                            // It's external, just save the path as usual.
                            parsedMat.TexturePaths[matType] = std::string(str.C_Str());
                        }
                    }
                }
            }
            return parsedMat;
        }

        static Shared<Prefab> FinalizeModelOnMainThread(const AssetMetadata& metadata, const ParsedModelData& data) {
            Shared<Prefab> prefab = MakeShared<Prefab>();

            // 1. Generate GPU Materials
            std::vector<UUID> materialHandles(data.Materials.size());
            for (size_t i = 0; i < data.Materials.size(); i++) {
                const auto& matData = data.Materials[i];
                Shared<Material> material = MatdefLoader::LoadMaterial("/home/tom/Coding/c++/opengl/shaders/PBR_Standard.matdef");

                material->Add({MaterialType::AlbedoTint, matData.AlbedoTint});
                material->Add({MaterialType::Emission, matData.Emission});
                material->Add({MaterialType::RoughnessMultiplier, matData.Roughness});
                material->Add({MaterialType::MetallicMultiplier, matData.Metallic});


                // Load Textures (Note: In a fully async system, you would trigger LoadTextureAsync here)
                for (const auto& [type, path] : matData.TexturePaths) {
                    std::string texturePath = data.Directory + "/" + path;
                    if (!std::filesystem::exists(texturePath)) continue;
                    UUID texHandle = AssetManager::ImportAsset(texturePath);
                    AssetMetadata metadata = AssetManager::GetMetadata(texHandle);

                    TextureImporter::LoadTexture2DAsync(metadata, [material, type, texHandle](Shared<Asset> asset) {
                        Shared<Texture2D> img = std::static_pointer_cast<Texture2D>(asset);
                        material->Add({type, img}); 
                    });
                }

                for (auto& [type, loadData] : matData.EmbeddedTextures) {
                    // RIGHT HERE! We use your existing method now that we are safe on the Main Thread

                    Shared<Texture2D> texture = Texture2D::Create(loadData);
                    
                    // Clean up the CPU RAM!
                    TextureImporter::FreeTextureData(loadData);
                    
                    AssetMetadata metadata;
                    metadata.Handle = UUID();
                    metadata.Name = "EmbeddedTexture";
                    metadata.FilePath = "Temp";
                    metadata.Type = AssetType::Texture2D;
                    metadata.IsMemoryAsset = true;

                    Shared<Asset> texCast = texture;
                    UUID texHandle = AssetManager::RegisterAsset(metadata, texCast);
                    material->Add({type, texture}); 
                }

                AssetMetadata matMeta;
                matMeta.Handle = UUID();
                matMeta.Name = data.Name + "_" + matData.Name;
                matMeta.Type = AssetType::Material;
                matMeta.IsMemoryAsset = true;
                Shared<Asset> assetCast = material;
                materialHandles[i] = AssetManager::RegisterAsset(matMeta, assetCast);
            }

            // 2. Generate GPU Meshes
            std::vector<UUID> meshHandles(data.Meshes.size());
            for (size_t i = 0; i < data.Meshes.size(); i++) {
                const auto& meshData = data.Meshes[i];
                Shared<Mesh> mesh = MakeShared<Mesh>();
                mesh->GetVertices() = meshData.Vertices; 
                mesh->GetIndices() = meshData.Indices;

                // --- OPENGL CALLS HAPPEN HERE ---
                Shared<VertexArray> VAO = VertexArray::Create();
                Shared<VertexBuffer> VBO = VertexBuffer::Create((float*)meshData.Vertices.data(), meshData.Vertices.size() * sizeof(float));
                
                // Reconstruct Layout dynamically (Simplified layout assumption based on your original code)
                VBO->SetLayout({
                    { ShaderDataType::Float3, "a_Position" },
                    { ShaderDataType::Float3, "a_Normal" },
                    { ShaderDataType::Float2, "a_TexCoord" },
                    { ShaderDataType::Float3, "a_Tangent" },
                    { ShaderDataType::Float3, "a_Bitangent" }
                });
                
                VAO->AddVertexBuffer(VBO);
                Shared<IndexBuffer> EBO = IndexBuffer::Create((unsigned int*)meshData.Indices.data(), meshData.Indices.size());
                VAO->SetIndexBuffer(EBO);

                mesh->SetVAO(VAO);
                mesh->SetVBOs({VBO});
                mesh->SetEBO(EBO);
                mesh->SetAABB({ meshData.MinExtents, meshData.MaxExtents });

                AssetMetadata meshMeta;
                meshMeta.Handle = UUID();
                meshMeta.Name = data.Name + "_" + meshData.Name;
                meshMeta.Type = AssetType::Mesh;
                meshMeta.IsMemoryAsset = true;
                Shared<Asset> assetCast = mesh;
                meshHandles[i] = AssetManager::RegisterAsset(meshMeta, assetCast);
            }

            // 3. Build ECS Hierarchy.
            // Lights and cameras are matched to nodes by name; precompute lookups so
            // traversal is O(nodes) instead of O(nodes * (lights + cameras)).
            std::unordered_map<std::string, const ParsedLight*> lightsByName = BuildNameLookup(data.Lights);
            std::unordered_map<std::string, const ParsedCamera*> camerasByName = BuildNameLookup(data.Cameras);

            // The Assimp root node becomes the prefab root directly (named after the
            // asset) rather than nesting under a redundant synthetic root entity.
            Entity nullParent{};
            BuildEntityHierarchy(data.RootNode, prefab, nullParent, meshHandles, materialHandles, data, lightsByName, camerasByName, data.Name);

            // 4. Mark Complete!
            Shared<Asset> prefabCast = prefab;
            AssetManager::RegisterAsset(metadata, prefabCast);
            std::cout << "Successfully loaded model: " << metadata.FilePath << std::endl;
            return prefab;
        }

        static void BuildEntityHierarchy(const ParsedNode& nodeData, Shared<Prefab>& prefab, Entity parentEntity, const std::vector<UUID>& meshHandles, const std::vector<UUID>& materialHandles, const ParsedModelData& fullData, const std::unordered_map<std::string, const ParsedLight*>& lightsByName, const std::unordered_map<std::string, const ParsedCamera*>& camerasByName, const std::string& nameOverride = "") {
            Entity currentEntity = prefab->CreateEntity(nameOverride.empty() ? nodeData.Name : nameOverride);
            currentEntity.SetParent(parentEntity);

            auto& transform = currentEntity.AddComponent<TransformComponent>();
            transform.translation = nodeData.Translation;
            transform.rotation = nodeData.Rotation;
            transform.scale = nodeData.Scale;

            // Attach a light or camera if this node carries one
            if (const ParsedLight* light = FindByName(lightsByName, nodeData.Name))
                AttachLight(currentEntity, *light);
            if (const ParsedCamera* camera = FindByName(camerasByName, nodeData.Name))
                AttachCamera(currentEntity, *camera);

            for (uint32_t meshIndex : nodeData.MeshIndices) {
                Entity meshEntity = (nodeData.MeshIndices.size() == 1) ? currentEntity : prefab->CreateEntity(nodeData.Name + "_MeshPart");
                if (nodeData.MeshIndices.size() > 1) {
                    meshEntity.SetParent(currentEntity);
                    meshEntity.AddComponent<TransformComponent>();
                }

                meshEntity.AddComponent<MeshComponent>(meshHandles[meshIndex]);

                // Map the material index from the ParsedMesh
                uint32_t matIndex = fullData.Meshes[meshIndex].MaterialIndex;
                if (matIndex < materialHandles.size()) {
                    meshEntity.AddComponent<MaterialComponent>(materialHandles[matIndex]);
                }
            }

            for (const auto& childNode : nodeData.Children) {
                BuildEntityHierarchy(childNode, prefab, currentEntity, meshHandles, materialHandles, fullData, lightsByName, camerasByName);
            }
        }

        // --- Lights & Cameras ---

        // Lights and cameras are matched to the scene node whose name they carry.
        // Precompute a name -> item lookup so attachment is O(1) per node; on
        // duplicate node names the first item wins.
        template<typename T>
        static std::unordered_map<std::string, const T*> BuildNameLookup(const std::vector<T>& items)
        {
            std::unordered_map<std::string, const T*> lookup;
            lookup.reserve(items.size());
            for (const T& item : items)
                lookup.emplace(item.Name, &item);
            return lookup;
        }

        template<typename T>
        static const T* FindByName(const std::unordered_map<std::string, const T*>& lookup, const std::string& name)
        {
            auto it = lookup.find(name);
            return it != lookup.end() ? it->second : nullptr;
        }

        static bool ParseLightData(const aiLight* light, ParsedLight& out)
        {
            switch (light->mType)
            {
                case aiLightSource_DIRECTIONAL: out.Type = LightComponent::LightType::Directional; break;
                case aiLightSource_POINT:       out.Type = LightComponent::LightType::Point;       break;
                case aiLightSource_SPOT:        out.Type = LightComponent::LightType::Cone;         break;
                default: // Ambient / area lights have no engine equivalent
                    std::cout << "Warning: Skipping unsupported light type for node: " << light->mName.C_Str() << std::endl;
                    return false;
            }

            out.Name = light->mName.C_Str();

            // Assimp folds intensity into the diffuse color. Split it back into a normalized
            // color + scalar intensity to match the engine's representation.
            glm::vec3 color = { light->mColorDiffuse.r, light->mColorDiffuse.g, light->mColorDiffuse.b };
            float intensity = glm::max(color.r, glm::max(color.g, color.b));

            out.Color = intensity > 0.0f ? color / intensity : glm::vec3(1.0f);
            out.Intensity = intensity;
            out.CastShadows = true;

            // Cone angles are measured from the light axis to the cone edge (half-angles);
            // the engine stores and compares against their cosine.
            out.InnerCutOff = glm::cos(light->mAngleInnerCone);
            out.OuterCutOff = glm::cos(light->mAngleOuterCone);

            return true;
        }

        static ParsedCamera ParseCameraData(const aiCamera* camera)
        {
            ParsedCamera out;
            out.Name = camera->mName.C_Str();
            out.NearClip = camera->mClipPlaneNear;
            out.FarClip = camera->mClipPlaneFar;

            // Fall back to a 16:9 aspect when the file omits one (mAspect == 0).
            float aspect = camera->mAspect > 0.0f ? camera->mAspect : (16.0f / 9.0f);

            if (camera->mOrthographicWidth > 0.0f)
            {
                out.Orthographic = true;
                // mOrthographicWidth is a half-width; the engine sizes ortho cameras by full height.
                out.OrthographicSize = (camera->mOrthographicWidth * 2.0f) / aspect;
            }
            else
            {
                // mHorizontalFOV is the half horizontal angle; convert to the full vertical FOV.
                out.VerticalFOV = 2.0f * glm::atan(glm::tan(camera->mHorizontalFOV) / aspect);
            }

            return out;
        }

        static void AttachLight(Entity entity, const ParsedLight& data)
        {
            auto& light = entity.AddComponent<LightComponent>();
            light.Type = data.Type;
            light.Color = data.Color;
            light.Intensity = data.Intensity;
            light.CastShadows = data.CastShadows;
            light.InnerCutOff = data.InnerCutOff;
            light.OuterCutOff = data.OuterCutOff;
        }

        static void AttachCamera(Entity entity, const ParsedCamera& data)
        {
            Shared<SceneCamera> camera = MakeShared<SceneCamera>();
            if (data.Orthographic)
                camera->SetOrthographic(data.OrthographicSize, data.NearClip, data.FarClip);
            else
                camera->SetPerspective(data.VerticalFOV, data.NearClip, data.FarClip);

            auto& cameraComponent = entity.AddComponent<SceneCameraComponent>(camera);
            cameraComponent.primary = data.Primary;
        }

        static glm::vec4 GetAlbedoTint(aiMaterial* mat)
        {
            glm::vec4 albedoTint = glm::vec4(1.0f);
            aiColor4D color;
            if (AI_SUCCESS == aiGetMaterialColor(mat, AI_MATKEY_COLOR_DIFFUSE, &color)) {
                albedoTint.r = color.r; albedoTint.g = color.g; albedoTint.b = color.b;
            }

            float opacity = 1.0f;
            if (AI_SUCCESS == aiGetMaterialFloat(mat, AI_MATKEY_OPACITY, &opacity)) {
                albedoTint.a = opacity;
            }
            return albedoTint;
        }

        static float GetEmission(aiMaterial* mat)
        {
            aiColor4D color;
            float emission = 0.0f;
            if (AI_SUCCESS == aiGetMaterialColor(mat, AI_MATKEY_COLOR_EMISSIVE, &color)) {
                // Average the RGB values to get a scalar emission intensity
                emission = (color.r + color.g + color.b) / 3.0f;
            }
            return emission;
        }

        static float GetRoughness(aiMaterial* mat)
        {
            float roughness = 0.5f; // Safe default
            float shininess = 0.0f;
            if (AI_SUCCESS == aiGetMaterialFloat(mat, AI_MATKEY_SHININESS, &shininess)) {
                // Prevent division by zero. 
                // Math: High shininess (e.g., 714) = very smooth = low roughness.
                roughness = glm::sqrt(2.0f / (shininess + 2.0f));
            }
            return roughness;
        }

        static float GetMetallic(aiMaterial* mat)
        {
            aiColor4D color;
            float metallic = 0.0f;
            if (AI_SUCCESS == aiGetMaterialColor(mat, AI_MATKEY_COLOR_SPECULAR, &color)) {
                // In legacy formats, highly reflective grey/colored specular usually implies metal.
                float specAverage = (color.r + color.g + color.b) / 3.0f;
                if (specAverage > 0.8f) {
                    metallic = 1.0f; 
                }
            }
            return metallic;
        }
    };

};
