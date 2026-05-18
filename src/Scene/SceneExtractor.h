#pragma once

#include "Entity.h"
#include "LightExtractor.h"
#include "MeshExtractor.h"
#include "Render/RenderView.h"
#include "Render/Camera.h"
#include "Scene.h"

namespace Engine {

    class SceneExtractor {
    public:
    static RenderView Extract(Scene* scene, RenderContext &ctx) {
        RenderView view;
        Frustum frustum = CreateFrustumFromMatrix(ctx.ViewProjection);

        UpdateWorldTransforms(scene);

        // 1. Extract Lights
        LightExtractor::Extract(scene, frustum, view);

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
        MeshExtractor::Extract(scene, frustum, view);

        // 3. Sort
        SortDrawCommands(view);

        return view;
    }

    private:

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

	static void UpdateWorldTransforms(Scene* scene)
	{
		auto view = scene->GetRegistry().view<TransformComponent>();
		for (auto e : view)
		{
			Entity entity{ e, scene };
			
			// We only want to start the cascade from ROOT entities. 
			// Children will be updated automatically by their parents.
			if (!entity.HasParent())
			{
                TransformComponent& comp = entity.GetComponent<TransformComponent>();
				UpdateTransformTree(e, scene, glm::mat4(1.0f), comp.IsDirty);
			}
		}
	}

	static void UpdateTransformTree(entt::entity handle, Scene* scene, const glm::mat4 &parentTransform, bool isParentDirty = false)
	{
		Entity entity{ handle, scene };
		if (!entity.HasComponent<TransformComponent>()) return;

		auto& transformComp = entity.GetComponent<TransformComponent>();
        bool isDirty = false;

        if (transformComp.IsDirty || isParentDirty)
        {
            transformComp.UpdateWorldTransform(parentTransform);
            isDirty = true;
        }

		if (entity.HasComponent<RelationshipComponent>())
		{
			auto& relComp = entity.GetComponent<RelationshipComponent>();
			for (auto childHandle : relComp.Children)
			{
				UpdateTransformTree(childHandle, scene, transformComp.WorldTransform, isDirty);
			}
		}
	}
};

}