#pragma once

#include "../pch.h"
#include "Preview.h"
#include "Asset/Prefab.h"
#include "Scene/Entity.h"

namespace Engine {

    class PrefabPreview : public Preview
    {
        public:
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