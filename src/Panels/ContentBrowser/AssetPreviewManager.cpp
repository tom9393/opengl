#include "AssetPreviewManager.h"
#include "../Asset/AssetManager.h"
#include "../Scene/Scene.h"
#include "../Asset/Material.h"
#include "../Asset/PrimitiveBuilder.h"
#include "../Render/Renderer.h"
#include "../Render/RenderGraph.h"
#include "../Render/RenderCommand.h"
#include "../Render/SceneCamera.h"
#include "../Scene/SceneExtractor.h"
#include "../Render/Passes/ShadowPass2D.h"
#include "../Render/Passes/ShadowPass3D.h"
#include "../Render/Passes/GeometryPass.h"
#include "../Render/Passes/ScenePass.h"

namespace Engine {

    std::unordered_map<UUID, Shared<Texture2D>> AssetPreviewManager::s_Previews;
    RenderGraph AssetPreviewManager::s_RenderGraph(256, 256);
    Shared<DynamicBufferPool> AssetPreviewManager::s_InstancePool;
    Shared<Texture2D> AssetPreviewManager::s_DefaultFileIcon;
    Shared<Texture2D> AssetPreviewManager::s_DefaultFolderIcon;
    Shared<Texture2D> AssetPreviewManager::s_LuaIcon;

    void AssetPreviewManager::Init() {
		s_DefaultFileIcon = Texture2D::Create("../assets/icons/documentIcon.png");
		s_DefaultFolderIcon = Texture2D::Create("../assets/icons/folderIcon.png");
		s_LuaIcon = Texture2D::Create("../assets/icons/luaIcon.png");

        s_InstancePool = MakeShared<DynamicBufferPool>(10 * 1024 * 1024); // 10MB

        s_RenderGraph.AddPass<ShadowPass2D>("ShadowPass2D");
        s_RenderGraph.AddPass<ShadowPass3D>("PointShadowPass");
        s_RenderGraph.AddPass<GeometryPass>("GeometryPass");
        s_RenderGraph.AddPass<ScenePass>("ScenePass");
    }

    void AssetPreviewManager::Shutdown() {
        s_Previews.clear();
        s_DefaultFileIcon.reset();
        s_DefaultFolderIcon.reset();
    }

    uint32_t AssetPreviewManager::GetPreview(UUID assetHandle) {
        if (s_Previews.find(assetHandle) != s_Previews.end()) {
            return s_Previews[assetHandle]->GetID();
        }

        Shared<Texture2D> previewTexture = GeneratePreview(assetHandle);
        if (previewTexture) {
            s_Previews[assetHandle] = previewTexture;
            return previewTexture->GetID();
        }

        return s_DefaultFileIcon->GetID();
    }

    void AssetPreviewManager::UpdatePreview(UUID assetHandle) {
        Shared<Texture2D> previewTexture = GeneratePreview(assetHandle);
        if (previewTexture) {
            s_Previews[assetHandle] = previewTexture;
        }
    }

    Shared<Texture2D> AssetPreviewManager::GeneratePreview(UUID assetHandle) {
        const AssetMetadata& metadata = AssetManager::GetMetadata(assetHandle);
        
        switch (metadata.Type) {
            case AssetType::Texture2D: {
                return AssetManager::GetAsset<Texture2D>(assetHandle);
            }
            case AssetType::Model: {
                Shared<Scene> model = AssetManager::GetAsset<Scene>(assetHandle);
                return GenerateModelPreview(model);
            }
            case AssetType::Material: {
                Shared<Material> material = AssetManager::GetAsset<Material>(assetHandle);
                return GenerateMaterialPreview(material);
            }
            case AssetType::LuaScript: {
                return s_LuaIcon;
            }
            default:
                return s_DefaultFileIcon;
        }
    }

    Shared<Texture2D> AssetPreviewManager::GenerateModelPreview(Shared<Scene> &model) {
        if (!model) return nullptr;

        Shared<Scene> copied = Scene::Copy(model);

        Entity light = copied->CreateEntity("ModelPreviewLight");
        LightComponent& lightComponent = light.AddComponent<LightComponent>();
        lightComponent.Type = LightComponent::LightType::Directional;
        lightComponent.Intensity = 25.0f;
        light.AddComponent<TransformComponent>(glm::vec3(-2, 3, 10));

        // float time = (float)glfwGetTime();
        // float speed = 1.0f; 
        // float radius = 10.0f;

        float camX = -2;
        float camZ = 10;
        // float camX = sin(time * speed) * radius;
        // float camZ = cos(time * speed) * radius;
        // float camY = sin(time * speed) * radius;
        float camY = 3.0f;

        glm::vec3 position = glm::vec3(camX, camY, camZ); 
        glm::vec3 target = glm::vec3(0.0f, 0.0f, 0.0f);
        glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

        glm::mat4 viewMatrix = glm::lookAt(position, target, up);

        RenderContext ctx;
        ctx.ViewProjection = glm::perspective(glm::radians(60.0f), 1.0f, 0.1f, 1000.0f) * viewMatrix;
        ctx.CameraPosition = position;
        ctx.InstancePool = s_InstancePool.get();

        RenderView view = SceneExtractor::Extract(copied.get(), ctx);

        Renderer::BeginScene();
        s_RenderGraph.Execute(ctx, view);
        Renderer::EndScene();

        uint32_t attachment = s_RenderGraph.GetPassColorAttachmentRendererID("ScenePass", 0);

        return Texture2D::Create(attachment, 256, 256);
    }

    Shared<Texture2D> AssetPreviewManager::GenerateMaterialPreview(Shared<Material> &material) {
        if (!material) return nullptr;

        Shared<Scene> materialPreviewScene = MakeShared<Scene>();

        Entity light = materialPreviewScene->CreateEntity("MaterialPreviewLight");
        LightComponent& lightComponent = light.AddComponent<LightComponent>();
        lightComponent.Type = LightComponent::LightType::Directional;
        lightComponent.Intensity = 25.0f;
        light.AddComponent<TransformComponent>(glm::vec3(-2, 3, 10));

        Entity entity = materialPreviewScene->CreateEntity("MaterialPreviewSphere");
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
        entity.AddComponent<MaterialComponent>(material->Handle);

        glm::vec3 position = glm::vec3(2.0f, 2.0f, 5.0f); 
        glm::vec3 target = glm::vec3(0.0f, 0.0f, 0.0f);
        glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

        glm::mat4 viewMatrix = glm::lookAt(position, target, up);

        RenderContext ctx;
        ctx.ViewProjection = glm::perspective(glm::radians(60.0f), 1.0f, 0.1f, 1000.0f) * viewMatrix;
        ctx.CameraPosition = position;
        ctx.InstancePool = s_InstancePool.get();

        RenderView view = SceneExtractor::Extract(materialPreviewScene.get(), ctx);

        Renderer::BeginScene();
        s_RenderGraph.Execute(ctx, view);
        Renderer::EndScene();

        uint32_t attachment = s_RenderGraph.GetPassColorAttachmentRendererID("ScenePass", 0);

        return Texture2D::Create(attachment, 256, 256);
    }
}