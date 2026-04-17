#include "SceneRenderer.h"
#include "Render/Renderer.h"
#include "Scene/Components.h"
#include "Scene/Entity.h"
#include "../Asset/AssetManager.h"

namespace Engine {

    SceneRenderer::SceneRenderer(Scene* scene)
        : m_Scene(scene), m_RenderGraph(m_ViewportWidth, m_ViewportHeight)
    {
        auto& shadowPass2D = m_RenderGraph.AddPass("ShadowPass2D");
        shadowPass2D.SetAttachments({ FramebufferTextureFormat::Depth }, BufferTextureType::Texture2D);
        shadowPass2D.SetCallback([this](RenderContext& ctx) {
            Shared<Shader> shadowShader = ShaderLibrary::Get("ShadowShader");
            shadowShader->Bind();
            shadowShader->SetMat4("u_LightSpaceMatrix", m_DirLightSpaceMatrix);

            // Set viewport to match your shadow map resolution (e.g., 2048x2048)
            // glViewport(0, 0, 2048, 2048); 

            size_t i = 0;
            while (i < m_RenderItems.Opaque.size()) {
                auto& base = m_RenderItems.Opaque[i];
                size_t batchSize = 0;
                while (i + batchSize < m_RenderItems.Opaque.size() && m_RenderItems.Opaque[i + batchSize].TargetMesh == base.TargetMesh) {
                    batchSize++;
                }
                m_Transforms.reserve(batchSize);
                for (size_t j = 0; j < batchSize; j++)
                    m_Transforms.emplace_back(m_RenderItems.Opaque[i + j].Transform);

                m_InstanceSSBO->SetData(m_Transforms.data(), m_Transforms.size() * sizeof(glm::mat4));
                m_InstanceSSBO->Bind(0);
                Renderer::SubmitInstancedMesh(base.TargetMesh, nullptr, m_Transforms.size()); // No material needed for depth
                i += batchSize;
                m_Transforms.clear();
            }
        });

        auto& pointShadowPass = m_RenderGraph.AddPass("PointShadowPass");
        // NOTE: Make sure your RenderGraph supports BufferTextureType::TextureCubeMap!
        pointShadowPass.SetAttachments({ FramebufferTextureFormat::Depth }, BufferTextureType::Cubemap);
        pointShadowPass.SetCallback([this](RenderContext& ctx) {
            Shared<Shader> shadowShader = ShaderLibrary::Get("PointShadowShader");
            shadowShader->Bind();

            // Create the 6 view-projection matrices for the cubemap faces
            // Aspect ratio is 1.0, FOV is 90 degrees
            glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, m_PointLightFarPlane);
            std::vector<glm::mat4> shadowTransforms;
            shadowTransforms.push_back(shadowProj * glm::lookAt(m_PointLightPos, m_PointLightPos + glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f))); // Right
            shadowTransforms.push_back(shadowProj * glm::lookAt(m_PointLightPos, m_PointLightPos + glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f))); // Left
            shadowTransforms.push_back(shadowProj * glm::lookAt(m_PointLightPos, m_PointLightPos + glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f))); // Top
            shadowTransforms.push_back(shadowProj * glm::lookAt(m_PointLightPos, m_PointLightPos + glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f))); // Bottom
            shadowTransforms.push_back(shadowProj * glm::lookAt(m_PointLightPos, m_PointLightPos + glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f))); // Near
            shadowTransforms.push_back(shadowProj * glm::lookAt(m_PointLightPos, m_PointLightPos + glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))); // Far

            // Send matrices to the geometry shader
            for (int i = 0; i < 6; ++i) {
                shadowShader->SetMat4("u_ShadowMatrices[" + std::to_string(i) + "]", shadowTransforms[i]);
            }
            shadowShader->SetFloat("u_FarPlane", m_PointLightFarPlane);
            shadowShader->SetVec3("u_LightPos", m_PointLightPos);

            // Submit meshes (identical logic to the 2D pass)
            size_t i = 0;
            while (i < m_RenderItems.Opaque.size()) {
                auto& base = m_RenderItems.Opaque[i];
                size_t batchSize = 0;
                while (i + batchSize < m_RenderItems.Opaque.size() && m_RenderItems.Opaque[i + batchSize].TargetMesh == base.TargetMesh) {
                    batchSize++;
                }
                m_Transforms.reserve(batchSize);
                for (size_t j = 0; j < batchSize; j++)
                    m_Transforms.emplace_back(m_RenderItems.Opaque[i + j].Transform);

                m_InstanceSSBO->SetData(m_Transforms.data(), m_Transforms.size() * sizeof(glm::mat4));
                m_InstanceSSBO->Bind(0);
                Renderer::SubmitInstancedMesh(base.TargetMesh, nullptr, m_Transforms.size()); 
                i += batchSize;
                m_Transforms.clear();
            }
        });

        // 2. Geometry pass (G-Buffer)
        auto& geoPass = m_RenderGraph.AddPass("GeometryPass");
        geoPass.SetAttachments({ 
            FramebufferTextureFormat::RGBA16F,     // 0: Position
            FramebufferTextureFormat::RGBA16F,     // 1: Normal
            FramebufferTextureFormat::RGBA8,       // 2: Albedo
            FramebufferTextureFormat::RGBA8,       // 3: PBR (Metallic, Roughness, AO)
            FramebufferTextureFormat::RED_INTEGER, // 4: Entity ID
            FramebufferTextureFormat::Depth 
        }, BufferTextureType::Texture2D);
        geoPass.SetClearColor({ 0.0f, 0.0f, 0.0f, 1.0f });
        geoPass.SetClearInt(4, -1);
        geoPass.SetCallback([this](RenderContext& ctx) {
            Shared<Shader> gBufferShader = ShaderLibrary::Get("GBufferShader");
            gBufferShader->Bind();

            size_t i = 0;
            while (i < m_RenderItems.Opaque.size()) {
                auto& base = m_RenderItems.Opaque[i];
                size_t batchSize = 0;

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
                
                if (base.TargetMaterial) {
                    base.TargetMaterial->Bind(); 
                }

                Renderer::SubmitInstancedMesh(base.TargetMesh, base.TargetMaterial, m_Transforms.size());
                i += batchSize;
                m_Transforms.clear();
            }
        });

        // 3. Deferred Lighting Pass
        auto& lightingPass = m_RenderGraph.AddPass("LightingPass");
        lightingPass.SetAttachments({ FramebufferTextureFormat::RGBA8 }, BufferTextureType::Texture2D );
        lightingPass.SetClearColor({ 0.2f, 0.2f, 0.2f, 1.0f });
        lightingPass.SetCallback([this](RenderContext &ctx) {
            Shared<Shader> deferredShader = ShaderLibrary::Get("DeferredLighting");
            deferredShader->Bind();

            // Note: In your actual engine, get these texture IDs from the G-Buffer Pass Framebuffer
            deferredShader->SetInt("gPosition", 0);
            deferredShader->SetInt("gNormal", 1);
            deferredShader->SetInt("gAlbedo", 2);
            deferredShader->SetInt("gPBR", 3);
            
            // Set Shadow Maps
            deferredShader->SetInt("u_DirSpotShadowMap", 4);
            deferredShader->SetInt("u_PointShadowCubemap", 5);
            deferredShader->SetFloat("u_FarPlane", m_PointLightFarPlane);

            // Render full screen quad to calculate lighting
            // Assuming you have a static method like this in your Renderer
            Renderer::DrawFullscreenQuad(); 
            
            PhysicsEngine::RenderDebug(ctx.ViewProjection);
        });

        m_InstanceSSBO = ShaderStorageBuffer::Create(m_MaxInstances * sizeof(glm::mat4));
    }

    void SceneRenderer::PrepareScene(const RenderContext& context)
    {
        m_RenderItems.Opaque.clear();
        Frustum cameraFrustum = CreateFrustumFromMatrix(context.ViewProjection);

        m_Scene->UpdateWorldTransforms();

        m_LightData.LightCount = 0;

        auto lightView = m_Scene->GetRegistry().view<LightComponent, TransformComponent>();
        for (auto& e : lightView)
        {
            if (m_LightData.LightCount >= 64) break;
            auto [light, transform] = lightView.get<LightComponent, TransformComponent>(e);
        
            glm::vec3 direction = transform.GetForward();
            float typeFloat = static_cast<float>(light.Type);
            int idx = m_LightData.LightCount;

            switch (light.Type)
            {
                case LightComponent::LightType::Point:
                {
                    m_LightData.Lights[idx].PositionType = glm::vec4(transform.translation, typeFloat);
                    m_LightData.Lights[idx].ColorInner   = glm::vec4(light.Color * light.Intensity, 0.0f);
                    m_LightData.Lights[idx].DirectionOuter = glm::vec4(0.0f);
                    m_LightData.Lights[idx].LightSpaceMatrix = glm::mat4(1.0f);
                    
                    m_PointLightPos = transform.translation; // Track for omnidirectional shadow
                    break;
                }
                case LightComponent::LightType::Directional:
                {
                    m_LightData.Lights[idx].PositionType = glm::vec4(0.0f, 0.0f, 0.0f, typeFloat);
                    m_LightData.Lights[idx].ColorInner   = glm::vec4(light.Color * light.Intensity, 0.0f);
                    m_LightData.Lights[idx].DirectionOuter = glm::vec4(direction, 0.0f);
                    
                    // Ortho projection for directional shadows
                    glm::mat4 lightProjection = glm::ortho(-20.0f, 20.0f, -20.0f, 20.0f, 1.0f, 50.0f);
                    glm::mat4 lightViewMat = glm::lookAt(transform.translation, transform.translation + direction, glm::vec3(0.0f, 1.0f, 0.0f));
                    m_LightData.Lights[idx].LightSpaceMatrix = lightProjection * lightViewMat;
                    
                    m_DirLightSpaceMatrix = m_LightData.Lights[idx].LightSpaceMatrix; // Track for shadow pass
                    break;
                }
                case LightComponent::LightType::Cone:
                {
                    m_LightData.Lights[idx].PositionType = glm::vec4(transform.translation, typeFloat);
                    m_LightData.Lights[idx].ColorInner   = glm::vec4(light.Color * light.Intensity, light.InnerCutOff);
                    m_LightData.Lights[idx].DirectionOuter = glm::vec4(direction, light.OuterCutOff);
                    
                    // Perspective projection for spotlight shadows
                    float fov = glm::acos(light.OuterCutOff) * 2.0f;
                    glm::mat4 lightProjection = glm::perspective(fov, 1.0f, 0.1f, 100.0f);
                    glm::mat4 lightViewMat = glm::lookAt(transform.translation, transform.translation + direction, glm::vec3(0.0f, 1.0f, 0.0f));
                    m_LightData.Lights[idx].LightSpaceMatrix = lightProjection * lightViewMat;
                    
                    m_DirLightSpaceMatrix = m_LightData.Lights[idx].LightSpaceMatrix;
                    break;
                }
            }
            m_LightData.LightCount++;
        }

        Renderer::UpdateLightBuffer(m_LightData);

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
