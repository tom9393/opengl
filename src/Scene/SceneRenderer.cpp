#include "SceneRenderer.h"
#include "Render/Renderer.h"
#include "Scene/Components.h"

namespace Engine {

    SceneRenderer::SceneRenderer(Scene* scene)
        : m_Scene(scene)
    {
    }

    void SceneRenderer::RenderEditor(const glm::mat4& viewProjection, const glm::vec3& cameraPosition)
    {
        FlushDrawCalls(viewProjection, cameraPosition);
    }

    void SceneRenderer::RenderRuntime()
    {
        // 1. Find the primary runtime camera in the ECS
        glm::mat4 viewProj = glm::mat4(1.0f);
        glm::vec3 cameraPos = glm::vec3(0.0f);
        bool hasCamera = false;

        auto view = m_Scene->GetRegistry().view<TransformComponent, SceneCameraComponent>();
        for (auto entity : view) 
        {
            auto [transform, camera] = view.get<TransformComponent, SceneCameraComponent>(entity);
            if (camera.primary) 
            {
                // Calculate ViewProjection from the entity's transform
                viewProj = camera.sceneCamera->GetProjection() * glm::inverse(transform.GetTransform());
                cameraPos = transform.translation;
                hasCamera = true;
                break; // Only use the first primary camera we find
            }
        }

        // 2. Render the scene if a camera exists
        if (hasCamera) {
            FlushDrawCalls(viewProj, cameraPos);
        }
    }

    void SceneRenderer::FlushDrawCalls(const glm::mat4& viewProjection, const glm::vec3& cameraPos)
    {
        // --- 1. GLOBAL RENDER SETUP ---
        // Bind PBR Shaders, push light uniforms, set camera position, etc.
        // e.g., Renderer::BeginScene(viewProjection, cameraPos);
	
	CameraData cameraData;
	cameraData.ViewProjection = viewProjection;
	cameraData.Position = cameraPos;
	Renderer::BeginScene(cameraData);

        // --- 2. GENERATE FRUSTUM ---
        Frustum cameraFrustum = CreateFrustumFromMatrix(viewProjection);

        // --- 3. ECS ITERATION & CULLING ---
	auto view = m_Scene->GetRegistry().view<TransformComponent, ModelComponent>();
        for (auto& e : view)
        {
		auto [transform, modelComp] = view.get<TransformComponent, ModelComponent>(e);
		if (!modelComp.model) continue;
		Renderer::SubmitModel(modelComp.model, transform.GetTransform());
        }

	m_Scene->m_Physicsworld.RenderDebug(viewProjection);

	Renderer::EndScene();

        // --- 4. END SCENE ---
        // e.g., Renderer::EndScene();
    }

}
