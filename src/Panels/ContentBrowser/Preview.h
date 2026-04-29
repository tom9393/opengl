#pragma once

#include "Render/RenderGraph.h"
#include "Render/Renderer.h"
#include "Scene/SceneExtractor.h"

namespace Engine {

    class Scene;

    class Preview {

        public:
        virtual ~Preview() = default;
        virtual uint32_t GetPreview() = 0;
        virtual void SetAsset(Shared<Asset> &asset) = 0;

        RenderGraph& GetRenderGraph() { return m_RenderGraph; }

        uint32_t UpdatePreview()
        {
            if (!m_Scene) return 0;

            float time = (float)glfwGetTime();
            float speed = 1.0f; 
            float radius = 10.0f;
            float camX = sin(time * speed) * radius;
            float camZ = cos(time * speed) * radius;
            float camY = 2.0f;

            glm::vec3 position = glm::vec3(camX, camY, camZ); 
            glm::vec3 target = glm::vec3(0.0f, 0.0f, 0.0f);
            glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

            glm::mat4 viewMatrix = glm::lookAt(position, target, up);

            m_InstancePool->BeginFrame();

            RenderContext ctx;
            ctx.ViewProjection = glm::perspective(glm::radians(60.0f), 1.0f, 0.1f, 1000.0f) * viewMatrix;
            ctx.CameraPosition = position;
            ctx.InstancePool = m_InstancePool.get();

            RenderView view = SceneExtractor::Extract(m_Scene.get(), ctx);

            Renderer::BeginScene();
            m_RenderGraph.Execute(ctx, view);
            Renderer::EndScene();

            m_InstancePool->InsertFrameFence();

            return m_RenderGraph.GetPassColorAttachmentRendererID("ScenePass", 0);
        }

        protected:
        RenderGraph m_RenderGraph{256, 256};
        Shared<DynamicBufferPool> m_InstancePool;
        Shared<Scene> m_Scene;
    };

}