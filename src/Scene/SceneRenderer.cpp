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
            SubmitBatchedMeshes(m_RenderItems.Opaque);
            
            // Render Physics debug shapes on top
            PhysicsEngine::RenderDebug(ctx.ViewProjection);
        
        });
    }

    void SceneRenderer::PrepareScene(const RenderContext& context)
    {
        m_RenderItems.Opaque.clear();
        Frustum cameraFrustum = CreateFrustumFromMatrix(context.ViewProjection);

        m_Scene->UpdateWorldTransforms();

        SceneLightData lightData = {};
        lightData.LightCount = 0;

        auto lightView = m_Scene->GetRegistry().view<PointLightComponent, TransformComponent>();
        for (auto& e : lightView)
        {
            auto [transform, light] = lightView.get<TransformComponent, PointLightComponent>(e);
        
            lightData.Lights[lightData.LightCount].Position = glm::vec4(transform.translation, 1.0f);
            lightData.Lights[lightData.LightCount].Color = glm::vec4(light.Color * light.Intensity, 1.0f);
            lightData.LightCount++;
        }

        Renderer::UpdateLightBuffer(lightData);

        auto view = m_Scene->GetRegistry().view<MeshComponent, TransformComponent>();
        for (auto& e : view)
        {
            auto [meshComponent, transformComponent] = view.get<MeshComponent, TransformComponent>(e);
            if (meshComponent.MeshHandle == 0) continue;

            Shared<Mesh> mesh = AssetManager::GetAsset<Mesh>(meshComponent.MeshHandle);
            Entity entity {e, m_Scene};

            glm::mat4 &transform = transformComponent.WorldTransform;

            // Frustum Culling
            if (!IsAABBInFrustum(cameraFrustum, mesh->GetAABB(), transform)) continue;

            // Build Command
            DrawCommand cmd;
            cmd.TargetMesh = mesh;
            cmd.Transform = transform;;
            
            if (entity.HasComponent<MaterialComponent>())
                cmd.TargetMaterial = AssetManager::GetAsset<Material>(entity.GetComponent<MaterialComponent>().MaterialHandle);
            else continue;
            
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

    void SceneRenderer::SubmitBatchedMeshes(std::vector<DrawCommand>& commands)
    {
        size_t i = 0;
        while (i < commands.size())
        {
            auto& base = commands[i];
            size_t batchSize = 0;

            // Find how many items share this Mesh + Material
            while (i + batchSize < commands.size() && 
                   commands[i + batchSize].TargetMesh == base.TargetMesh && 
                   commands[i + batchSize].TargetMaterial == base.TargetMaterial) 
            {
                batchSize++;
            }

            m_TransformBuffer.reserve(batchSize);
            for (size_t j = 0; j < batchSize; j++)
                m_TransformBuffer.emplace_back(commands[i + j].Transform);

            Renderer::SubmitInstancedMesh(base.TargetMesh, base.TargetMaterial, m_TransformBuffer);
            i += batchSize;
            m_TransformBuffer.clear();
        }
    }

}
