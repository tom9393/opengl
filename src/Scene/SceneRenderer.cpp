#include "SceneRenderer.h"
#include "Render/Renderer.h"
#include "Scene/Components.h"
#include "Scene/Entity.h"
#include "../Asset/AssetManager.h"

namespace Engine {

    SceneRenderer::SceneRenderer(Scene* scene)
        : m_Scene(scene), m_RenderGraph(m_ViewportWidth, m_ViewportHeight), m_PointLightFarPlane(100.0f)
    {
        auto& shadowPass2D = m_RenderGraph.AddPass("ShadowPass2D");
        // Ensure BufferTextureType::Texture2DArray is supported by your RenderGraph!
        shadowPass2D.SetAttachments({ FramebufferTextureFormat::Depth }, BufferTextureType::Texture2D_Array);
        shadowPass2D.SetCallback([this](RenderContext& ctx) {
            Shared<Shader> shadowShader = ShaderLibrary::Get("ShadowShader");
            shadowShader->Bind();

            auto fb = m_RenderGraph.GetPassFramebuffer("ShadowPass2D");
            uint32_t fboID = fb ? fb->GetDepthAttachmentRendererID() : 0;

            // Loop over all directional/spot lights that cast shadows
            for (size_t layer = 0; layer < m_DirSpotShadowCasters.size(); ++layer) {
                // Attach specific layer to FBO
                m_RenderGraph.GetPassFramebuffer("ShadowPass2D")->GetFramebuffer()->AttachLayer(GL_DEPTH_ATTACHMENT, layer);
                
                glClear(GL_DEPTH_BUFFER_BIT);
                shadowShader->SetMat4("u_LightSpaceMatrix", m_DirSpotShadowCasters[layer]);

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
            }
        });

        auto& pointShadowPass = m_RenderGraph.AddPass("PointShadowPass");
        // Ensure BufferTextureType::CubemapArray is supported by your RenderGraph!
        pointShadowPass.SetAttachments({ FramebufferTextureFormat::Depth }, BufferTextureType::Cubemap_Array);
        pointShadowPass.SetViewportSize(1024, 1024);
        pointShadowPass.SetCallback([this](RenderContext& ctx) {
            Shared<Shader> pointshadowShader = ShaderLibrary::Get("PointShadowShader");
            pointshadowShader->Bind();

            auto fb = m_RenderGraph.GetPassFramebuffer("PointShadowPass");
            uint32_t fboID = fb ? fb->GetDepthAttachmentRendererID() : 0;

            glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, m_PointLightFarPlane);

            for (size_t layer = 0; layer < m_PointShadowCasters.size(); ++layer) {
                glm::vec3 lightPos = m_PointShadowCasters[layer];
                
                // Note: Geometry shader must handle `gl_Layer = (layer * 6) + gl_InvocationID;`
                // Alternatively, attach layer by layer using glFramebufferTextureLayer if modifying GS is difficult.
                
                // For this example, we assume you update the geometry shader with `u_BaseLayer` 
                // OR you manually iterate all 6 faces per layer here. 
                // Using glFramebufferTexture (no layer) attaches the whole array.
                
                glClear(GL_DEPTH_BUFFER_BIT);

                std::vector<glm::mat4> shadowTransforms;
                shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f))); 
                shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f))); 
                shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f))); 
                shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f))); 
                shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f))); 
                shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))); 

                for (int i = 0; i < 6; ++i) {
                    pointshadowShader->SetMat4("u_ShadowMatrices[" + std::to_string(i) + "]", shadowTransforms[i]);
                }
                pointshadowShader->SetFloat("u_FarPlane", m_PointLightFarPlane);
                pointshadowShader->SetVec3("u_LightPos", lightPos);
                pointshadowShader->SetInt("u_BaseLayer", layer); // NEW: Send layer to geometry shader

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
        auto& lightingPass = m_RenderGraph.AddPass("ScenePass");
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
            deferredShader->SetInt("u_DirSpotShadowMaps", 4);    // Updated to array
            deferredShader->SetInt("u_PointShadowCubemaps", 5);  // Updated to array
            deferredShader->SetFloat("u_FarPlane", m_PointLightFarPlane);

            auto gBufferFB = m_RenderGraph.GetPassFramebuffer("GeometryPass");
            auto shadow2DFB = m_RenderGraph.GetPassFramebuffer("ShadowPass2D");
            auto shadowCubeFB = m_RenderGraph.GetPassFramebuffer("PointShadowPass");

            if (gBufferFB)
            {
                // Note: Ensure glad.h is included at the top of SceneRenderer.cpp
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, gBufferFB->GetColorAttachmentRendererID(0)); // Position
                glActiveTexture(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_2D, gBufferFB->GetColorAttachmentRendererID(1)); // Normal
                glActiveTexture(GL_TEXTURE2);
                glBindTexture(GL_TEXTURE_2D, gBufferFB->GetColorAttachmentRendererID(2)); // Albedo
                glActiveTexture(GL_TEXTURE3);
                glBindTexture(GL_TEXTURE_2D, gBufferFB->GetColorAttachmentRendererID(3)); // PBR
            }

            // Bind 2D Shadow Map
            if (shadow2DFB)
            {
                glActiveTexture(GL_TEXTURE4);
                glBindTexture(GL_TEXTURE_2D_ARRAY, shadow2DFB->GetDepthAttachmentRendererID()); 
            }

            // Bind Point Light Shadow Cubemap
            if (shadowCubeFB)
            {
                glActiveTexture(GL_TEXTURE5);
                glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, shadowCubeFB->GetDepthAttachmentRendererID()); 
            }

            // Render full screen quad to calculate lighting
            // Assuming you have a static method like this in your Renderer
            glDisable(GL_DEPTH_TEST);
            Renderer::DrawFullscreenQuad(); 
            glEnable(GL_DEPTH_TEST);
            
            PhysicsEngine::RenderDebug(ctx.ViewProjection);
        });

        m_InstanceSSBO = ShaderStorageBuffer::Create(m_MaxInstances * sizeof(glm::mat4));
    }

    void SceneRenderer::PrepareScene(const RenderContext& context)
    {
        m_RenderItems.Opaque.clear();
        Frustum cameraFrustum = CreateFrustumFromMatrix(context.ViewProjection);

        m_Scene->UpdateWorldTransforms();

        m_DirSpotShadowCasters.clear();
        m_PointShadowCasters.clear();
        m_LightData.LightCount = 0;

        auto lightView = m_Scene->GetRegistry().view<LightComponent, TransformComponent>();
        for (auto& e : lightView)
        {
            if (m_LightData.LightCount >= 64) break;
            auto [light, transform] = lightView.get<LightComponent, TransformComponent>(e);

            int idx = m_LightData.LightCount;
            int shadowIndex = -1; // Default to no shadow
            float typeFloat = static_cast<float>(light.Type);
            glm::vec3 direction = transform.GetForward();

            if (!light.CastShadows) continue;
        
            switch (light.Type)
            {
                case LightComponent::LightType::Point:
                {
                    m_LightData.Lights[idx].PositionType = glm::vec4(transform.translation, typeFloat);
                    m_LightData.Lights[idx].ColorInner   = glm::vec4(light.Color * light.Intensity, 0.0f);
                    m_LightData.Lights[idx].DirectionOuter = glm::vec4(0.0f);
                    m_LightData.Lights[idx].LightSpaceMatrix = glm::mat4(1.0f);
                    
                    if (light.CastShadows) {
                        shadowIndex = m_PointShadowCasters.size();
                        m_PointShadowCasters.push_back(transform.translation);
                    }
                    break;
                }
                case LightComponent::LightType::Directional:
                {
                    m_LightData.Lights[idx].PositionType = glm::vec4(0.0f, 0.0f, 0.0f, typeFloat);
                    m_LightData.Lights[idx].ColorInner   = glm::vec4(light.Color * light.Intensity, 0.0f);
                    m_LightData.Lights[idx].DirectionOuter = glm::vec4(direction, 0.0f);

                    glm::mat4 lightProjection = glm::ortho(-100.0f, 100.0f, -100.0f, 100.0f, 1.0f, 1000.0f);
                    glm::vec3 up = glm::abs(direction.y) > 0.999f ? glm::vec3(0.0f, 0.0f, 1.0f) : glm::vec3(0.0f, 1.0f, 0.0f);
                    glm::vec3 shadowCameraPos = transform.translation - (direction * 300.0f);
                    
                    glm::mat4 lightViewMat = glm::lookAt(transform.translation, transform.translation + direction, glm::vec3(0.0f, 1.0f, 0.0f));
                    m_LightData.Lights[idx].LightSpaceMatrix = lightProjection * lightViewMat;
                    
                    if (light.CastShadows) {
                        shadowIndex = m_DirSpotShadowCasters.size();
                        m_DirSpotShadowCasters.push_back(m_LightData.Lights[idx].LightSpaceMatrix);
                    }
                    break;
                }
                case LightComponent::LightType::Cone:
                {
                    m_LightData.Lights[idx].PositionType = glm::vec4(transform.translation, typeFloat);
                    m_LightData.Lights[idx].ColorInner   = glm::vec4(light.Color * light.Intensity, light.InnerCutOff);
                    m_LightData.Lights[idx].DirectionOuter = glm::vec4(direction, light.OuterCutOff);
                    
                    float fov = glm::acos(light.OuterCutOff) * 2.0f;
                    glm::mat4 lightProjection = glm::perspective(fov, 1.0f, 0.1f, 100.0f);
                    glm::mat4 lightViewMat = glm::lookAt(transform.translation, transform.translation + direction, glm::vec3(0.0f, 1.0f, 0.0f));
                    m_LightData.Lights[idx].LightSpaceMatrix = lightProjection * lightViewMat;
                    
                    if (light.CastShadows) {
                        shadowIndex = m_DirSpotShadowCasters.size();
                        m_DirSpotShadowCasters.push_back(m_LightData.Lights[idx].LightSpaceMatrix);
                    }
                    break;
                }
            }
            m_LightData.Lights[idx].Params = glm::ivec4(shadowIndex, 0, 0, 0); // Injecting layer index
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
            //if (!IsAABBInFrustum(cameraFrustum, mesh->GetAABB(), transform)) continue;

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
