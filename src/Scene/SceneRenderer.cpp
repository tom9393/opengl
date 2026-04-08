#include "SceneRenderer.h"
#include "Render/Renderer.h"
#include "Scene/Components.h"
#include "Scene/Entity.h"
#include "../Asset/AssetManager.h"

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

        auto view = m_Scene->GetRegistry().view<MeshComponent>();
        for (auto& e : view)
        {
            auto& meshComp = view.get<MeshComponent>(e);
            UUID meshHandle = meshComp.MeshHandle;
            if (meshHandle == 0) continue;

            Shared<Mesh> mesh = AssetManager::GetAsset<Mesh>(meshHandle);

            Entity entity {e, m_Scene};
            glm::mat4 transform;

            if (entity.HasComponent<TransformComponent>())
            {
                transform = entity.GetComponent<TransformComponent>().GetTransform();
            } else
            {
                entt::entity topTransform = entity.GetRootWithComponent<TransformComponent>();
                if (topTransform == entt::null) continue;
                Entity top = { topTransform, m_Scene};
                transform = top.GetComponent<TransformComponent>().GetTransform();
            }


            if (!IsAABBInFrustum(cameraFrustum, mesh->GetAABB(), transform)) continue;
            DrawCommand cmd;
            cmd.TargetMesh = mesh;
            cmd.Transform = transform;
            cmd.TargetMaterial = nullptr;
            if (entity.HasComponent<MaterialComponent>())
            {
                UUID materialHandle = entity.GetComponent<MaterialComponent>().MaterialHandle;
                cmd.TargetMaterial = AssetManager::GetAsset<Material>(materialHandle);
            }
            if (!cmd.TargetMaterial)
            {
                static Shared<Material> fallbackMaterial = MakeShared<Material>();
                fallbackMaterial->AlbedoTint = glm::vec4(1.0f, 0.0f, 1.0f, 1.0f); // Magenta
                fallbackMaterial->MetallicMultiplier = 0.0f;
                fallbackMaterial->RoughnessMultiplier = 0.5f;
                cmd.TargetMaterial = fallbackMaterial;
            }

            m_OpaqueDrawList.emplace_back(cmd);
        }


        std::sort(m_OpaqueDrawList.begin(), m_OpaqueDrawList.end(), [](const DrawCommand& a, const DrawCommand& b) {
                if (a.TargetMesh.get() != b.TargetMesh.get())
                    return a.TargetMesh.get() < b.TargetMesh.get();
            
                return a.TargetMaterial.get() < b.TargetMaterial.get();
        });

        size_t i = 0;
        while (i < m_OpaqueDrawList.size())
        {
            Shared<Mesh> currentMesh = m_OpaqueDrawList[i].TargetMesh;
            Shared<Material> currentMaterial = m_OpaqueDrawList[i].TargetMaterial;

            // 1. Count how many consecutive instances have the EXACT same mesh
             size_t batchSize = 0;

            // Check if the next commands share the EXACT same mesh and material pointers
            while (i + batchSize < m_OpaqueDrawList.size() && m_OpaqueDrawList[i + batchSize].TargetMesh.get() == currentMesh.get() && m_OpaqueDrawList[i + batchSize].TargetMaterial.get() == currentMaterial.get()) 
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
            Renderer::SubmitInstancedMesh(currentMesh, currentMaterial, transforms);

            // 4. Jump forward to the next entirely different mesh
            i += batchSize;
        }

        PhysicsEngine::RenderDebug(viewProjection);

        Renderer::EndScene();
    }

}
