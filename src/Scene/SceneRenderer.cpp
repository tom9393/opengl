#include "SceneRenderer.h"
#include "Asset/AssetManager.h"
#include "Scene/Entity.h"
#include "Passes/ShadowPass2D.h"
#include "Passes/ShadowPass3D.h"
#include "Passes/GeometryPass.h"
#include "Passes/ScenePass.h"
#include "Render/Renderer.h"
#include "RenderView.h"
#include "Scene/Components.h"
#include "Scene/SceneExtractor.h"

namespace Engine {

    SceneRenderer::SceneRenderer(Scene* scene)
        : m_Scene(scene), m_RenderGraph(m_ViewportWidth, m_ViewportHeight)
    {
        m_InstancePool = MakeShared<DynamicBufferPool>(10 * 1024 * 1024); // 10MB

        m_RenderGraph.AddPass<ShadowPass2D>("ShadowPass2D");
        m_RenderGraph.AddPass<ShadowPass3D>("PointShadowPass");
        m_RenderGraph.AddPass<GeometryPass>("GeometryPass");
        m_RenderGraph.AddPass<ScenePass>("ScenePass");
    }

    void SceneRenderer::RenderEditor(const glm::mat4& viewProj, const glm::vec3& camPos)
    {
        m_InstancePool->BeginFrame();

        RenderContext ctx;
        ctx.ViewProjection = viewProj;
        ctx.CameraPosition = camPos;
        ctx.InstancePool = m_InstancePool.get();

        RenderView view = SceneExtractor::Extract(m_Scene, ctx);

        Renderer::BeginScene();
        m_RenderGraph.Execute(ctx, view);
        Renderer::EndScene();

        m_InstancePool->InsertFrameFence();
    }

    void SceneRenderer::RenderRuntime()
    {
        auto view = m_Scene->GetRegistry().view<TransformComponent, SceneCameraComponent>();
        for (auto entity : view) 
        {
            auto [transform, camera] = view.get<TransformComponent, SceneCameraComponent>(entity);
            if (camera.primary) 
            {
                m_InstancePool->BeginFrame();

                RenderContext ctx;
                ctx.ViewProjection = camera.sceneCamera->GetProjection() * glm::inverse(transform.GetTransform());
                ctx.CameraPosition = transform.translation;
                ctx.InstancePool = m_InstancePool.get();

                RenderView view = SceneExtractor::Extract(m_Scene, ctx);
                
                Renderer::BeginScene();
                m_RenderGraph.Execute(ctx, view);
                Renderer::EndScene();

                m_InstancePool->InsertFrameFence();
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
