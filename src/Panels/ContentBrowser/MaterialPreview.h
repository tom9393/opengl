#pragma once

#include "../pch.h"
#include "Preview.h"
#include "Asset/Prefab.h"
#include "Scene/Entity.h"
#include "Render/Renderer.h"
#include "../Scene/SceneExtractor.h"
#include "../Asset/PrimitiveBuilder.h"
#include "../Render/Passes/ShadowPass2D.h"
#include "../Render/Passes/ShadowPass3D.h"
#include "../Render/Passes/GeometryPass.h"
#include "../Render/Passes/ScenePass.h"

namespace Engine {

    class MaterialPreview : public Preview
    {
        public:
        MaterialPreview()
        {
            m_InstancePool = MakeShared<DynamicBufferPool>(1024 * 1024); // 1MB
            m_RenderGraph.AddPass<ShadowPass2D>("ShadowPass2D");
            m_RenderGraph.AddPass<ShadowPass3D>("PointShadowPass");
            m_RenderGraph.AddPass<GeometryPass>("GeometryPass");
            m_RenderGraph.AddPass<ScenePass>("ScenePass");
        }

        uint32_t GetPreview() override
        {
            return m_RenderGraph.GetPassColorAttachmentRendererID("ScenePass", 0);
        }

        uint32_t UpdatePreview(Shared<Asset> &material) override
        {
            if (!material) return 0;

            Shared<Prefab> materialPreviewScene = MakeShared<Prefab>();

            Entity light = materialPreviewScene->CreateEntity("MaterialPreviewLight");
            LightComponent& lightComponent = light.AddComponent<LightComponent>();
            lightComponent.Type = LightComponent::LightType::Directional;
            lightComponent.Intensity = 25.0f;
            light.AddComponent<TransformComponent>(glm::vec3(-2, 3, 10));

            Entity entity = materialPreviewScene->CreateEntity("MaterialPreviewSphere");
            PrimitiveSpecification spec;
            spec.Type = PrimitiveType::Sphere;
            spec.Segments[0] = 25;
            spec.Segments[1] = 25;
            spec.Segments[2] = 25;
            spec.Size = glm::vec3(5);

            AssetMetadata metadata;
            metadata.Handle = UUID();
            metadata.Name = "MaterialPreviewSphere";
            metadata.FilePath = std::string("/home/tom/Coding/c++/opengl/assets/MaterialPreviewSphere");
            metadata.IsMemoryAsset = true;
            metadata.Type = AssetType::Mesh;

            Shared<Asset> primitive = PrimitiveBuilder::CreatePrimitive(spec, "MaterialPreviewSphere");
            UUID meshID = AssetManager::RegisterAsset(metadata, primitive);

            entity.AddComponent<MeshComponent>(meshID);
            entity.AddComponent<TransformComponent>(); 
            entity.AddComponent<MaterialComponent>(material->Handle);

            glm::vec3 position = glm::vec3(2.0f, 2.0f, 5.0f); 
            glm::vec3 target = glm::vec3(0.0f, 0.0f, 0.0f);
            glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

            glm::mat4 viewMatrix = glm::lookAt(position, target, up);

            m_InstancePool->BeginFrame();

            RenderContext ctx;
            ctx.ViewProjection = glm::perspective(glm::radians(60.0f), 1.0f, 0.1f, 1000.0f) * viewMatrix;
            ctx.CameraPosition = position;
            ctx.InstancePool = m_InstancePool.get();

            RenderView view = SceneExtractor::Extract(materialPreviewScene.get(), ctx);

            Renderer::BeginScene();
            m_RenderGraph.Execute(ctx, view);
            Renderer::EndScene();

            m_InstancePool->InsertFrameFence();

            return m_RenderGraph.GetPassColorAttachmentRendererID("ScenePass", 0);
        }

    };

}