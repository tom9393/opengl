#pragma once

#include "Render/RenderGraph.h"
#include "Render/Renderer.h"
#include "Scene/SceneExtractor.h"

namespace Engine {

    class Scene;

    class Preview {

        public:
        virtual ~Preview() = default;
        virtual void SetAsset(Shared<Asset> &asset) = 0;

        uint32_t GetCachedTextureID() const { return m_Texture ? m_Texture->GetID() : 0; }

        uint32_t UpdatePreview(RenderGraph &graph, DynamicBufferPool *pool)
        {
            if (!m_Scene) return 0;

            float time = (float)glfwGetTime();
            if (time - m_LastRenderTime < (1.0f / 60.0f)) {
                // Return the cached framebuffer texture if it's not time to render yet
                return m_Texture->GetID();
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

            Renderer::BeginScene();
            graph.Execute(ctx, view);
            Renderer::EndScene();

            pool->InsertFrameFence();

            uint32_t sharedTexID = graph.GetPassColorAttachmentRendererID("ScenePass", 0);

            if (!m_Texture)
            {
                TextureLoadData data;
                data.Height = 256;
                data.Width = 256;
                m_Texture = Texture2D::Create(data);
            }

            m_Texture->Copy(sharedTexID, 256, 256);

            return m_Texture->GetID();
        }

        protected:
        Shared<Scene> m_Scene;

        private:
        Shared<Texture2D> m_Texture;
        float m_LastRenderTime = 0.0f;
    };

}