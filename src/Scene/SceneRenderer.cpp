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
        auto view = m_Scene->GetRegistry().view<TransformComponent, SceneCameraComponent>();
        for (auto entity : view) 
        {
            auto [transform, camera] = view.get<TransformComponent, SceneCameraComponent>(entity);
            if (camera.primary) 
            {
                glm::mat4 viewProj = camera.sceneCamera->GetProjection() * glm::inverse(transform.GetTransform());
                glm::vec3 cameraPos = transform.translation;
                FlushDrawCalls(viewProj, cameraPos);
                break;
            }
        }
    }

    void SceneRenderer::OnViewportResize(unsigned int width, unsigned int height)
    {
		m_ViewportWidth = width;
		m_ViewportHeight = height;
        if (!m_Scene) return;
		auto view = m_Scene->m_Registry.view<SceneCameraComponent>();
		for (auto& e : view)
		{
			auto& cameraComponent = view.get<SceneCameraComponent>(e);
			if (!cameraComponent.fixedAspectRatio) cameraComponent.sceneCamera->SetViewportSize(width, height);
		}
    }

    void SceneRenderer::FlushDrawCalls(const glm::mat4& viewProjection, const glm::vec3& cameraPos)
    {
        m_OpaqueDrawList.clear();
        
        CameraData cameraData;
        cameraData.ViewProjection = viewProjection;
        cameraData.Position = cameraPos;
        Renderer::BeginScene(cameraData);

        Frustum cameraFrustum = CreateFrustumFromMatrix(viewProjection);

        auto view = m_Scene->GetRegistry().view<TransformComponent, ModelComponent>();
        for (auto& e : view)
        {
            auto [transform, modelComp] = view.get<TransformComponent, ModelComponent>(e);
            if (!modelComp.model) continue;

            for (auto& mesh : modelComp.model->GetMeshes()) 
            {
                if (!IsAABBInFrustum(cameraFrustum, mesh->GetAABB(), transform.GetTransform())) continue;
                DrawCommand cmd;
                cmd.TargetMesh = mesh;
                cmd.Transform = transform.GetTransform();
                
                // The SortKey: Group by the Mesh's memory address. 
                // Identical meshes (with identical materials) will have the same exact key!
                cmd.SortKey = reinterpret_cast<uintptr_t>(mesh.get()); 
                
                m_OpaqueDrawList.push_back(cmd);
            }
        }

        std::sort(m_OpaqueDrawList.begin(), m_OpaqueDrawList.end(), [](const DrawCommand& a, const DrawCommand& b) {
                return a.SortKey < b.SortKey;
        });

        size_t i = 0;
        while (i < m_OpaqueDrawList.size())
        {
            uintptr_t currentKey = m_OpaqueDrawList[i].SortKey;
            Shared<Mesh> currentMesh = m_OpaqueDrawList[i].TargetMesh;

            // 1. Count how many consecutive instances have the EXACT same mesh
            size_t batchSize = 0;
            while (i + batchSize < m_OpaqueDrawList.size() && m_OpaqueDrawList[i + batchSize].SortKey == currentKey) 
            {
                batchSize++;
            }

            // 2. Extract their transforms into an array
            std::vector<glm::mat4> transforms(batchSize);
            for (size_t j = 0; j < batchSize; j++) 
            {
                transforms[j] = m_OpaqueDrawList[i + j].Transform;
            }

            // 3. Send the entire batch to the GPU at once
            Renderer::SubmitInstancedMesh(currentMesh, transforms);

            // 4. Jump forward to the next entirely different mesh
            i += batchSize;
        }

        //m_Scene->m_Physicsworld.RenderDebug(viewProjection);

        Renderer::EndScene();
    }

}
