#pragma once

#include "pch.h"
#include "Asset/Prefab.h"
#include "Preview.h"
#include "Scene/Entity.h"
#include "Scene/SceneExtractor.h"

namespace Engine {

    class PrefabPreview : public Preview
    {
        public:
        void SetAsset(Shared<Asset> &asset) override
        {
            auto prefab = std::static_pointer_cast<Prefab>(asset);
            m_Scene = Scene::Copy(prefab);
            Entity light = m_Scene->CreateEntity("ModelPreviewLight");
            LightComponent& lightComponent = light.AddComponent<LightComponent>();
            lightComponent.Type = LightComponent::LightType::Directional;
            lightComponent.Intensity = 25.0f;
            light.AddComponent<TransformComponent>(glm::vec3(-2, 3, 10));
        }

        void UpdatePreview(RenderGraph &graph, DynamicBufferPool *pool) override
        {
            if (!m_Scene) return;

            float time = (float)glfwGetTime();
            if (time - m_LastRenderTime < (1.0f / 60.0f)) {
                return;
            }
            m_LastRenderTime = time;

            float speed = 1.0f; 
            float radius = 10.0f;
            float camX = sin(time * speed) * radius;
            float camZ = cos(time * speed) * radius;
            float camY = 2.0f;

            glm::vec3 position = glm::vec3(camX, camY, camZ); 
            glm::vec3 target = glm::vec3(0.0f, 0.0f, 0.0f);
            glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

            glm::mat4 viewMatrix = glm::lookAt(position, target, up);

            pool->BeginFrame();

            RenderContext ctx;
            ctx.ViewProjection = glm::perspective(glm::radians(60.0f), 1.0f, 0.1f, 1000.0f) * viewMatrix;
            ctx.CameraPosition = position;
            ctx.InstancePool = pool;

            RenderView view = SceneExtractor::Extract(m_Scene.get(), ctx);

            Renderer::BeginScene(ctx);
            graph.Execute(ctx, view);
            Renderer::EndScene();

            pool->InsertFrameFence();

            uint32_t sharedTexID = graph.GetPassColorAttachmentRendererID("PostProcessPass", 0);

            if (!m_Texture)
            {
                TextureLoadData data;
                data.Height = 256;
                data.Width = 256;
                m_Texture = Texture2D::Create(data);
            }

            m_Texture->Copy(sharedTexID, 256, 256);
        }

        private:
        Shared<Scene> m_Scene;
        float m_LastRenderTime = 0.0f;
    };

}