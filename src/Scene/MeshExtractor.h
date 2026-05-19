#pragma once

#include "Asset/AssetManager.h"
#include "Scene.h"
#include "Render/RenderView.h"
#include "Render/Camera.h"

namespace Engine {

class MeshExtractor {
public:
    static void Extract(Scene* scene, Frustum& frustum, RenderView& renderView)
    {
        auto view = scene->GetRegistry().view<MeshComponent, TransformComponent, MaterialComponent>();
        for (auto& e : view)
        {
            auto [meshComponent, transformComponent, materialComponent] = view.get<MeshComponent, TransformComponent, MaterialComponent>(e);

            Shared<Mesh> mesh;
            Shared<Material> material;

            AssetManager::GetAssetAsync(meshComponent.MeshHandle, [&](Shared<Asset> asset){
                mesh = std::static_pointer_cast<Mesh>(asset);
            });
            AssetManager::GetAssetAsync(materialComponent.MaterialHandle, [&](Shared<Asset> asset){
                material = std::static_pointer_cast<Material>(asset);
            });

            if (!mesh || !material) continue;

            glm::mat4 transform = transformComponent.WorldTransform;

            DrawCommand cmd;
            cmd.TargetMesh = mesh;
            cmd.Transform = transform;
            cmd.TargetMaterial = material;

            if (IsAABBInFrustum(frustum, mesh->GetAABB(), transform)) {
                renderView.OpaqueItems.emplace_back(cmd);
            }

            for (auto& caster : renderView.DirectionalShadowCasters) {
                Frustum lightFrustum = CreateFrustumFromMatrix(caster.LightSpaceMatrix);
                if (IsAABBInFrustum(lightFrustum, mesh->GetAABB(), transform)) {
                    caster.RenderItems.emplace_back(cmd);
                }
            }

            for (auto& caster : renderView.ConeShadowCasters) {
                Frustum lightFrustum = CreateFrustumFromMatrix(caster.LightSpaceMatrix);
                if (IsAABBInFrustum(lightFrustum, mesh->GetAABB(), transform)) {
                    caster.RenderItems.emplace_back(cmd);
                }
            }

            for (auto& caster : renderView.PointShadowCasters) {
                float distanceToLight = glm::distance(caster.Position, glm::vec3(transform[3]));
                if (distanceToLight < 40.0f) {
                    caster.RenderItems.emplace_back(cmd);
                }
            }
        }
    }
};

}
