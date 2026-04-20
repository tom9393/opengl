#pragma once

#include "../Render/RenderView.h"
#include "Scene.h"
#include "../Render/Camera.h"
#include "../Asset/AssetManager.h"

namespace Engine {

    class SceneExtractor {
    public:
    static RenderView Extract(Scene* scene, RenderContext &ctx) {
        RenderView view;
        Frustum frustum = CreateFrustumFromMatrix(ctx.ViewProjection);

        // 1. Extract Lights
        ExtractLights(scene, frustum, view);

        CameraData cameraData;
        cameraData.CameraPosition = ctx.CameraPosition;
        cameraData.ViewProjection = ctx.ViewProjection;

        // Camera binding 1 UBO
        auto camAlloc = ctx.InstancePool->Allocate(&cameraData, sizeof(CameraData));
        ctx.InstancePool->BindAsUBO(0, camAlloc.Offset, camAlloc.Size);

        // Lights binding 2 UBO
        auto lightAlloc = ctx.InstancePool->Allocate(&view.LightData, sizeof(SceneLightData));
        ctx.InstancePool->BindAsUBO(1, lightAlloc.Offset, lightAlloc.Size);

        // 2. Extract and Cull Meshes
        ExtractMeshes(scene, frustum, view);

        // 3. Sort
        SortDrawCommands(view);

        return view;
    }

    private:

    static void ExtractLights(Scene* scene, Frustum &frustum, RenderView &renderView)
    {
        scene->UpdateWorldTransforms();

        auto view = scene->GetRegistry().view<LightComponent, TransformComponent>();
        for (auto& e : view)
        {
            if (renderView.LightData.LightCount >= 64) break;
            auto [light, transform] = view.get<LightComponent, TransformComponent>(e);

            int idx = renderView.LightData.LightCount;
            int shadowIndex = -1; // Default to no shadow
            float typeFloat = static_cast<float>(light.Type);
            glm::vec3 direction = transform.GetForward();

            if (!light.CastShadows) continue;
        
            switch (light.Type)
            {
                case LightComponent::LightType::Point:
                {
                    renderView.LightData.Lights[idx].PositionType = glm::vec4(transform.translation, typeFloat);
                    renderView.LightData.Lights[idx].ColorInner   = glm::vec4(light.Color * light.Intensity, 0.0f);
                    renderView.LightData.Lights[idx].DirectionOuter = glm::vec4(0.0f);
                    renderView.LightData.Lights[idx].LightSpaceMatrix = glm::mat4(1.0f);
                    
                    if (light.CastShadows) {
                        shadowIndex = renderView.PointShadowCasters.size();
                        renderView.PointShadowCasters.push_back({ transform.translation, {} });
                    }
                    break;
                }
                case LightComponent::LightType::Directional:
                {
                    renderView.LightData.Lights[idx].PositionType = glm::vec4(0.0f, 0.0f, 0.0f, typeFloat);
                    renderView.LightData.Lights[idx].ColorInner   = glm::vec4(light.Color * light.Intensity, 0.0f);
                    renderView.LightData.Lights[idx].DirectionOuter = glm::vec4(direction, 0.0f);

                    float orthoRange = 40.0f; 
                    glm::mat4 lightProjection = glm::ortho(-orthoRange, orthoRange, -orthoRange, orthoRange, 1.0f, 400.0f);
                    glm::vec3 up = glm::abs(direction.y) > 0.999f ? glm::vec3(0.0f, 0.0f, 1.0f) : glm::vec3(0.0f, 1.0f, 0.0f);
                    glm::vec3 shadowCameraPos = transform.translation - (direction * 300.0f);
                    
                    glm::mat4 lightViewMat = glm::lookAt(transform.translation, transform.translation + direction, glm::vec3(0.0f, 1.0f, 0.0f));
                    renderView.LightData.Lights[idx].LightSpaceMatrix = lightProjection * lightViewMat;
                    
                    if (light.CastShadows) {
                        shadowIndex = renderView.DirectionalShadowCasters.size();
                        renderView.DirectionalShadowCasters.push_back({ renderView.LightData.Lights[idx].LightSpaceMatrix, {} });
                    }
                    break;
                }
                case LightComponent::LightType::Cone:
                {
                    renderView.LightData.Lights[idx].PositionType = glm::vec4(transform.translation, typeFloat);
                    renderView.LightData.Lights[idx].ColorInner   = glm::vec4(light.Color * light.Intensity, light.InnerCutOff);
                    renderView.LightData.Lights[idx].DirectionOuter = glm::vec4(direction, light.OuterCutOff);
                    
                    float fov = glm::acos(light.OuterCutOff) * 2.0f;
                    glm::mat4 lightProjection = glm::perspective(fov, 1.0f, 0.1f, 100.0f);
                    glm::mat4 lightViewMat = glm::lookAt(transform.translation, transform.translation + direction, glm::vec3(0.0f, 1.0f, 0.0f));
                    renderView.LightData.Lights[idx].LightSpaceMatrix = lightProjection * lightViewMat;
                    
                    if (light.CastShadows) {
                        shadowIndex = renderView.DirectionalShadowCasters.size();
                        renderView.DirectionalShadowCasters.push_back({ renderView.LightData.Lights[idx].LightSpaceMatrix, {} });
                    }
                    break;
                }
            }
            renderView.LightData.Lights[idx].Params = glm::ivec4(shadowIndex, 0, 0, 0);
            renderView.LightData.LightCount++;
        }
    }

    static void ExtractMeshes(Scene* scene, Frustum &frustum, RenderView &renderView)
    {
        auto view = scene->GetRegistry().view<MeshComponent, TransformComponent, MaterialComponent>();
        for (auto& e : view)
        {
            auto [meshComponent, transformComponent, materialComponent] = view.get<MeshComponent, TransformComponent, MaterialComponent>(e);
            if (meshComponent.MeshHandle == 0) continue;

            Shared<Mesh> mesh = AssetManager::GetAsset<Mesh>(meshComponent.MeshHandle);
            glm::mat4 transform = transformComponent.WorldTransform;

            // Build Command
            DrawCommand cmd;
            cmd.TargetMesh = mesh;
            cmd.Transform = transform;
            cmd.TargetMaterial = AssetManager::GetAsset<Material>(materialComponent.MaterialHandle);
            
            if (IsAABBInFrustum(frustum, mesh->GetAABB(), transform)) {
                renderView.OpaqueItems.emplace_back(cmd);
            }

            for (auto& caster : renderView.DirectionalShadowCasters) {
                Frustum lightFrustum = CreateFrustumFromMatrix(caster.LightSpaceMatrix);
                if (IsAABBInFrustum(lightFrustum, mesh->GetAABB(), transform)) {
                    caster.RenderItems.emplace_back(cmd);
                }
            }

            for (auto& caster : renderView.PointShadowCasters) {
                // Simple distance check (fallback since we don't have a specific Sphere vs AABB function)
                // You can refine this radius check based on your mesh sizes!
                float distanceToLight = glm::distance(caster.Position, glm::vec3(transform[3]));
                if (distanceToLight < 40.0f) {
                    caster.RenderItems.emplace_back(cmd);
                }
            }
        }

    }

    static void SortDrawCommands(RenderView &renderView)
    {
        auto sortFunc = [](const DrawCommand& a, const DrawCommand& b) {
            if (a.TargetMesh.get() != b.TargetMesh.get()) return a.TargetMesh.get() < b.TargetMesh.get();
            return a.TargetMaterial.get() < b.TargetMaterial.get();
        };

        std::sort(renderView.OpaqueItems.begin(), renderView.OpaqueItems.end(), sortFunc);
        for (auto& caster : renderView.DirectionalShadowCasters) std::sort(caster.RenderItems.begin(), caster.RenderItems.end(), sortFunc);
        for (auto& caster : renderView.PointShadowCasters)   std::sort(caster.RenderItems.begin(), caster.RenderItems.end(), sortFunc);
    }
};

}