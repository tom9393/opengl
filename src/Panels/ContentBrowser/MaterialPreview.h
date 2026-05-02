#pragma once

#include "../pch.h"
#include "Preview.h"
#include "Asset/Prefab.h"
#include "Scene/Entity.h"
#include "../Asset/PrimitiveBuilder.h"

namespace Engine {

    class MaterialPreview : public Preview
    {
        public:
        void SetAsset(Shared<Asset> &asset) override
        {
            m_Scene = MakeShared<Scene>();

            Entity light = m_Scene->CreateEntity("MaterialPreviewLight");
            LightComponent& lightComponent = light.AddComponent<LightComponent>();
            lightComponent.Type = LightComponent::LightType::Directional;
            lightComponent.Intensity = 25.0f;
            light.AddComponent<TransformComponent>(glm::vec3(-2, 3, 10));

            Entity entity = m_Scene->CreateEntity("MaterialPreviewSphere");
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
            entity.AddComponent<MaterialComponent>(asset->Handle);
        }
    };

}