#pragma once

#include "../pch.h"
#include "Preview.h"
#include "Asset/Prefab.h"
#include "Scene/Entity.h"
#include "../Render/Passes/ShadowPass2D.h"
#include "../Render/Passes/ShadowPass3D.h"
#include "../Render/Passes/GeometryPass.h"
#include "../Render/Passes/ScenePass.h"

namespace Engine {

    class PrefabPreview : public Preview
    {
        public:
        PrefabPreview()
        {
            m_InstancePool = MakeShared<DynamicBufferPool>(1024 * 1024); // 1MB
            m_RenderGraph.AddPass<ShadowPass2D>("ShadowPass2D");
            m_RenderGraph.AddPass<ShadowPass3D>("PointShadowPass");
            m_RenderGraph.AddPass<GeometryPass>("GeometryPass");
            m_RenderGraph.AddPass<ScenePass>("ScenePass");
        }

        uint32_t GetPreview() override
        {
            return m_RenderGraph.GetPassColorAttachmentRendererID("ScenePass", 0);
        }

        void SetAsset(Shared<Asset> &asset) override
        {
            auto prefab = std::static_pointer_cast<Prefab>(asset);
            m_Scene = Scene::Copy(prefab);
            Entity light = m_Scene->CreateEntity("ModelPreviewLight");
            LightComponent& lightComponent = light.AddComponent<LightComponent>();
            lightComponent.Type = LightComponent::LightType::Directional;
            lightComponent.Intensity = 25.0f;
            light.AddComponent<TransformComponent>(glm::vec3(-2, 3, 10));
        }
    };

}