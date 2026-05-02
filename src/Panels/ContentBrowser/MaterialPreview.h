#pragma once

#include "pch.h"
#include "Asset/Prefab.h"
#include "Asset/PrimitiveBuilder.h"
#include "Preview.h"
#include "Scene/Entity.h"
#include "Scene/SceneExtractor.h"

namespace Engine {

    class MaterialPreview : public Preview
    {
        public:
        void SetAsset(Shared<Asset> &asset) override
        {
            m_Scene = MakeShared<Scene>();

            Entity light = m_Scene->CreateEntity("MaterialPreviewLight");
            LightComponent& lightComponent = light.AddComponent<LightComponent>();
            lightComponent.Type = LightComponent::LightType::Directional;
            lightComponent.Intensity = 25.0f;
            light.AddComponent<TransformComponent>(glm::vec3(-2, 3, 10));

            Entity entity = m_Scene->CreateEntity("MaterialPreviewSphere");
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
            entity.AddComponent<MaterialComponent>(asset->Handle);
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
        }

        private:
        Shared<Scene> m_Scene;
        float m_LastRenderTime = 0.0f;
    };

}