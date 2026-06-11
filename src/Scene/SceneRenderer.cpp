#include "SceneRenderer.h"
#include "Asset/AssetManager.h"
#include "Scene/Entity.h"
#include "Passes/ShadowPass2D.h"
#include "Passes/ShadowPass3D.h"
#include "Passes/GeometryPass.h"
#include "Passes/ScenePass.h"
#include "Passes/ParticlePass.h"
#include "Passes/BloomPass.h"
#include "Passes/PostProcessPass.h"
#include "Render/Renderer.h"
#include "Render/SceneCamera.h"
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
        m_RenderGraph.AddPass<ParticlePass>("ParticlePass");
        AddBloomPasses(m_RenderGraph, "SceneColorParticles");
        m_RenderGraph.AddPass<PostProcessPass>("PostProcessPass");

        // FinalColor (post-processed) is the presented viewport texture; this drives culling.
        m_RenderGraph.MarkPresent("FinalColor");
        m_RenderGraph.Compile();
    }

    // Editor viewport drags fire OnViewportResize every frame; respecifying the
    // whole graph's texture storage each time is wasteful, so the resize is
    // applied once the requested size has been stable for a few frames. Cameras
    // get the new aspect immediately, so the stretched blit stays proportional.
    static constexpr unsigned int ResizeSettleFrames = 10;

    void SceneRenderer::ApplyPendingResize()
    {
        if (m_PendingWidth == m_ViewportWidth && m_PendingHeight == m_ViewportHeight) return;
        if (++m_FramesSincePendingResize < ResizeSettleFrames) return;

        m_ViewportWidth = m_PendingWidth;
        m_ViewportHeight = m_PendingHeight;
        m_RenderGraph.Resize(m_ViewportWidth, m_ViewportHeight);
    }

    void SceneRenderer::RenderEditor(const glm::mat4& viewProj, const glm::vec3& camPos, float nearPlane, float farPlane)
    {
        ApplyPendingResize();
        m_InstancePool->BeginFrame();

        RenderContext ctx;
        ctx.ViewProjection = viewProj;
        ctx.CameraPosition = camPos;
        ctx.InstancePool = m_InstancePool.get();

        RenderView view = SceneExtractor::Extract(m_Scene, ctx);
        view.NearPlane = nearPlane;
        view.FarPlane = farPlane;
        view.ViewportSize = { m_ViewportWidth, m_ViewportHeight };

        Renderer::BeginScene(ctx);
        m_RenderGraph.Execute(ctx, view);
        Renderer::EndScene();

        m_InstancePool->InsertFrameFence();
    }

    void SceneRenderer::RenderRuntime()
    {
        ApplyPendingResize();
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

                RenderView renderView = SceneExtractor::Extract(m_Scene, ctx);
                renderView.ViewportSize = { m_ViewportWidth, m_ViewportHeight };
                renderView.NearPlane = camera.sceneCamera->GetNearClip();
                renderView.FarPlane  = camera.sceneCamera->GetFarClip();

                Renderer::BeginScene(ctx);
                m_RenderGraph.Execute(ctx, renderView);
                Renderer::EndScene();

                m_InstancePool->InsertFrameFence();
                break;
            }
        }
    }

    void SceneRenderer::OnViewportResize(unsigned int width, unsigned int height)
    {
        if (width == 0 || height == 0) return;
        if (width != m_PendingWidth || height != m_PendingHeight)
        {
            m_PendingWidth = width;
            m_PendingHeight = height;
            m_FramesSincePendingResize = 0;
        }
        if (!m_Scene) return;
		auto view = m_Scene->GetRegistry().view<SceneCameraComponent>();
		for (auto& e : view)
		{
			auto& cameraComponent = view.get<SceneCameraComponent>(e);
			if (!cameraComponent.fixedAspectRatio) cameraComponent.sceneCamera->SetViewportSize(width, height);
		}
    }

}
