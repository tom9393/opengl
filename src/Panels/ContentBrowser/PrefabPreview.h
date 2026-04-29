#pragma once

#include "../pch.h"
#include "Preview.h"
#include "Asset/Prefab.h"
#include "Scene/Entity.h"
#include "Render/Renderer.h"
#include "../Scene/SceneExtractor.h"
#include "../Render/Passes/ShadowPass2D.h"
#include "../Render/Passes/ShadowPass3D.h"
#include "../Render/Passes/GeometryPass.h"
#include "../Render/Passes/ScenePass.h"

namespace Engine {

    class PrefabPreview : public Preview
    {
        public:
        PrefabPreview()
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

        uint32_t UpdatePreview(Shared<Asset> &asset) override
        {
            if (!asset) return 0;

            auto prefab = std::static_pointer_cast<Prefab>(asset);

            Shared<Scene> copied = Scene::Copy(prefab);

            Entity light = copied->CreateEntity("ModelPreviewLight");
            LightComponent& lightComponent = light.AddComponent<LightComponent>();
            lightComponent.Type = LightComponent::LightType::Directional;
            lightComponent.Intensity = 25.0f;
            light.AddComponent<TransformComponent>(glm::vec3(-2, 3, 10));

            float time = (float)glfwGetTime();
            float speed = 1.0f; 
            float radius = 10.0f;
            float camX = sin(time * speed) * radius;
            float camZ = cos(time * speed) * radius;
            float camY = sin(time * speed) * radius;

            // float camX = -2;
            // float camZ = 10;
            // float camY = 3.0f;

            glm::vec3 position = glm::vec3(camX, camY, camZ); 
            glm::vec3 target = glm::vec3(0.0f, 0.0f, 0.0f);
            glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

            glm::mat4 viewMatrix = glm::lookAt(position, target, up);

            m_InstancePool->BeginFrame();

            RenderContext ctx;
            ctx.ViewProjection = glm::perspective(glm::radians(60.0f), 1.0f, 0.1f, 1000.0f) * viewMatrix;
            ctx.CameraPosition = position;
            ctx.InstancePool = m_InstancePool.get();

            RenderView view = SceneExtractor::Extract(copied.get(), ctx);

            Renderer::BeginScene();
            m_RenderGraph.Execute(ctx, view);
            Renderer::EndScene();

            m_InstancePool->InsertFrameFence();

            return m_RenderGraph.GetPassColorAttachmentRendererID("ScenePass", 0);
        }

    };

}