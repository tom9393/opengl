#include "SceneRenderer.h"
#include "Render/Renderer.h"
#include "Scene/Components.h"
#include "Scene/Entity.h"
#include "../Asset/AssetManager.h"

namespace Engine {

    SceneRenderer::SceneRenderer(Scene* scene)
        : m_Scene(scene), m_RenderGraph(m_ViewportWidth, m_ViewportHeight)
    {
        auto& pass = m_RenderGraph.AddPass("ScenePass");
        pass.SetAttachments({ FramebufferTextureFormat::RGBA8, FramebufferTextureFormat::RED_INTEGER, FramebufferTextureFormat::Depth });
        pass.SetClearColor({ 0.2f, 0.2f, 0.2f, 1.0f });
        pass.SetClearInt(1, -1);
        pass.SetCallback([this](RenderContext &ctx)
        {
            size_t i = 0;
            while (i < m_RenderItems.Opaque.size())
            {
                auto& base = m_RenderItems.Opaque[i];
                size_t batchSize = 0;

                // Find how many items share this Mesh + Material
                while (i + batchSize < m_RenderItems.Opaque.size() && 
                    m_RenderItems.Opaque[i + batchSize].TargetMesh == base.TargetMesh && 
                    m_RenderItems.Opaque[i + batchSize].TargetMaterial == base.TargetMaterial) 
                {
                    batchSize++;
                }

                m_Transforms.reserve(batchSize);
                for (size_t j = 0; j < batchSize; j++)
                    m_Transforms.emplace_back(m_RenderItems.Opaque[i + j].Transform);

                m_InstanceSSBO->SetData(m_Transforms.data(), m_Transforms.size() * sizeof(glm::mat4));
                m_InstanceSSBO->Bind(0);
                Renderer::SubmitInstancedMesh(base.TargetMesh, base.TargetMaterial, m_Transforms.size());
                i += batchSize;
                m_Transforms.clear();
            }

            PhysicsEngine::RenderDebug(ctx.ViewProjection);
        });

        m_InstanceSSBO = ShaderStorageBuffer::Create(m_MaxInstances * sizeof(glm::mat4));
    }

    void SceneRenderer::PrepareScene(const RenderContext& context)
    {
        m_RenderItems.Opaque.clear();
        Frustum cameraFrustum = CreateFrustumFromMatrix(context.ViewProjection);

        m_Scene->UpdateWorldTransforms();

        SceneLightData lightData = {};
        lightData.LightCount = 0;

        auto lightView = m_Scene->GetRegistry().view<LightComponent, TransformComponent>();
        for (auto& e : lightView)
        {
            if (lightData.LightCount >= 64) break;
            auto [light, transform] = lightView.get<LightComponent, TransformComponent>(e);
        
            glm::vec3 direction = transform.GetForward();
            float typeFloat = static_cast<float>(light.Type);

            switch (light.Type)
            {
                case LightComponent::LightType::Point:
                    lightData.Lights[lightData.LightCount].PositionType = glm::vec4(transform.translation, typeFloat);
                    lightData.Lights[lightData.LightCount].ColorInner   = glm::vec4(light.Color * light.Intensity, 0.0f);
                    lightData.Lights[lightData.LightCount].DirectionOuter = glm::vec4(0.0f);
                    break;

                case LightComponent::LightType::Directional:
                    // Directional lights don't care about position
                    lightData.Lights[lightData.LightCount].PositionType = glm::vec4(0.0f, 0.0f, 0.0f, typeFloat);
                    lightData.Lights[lightData.LightCount].ColorInner   = glm::vec4(light.Color * light.Intensity, 0.0f);
                    lightData.Lights[lightData.LightCount].DirectionOuter = glm::vec4(direction, 0.0f);
                    break;

                case LightComponent::LightType::Cone:
                    lightData.Lights[lightData.LightCount].PositionType = glm::vec4(transform.translation, typeFloat);
                    lightData.Lights[lightData.LightCount].ColorInner   = glm::vec4(light.Color * light.Intensity, light.InnerCutOff);
                    lightData.Lights[lightData.LightCount].DirectionOuter = glm::vec4(direction, light.OuterCutOff);
                    break;
            }
            
            lightData.LightCount++;
        }

        Renderer::UpdateLightBuffer(lightData);

        auto view = m_Scene->GetRegistry().view<MeshComponent, TransformComponent, MaterialComponent>();
        for (auto& e : view)
        {
            auto [meshComponent, transformComponent, materialComponent] = view.get<MeshComponent, TransformComponent, MaterialComponent>(e);
            if (meshComponent.MeshHandle == 0) continue;

            Shared<Mesh> mesh = AssetManager::GetAsset<Mesh>(meshComponent.MeshHandle);
            glm::mat4 transform = transformComponent.GetTransform();

            // Frustum Culling
            if (!IsAABBInFrustum(cameraFrustum, mesh->GetAABB(), transform)) continue;

            // Build Command
            DrawCommand cmd;
            cmd.TargetMesh = mesh;
            cmd.Transform = transform;
            cmd.TargetMaterial = AssetManager::GetAsset<Material>(materialComponent.MaterialHandle);
            
            // Instead of submitting, just fill the list
            m_RenderItems.Opaque.emplace_back(cmd);
        }

        // Sort once per frame to benefit all passes
        std::sort(m_RenderItems.Opaque.begin(), m_RenderItems.Opaque.end(), [](const DrawCommand& a, const DrawCommand& b) {
            if (a.TargetMesh.get() != b.TargetMesh.get())
                return a.TargetMesh.get() < b.TargetMesh.get();
        
            return a.TargetMaterial.get() < b.TargetMaterial.get();
        });
    }

    void SceneRenderer::RenderEditor(const glm::mat4& viewProj, const glm::vec3& camPos)
    {
        RenderContext ctx{ viewProj, camPos };
        PrepareScene(ctx);
        
        Renderer::BeginScene(ctx);
        m_RenderGraph.Execute(ctx);
        Renderer::EndScene();
    }

    void SceneRenderer::RenderRuntime()
    {
        auto view = m_Scene->GetRegistry().view<TransformComponent, SceneCameraComponent>();
        for (auto entity : view) 
        {
            auto [transform, camera] = view.get<TransformComponent, SceneCameraComponent>(entity);
            if (camera.primary) 
            {
                RenderContext ctx {camera.sceneCamera->GetProjection() * glm::inverse(transform.GetTransform()), transform.translation};
                PrepareScene(ctx);
                
                Renderer::BeginScene(ctx);
                m_RenderGraph.Execute(ctx);
                Renderer::EndScene();
                break;
            }
        }
    }

    void SceneRenderer::OnViewportResize(unsigned int width, unsigned int height)
    {
		m_ViewportWidth = width;
		m_ViewportHeight = height;
        m_RenderGraph.Resize(m_ViewportWidth, m_ViewportHeight);
        if (!m_Scene) return;
		auto view = m_Scene->m_Registry.view<SceneCameraComponent>();
		for (auto& e : view)
		{
			auto& cameraComponent = view.get<SceneCameraComponent>(e);
			if (!cameraComponent.fixedAspectRatio) cameraComponent.sceneCamera->SetViewportSize(width, height);
		}
    }

}
