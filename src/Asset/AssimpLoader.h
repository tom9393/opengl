#pragma once

#include <assimp/Importer.hpp>
#include <assimp/material.h>
#include <assimp/scene.h>
#include <assimp/light.h>
#include <assimp/camera.h>
#include <assimp/postprocess.h>

#include "pch.h"
#include "Asset.h"
#include "ParsedScene.h"
#include "MaterialSpecification.h"
#include "TextureImporter.h"

namespace Engine
{

    //static constexpr unsigned int s_AssimpFlags = aiProcess_Triangulate | aiProcess_SortByPType | aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace | aiProcess_JoinIdenticalVertices | aiProcess_OptimizeGraph | aiProcess_OptimizeMeshes | aiProcess_ImproveCacheLocality;
    static constexpr unsigned int s_AssimpFlags = aiProcess_Triangulate | aiProcess_SortByPType | aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace | aiProcess_JoinIdenticalVertices | aiProcess_ImproveCacheLocality;

    static MaterialType AssimpToTextureType(aiTextureType type)
    {
        switch (type)
        {
            // Albedo / Diffuse
            case aiTextureType_DIFFUSE:
            case aiTextureType_BASE_COLOR:
                return MaterialType::Albedo;

            // Specular
            case aiTextureType_SPECULAR:
                return MaterialType::Specular;

            // Normals / Height (OBJ files often use Height for Normals)
            case aiTextureType_NORMALS:
            case aiTextureType_HEIGHT:
            case aiTextureType_NORMAL_CAMERA:
                return MaterialType::Normal;

            // Metallic / Shininess
            case aiTextureType_METALNESS:
            case aiTextureType_SHININESS:
                return MaterialType::Metallic;

            // Roughness
            case aiTextureType_DIFFUSE_ROUGHNESS:
            case aiTextureType_MAYA_SPECULAR_ROUGHNESS:
                return MaterialType::Roughness;

            // Ambient Occlusion / Lightmaps
            case aiTextureType_AMBIENT_OCCLUSION:
            case aiTextureType_LIGHTMAP:
            case aiTextureType_AMBIENT:
                return MaterialType::AO;

            // Emissive
            case aiTextureType_EMISSIVE:
            case aiTextureType_EMISSION_COLOR:
                return MaterialType::Emissive;

            // Transparency / Opacity
            case aiTextureType_OPACITY:
                return MaterialType::Transparency;

            default:
                return MaterialType::None;
        }
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

    class AssimpLoader
    {
        // Processes a scene into ParsedScene which will be used to instantiate the scene in our engine
        // Node transforms and names are in tree-formed nodes, materials and meshes global and indexed
        public:
        static std::optional<ParsedScene> LoadScene(const AssetMetadata& metadata)
        {
            Assimp::Importer importer;
            const aiScene* scene = importer.ReadFile(metadata.FilePath, s_AssimpFlags);

            if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
                std::cerr << "Assimp failed to load file: " << importer.GetErrorString() << std::endl;
                return std::nullopt;
            }

            ParsedScene out;
            out.Name = metadata.Name;
            out.Directory = std::filesystem::path(metadata.FilePath).parent_path().string();

            // Parse materials once up front; meshes reference them by global index
            out.Materials.reserve(scene->mNumMaterials);
            for (uint32_t i = 0; i < scene->mNumMaterials; i++)
                out.Materials.push_back(ParseMaterialData(scene->mMaterials[i], scene));

            out.RootNode = ParseNodeData(scene->mRootNode, scene);

            return out;
        }

    private:
        // Mesh/material UUIDs are hashed to ensure they are always the same
        // FNV-1a; chains so callers can fold several buffers into one stable key
        static uint64_t HashBytes(uint64_t hash, const void* data, size_t size) {
            const uint8_t* bytes = static_cast<const uint8_t*>(data);
            for (size_t i = 0; i < size; i++) {
                hash ^= bytes[i];
                hash *= 1099511628211ULL;
            }
            return hash;
        }

        static constexpr uint64_t s_FNVOffsetBasis = 14695981039346656037ULL;

        static ParsedSceneNode ParseNodeData(aiNode* node, const aiScene* scene) {
            ParsedSceneNode parsedNode;
            parsedNode.Name = node->mName.C_Str();
            if (parsedNode.Name.empty()) parsedNode.Name = "Unnamed_Node";

            aiVector3D aiScale, aiPos;
            aiQuaternion aiRot;
            node->mTransformation.Decompose(aiScale, aiRot, aiPos);

            parsedNode.Translation = { aiPos.x, aiPos.y, aiPos.z };
            parsedNode.Rotation = glm::eulerAngles(glm::quat(aiRot.w, aiRot.x, aiRot.y, aiRot.z));
            parsedNode.Scale = { aiScale.x, aiScale.y, aiScale.z };

            for (uint32_t i = 0; i < node->mNumMeshes; i++) {
                aiMesh* sourceMesh = scene->mMeshes[node->mMeshes[i]];
                parsedNode.Meshes.push_back(ParseMeshData(sourceMesh)); // ParseMeshData assigns material index
            }

            for (uint32_t i = 0; i < scene->mNumLights; i++) {
                if (parsedNode.Name != scene->mLights[i]->mName.C_Str()) continue; // All cameras/lights are global so we must do this to assign them to a specific node
                ParsedLight parsed;
                if (ParseLightData(scene->mLights[i], parsed))
                    parsedNode.Light = std::move(parsed);
                break;
            }
            for (uint32_t i = 0; i < scene->mNumCameras; i++) {
                if (parsedNode.Name != scene->mCameras[i]->mName.C_Str()) continue; // Same as above
                parsedNode.Camera = ParseCameraData(scene->mCameras[i]);
                break;
            }

            for (uint32_t i = 0; i < node->mNumChildren; i++) {
                parsedNode.Children.push_back(ParseNodeData(node->mChildren[i], scene));
            }

            return parsedNode;
        }

        static ParsedMesh ParseMeshData(aiMesh* aiMesh) {
            ParsedMesh parsedMesh;
            parsedMesh.Name = aiMesh->mName.C_Str();
            if (parsedMesh.Name.empty()) parsedMesh.Name = "Unnamed_Mesh";
            parsedMesh.MaterialIndex = aiMesh->mMaterialIndex;

            glm::vec3 minExtents(std::numeric_limits<float>::max()); // Assimp does provide min/max AABB out of the box but this is per parent,
            glm::vec3 maxExtents(std::numeric_limits<float>::lowest()); // hence if we want per entity AABBs we need to calculate them manually

            bool hasPositions = aiMesh->HasPositions();
            bool hasNormals = aiMesh->HasNormals();
            bool hasUVs = aiMesh->mTextureCoords[0] != nullptr;
            bool hasTangents = aiMesh->HasTangentsAndBitangents();

            std::vector<BufferElement> elements;
            if (hasPositions) elements.emplace_back(ShaderDataType::Float3, "a_Position");
            if (hasNormals)   elements.emplace_back(ShaderDataType::Float3, "a_Normal");
            if (hasUVs)       elements.emplace_back(ShaderDataType::Float2, "a_TexCoord");
            if (hasTangents) {
                elements.emplace_back(ShaderDataType::Float3, "a_Tangent");
                elements.emplace_back(ShaderDataType::Float3, "a_Bitangent");
            }

            parsedMesh.Layout = BufferLayout(elements);

            size_t floatsPerVertex = parsedMesh.Layout.GetStride() / sizeof(float);
            parsedMesh.Vertices.reserve(aiMesh->mNumVertices * floatsPerVertex);

            uint64_t hash = HashBytes(s_FNVOffsetBasis, parsedMesh.Name.data(), parsedMesh.Name.size());

            for (unsigned int i = 0; i < aiMesh->mNumVertices; i++) {
                if (hasPositions) {
                    glm::vec3 pos(aiMesh->mVertices[i].x, aiMesh->mVertices[i].y, aiMesh->mVertices[i].z);
                    minExtents = glm::min(minExtents, pos);
                    maxExtents = glm::max(maxExtents, pos);
                    parsedMesh.Vertices.push_back(pos.x); parsedMesh.Vertices.push_back(pos.y); parsedMesh.Vertices.push_back(pos.z);
                    hash = HashBytes(hash, &pos, sizeof(pos));
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
            parsedMesh.Hash = hash;

            return parsedMesh;
        }

        static ParsedMaterial ParseMaterialData(aiMaterial* mat, const aiScene* scene) {
            ParsedMaterial parsedMat;
            aiString aiMatName;
            mat->Get(AI_MATKEY_NAME, aiMatName);
            parsedMat.Name = aiMatName.length > 0 ? aiMatName.C_Str() : "Unnamed_Material";

            parsedMat.AlbedoTint = GetAlbedoTint(mat);
            parsedMat.Emission = GetEmission(mat);
            parsedMat.Roughness = GetRoughness(mat);
            parsedMat.Metallic = GetMetallic(mat);

            std::array<aiTextureType, 5> typesToCheck = { aiTextureType_DIFFUSE, aiTextureType_NORMALS, aiTextureType_METALNESS, aiTextureType_DIFFUSE_ROUGHNESS, aiTextureType_AMBIENT_OCCLUSION };

            uint64_t hash = HashBytes(s_FNVOffsetBasis, parsedMat.Name.data(), parsedMat.Name.size());

            for (auto type : typesToCheck) {
                if (mat->GetTextureCount(type) < 1) continue;
                aiString str;
                mat->GetTexture(type, 0, &str);
                MaterialType matType = AssimpToTextureType(type);
                if (matType != MaterialType::None) {
                    parsedMat.TexturePaths[matType] = std::string(str.C_Str());
                    // Embedded textures will be saved in scene, non-embedded ones are referenced via their path
                    if (const aiTexture* embeddedTex = scene->GetEmbeddedTexture(str.C_Str())) {
                        size_t byteCount = embeddedTex->mHeight == 0 ? embeddedTex->mWidth : (size_t)embeddedTex->mWidth * embeddedTex->mHeight * sizeof(aiTexel);
                        hash = HashBytes(hash, embeddedTex->pcData, byteCount);
                        if (embeddedTex->mHeight == 0) {
                            parsedMat.EmbeddedTextures[matType] = TextureImporter::DecodeTextureFromMemory(embeddedTex->pcData, embeddedTex->mWidth);
                        } else {
                            parsedMat.EmbeddedTextures[matType] = TextureImporter::DecodeTextureUncompressed(embeddedTex->pcData, embeddedTex->mWidth, embeddedTex->mHeight);
                        }
                    } else {
                        parsedMat.TexturePaths[matType] = std::string(str.C_Str());
                        hash = HashBytes(hash, str.C_Str(), str.length);
                    }
                }
            }
            parsedMat.Hash = hash;
            return parsedMat;
        }

        static bool ParseLightData(const aiLight* light, ParsedLight& out)
        {
            switch (light->mType)
            {
                case aiLightSource_DIRECTIONAL: out.Type = LightComponent::LightType::Directional; break;
                case aiLightSource_POINT:       out.Type = LightComponent::LightType::Point;       break;
                case aiLightSource_SPOT:        out.Type = LightComponent::LightType::Cone;         break;
                default: // Currently no support for area/ambient light
                    return false;
            }

            out.Name = light->mName.C_Str();

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

            // Fall back case
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
                emission = (color.r + color.g + color.b) / 3.0f;
            }
            return emission;
        }

        static float GetRoughness(aiMaterial* mat)
        {
            float roughness = 0.5f;
            float shininess = 0.0f;
            if (AI_SUCCESS == aiGetMaterialFloat(mat, AI_MATKEY_SHININESS, &shininess)) {
                // Prevent division by zero.
                roughness = glm::sqrt(2.0f / (shininess + 2.0f));
            }
            return roughness;
        }

        static float GetMetallic(aiMaterial* mat)
        {
            aiColor4D color;
            float metallic = 0.0f;
            if (AI_SUCCESS == aiGetMaterialColor(mat, AI_MATKEY_COLOR_SPECULAR, &color)) {
                float specAverage = (color.r + color.g + color.b) / 3.0f;
                if (specAverage > 0.8f) {
                    metallic = 1.0f;
                }
            }
            return metallic;
        }
    };

};
