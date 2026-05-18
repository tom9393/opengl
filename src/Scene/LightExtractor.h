#pragma once

#include "Scene.h"
#include "Render/RenderView.h"
#include "Render/Camera.h"

namespace Engine {

class LightExtractor {
public:
    static void Extract(Scene* scene, Frustum& frustum, RenderView& renderView)
    {
        auto view = scene->GetRegistry().view<LightComponent, TransformComponent>();
        for (auto& e : view)
        {
            if (renderView.LightData.LightCount >= 64) break;
            auto [light, transform] = view.get<LightComponent, TransformComponent>(e);

            int idx = renderView.LightData.LightCount;
            int shadowIndex = -1;
            float typeFloat = static_cast<float>(light.Type);
            glm::vec3 direction = transform.GetForward();

            switch (light.Type)
            {
                case LightComponent::LightType::Point:
                {
                    renderView.LightData.Lights[idx].PositionType = glm::vec4(transform.translation, typeFloat);
                    renderView.LightData.Lights[idx].ColorInner   = glm::vec4(light.Color * light.Intensity, 0.0f);
                    renderView.LightData.Lights[idx].DirectionOuter = glm::vec4(0.0f);
                    renderView.LightData.Lights[idx].LightSpaceMatrix = glm::mat4(1.0f);

                    if (light.CastShadows) {
                        shadowIndex = renderView.PointShadowCasters.size();
                        renderView.PointShadowCasters.push_back({ transform.translation, {} });
                    }
                    break;
                }
                case LightComponent::LightType::Directional:
                {
                    renderView.LightData.Lights[idx].PositionType = glm::vec4(0.0f, 0.0f, 0.0f, typeFloat);
                    renderView.LightData.Lights[idx].ColorInner   = glm::vec4(light.Color * light.Intensity, 0.0f);
                    renderView.LightData.Lights[idx].DirectionOuter = glm::vec4(direction, 0.0f);

                    float orthoRange = 40.0f;
                    glm::mat4 lightProjection = glm::ortho(-orthoRange, orthoRange, -orthoRange, orthoRange, 1.0f, 400.0f);
                    glm::mat4 lightViewMat = glm::lookAt(transform.translation, transform.translation + direction, glm::vec3(0.0f, 1.0f, 0.0f));
                    renderView.LightData.Lights[idx].LightSpaceMatrix = lightProjection * lightViewMat;

                    if (light.CastShadows) {
                        shadowIndex = renderView.DirectionalShadowCasters.size();
                        renderView.DirectionalShadowCasters.push_back({ renderView.LightData.Lights[idx].LightSpaceMatrix, {} });
                    }
                    break;
                }
                case LightComponent::LightType::Cone:
                {
                    renderView.LightData.Lights[idx].PositionType = glm::vec4(transform.translation, typeFloat);
                    renderView.LightData.Lights[idx].ColorInner   = glm::vec4(light.Color * light.Intensity, light.InnerCutOff);
                    renderView.LightData.Lights[idx].DirectionOuter = glm::vec4(direction, light.OuterCutOff);

                    float fov = glm::acos(light.OuterCutOff) * 2.0f;
                    glm::mat4 lightProjection = glm::perspective(fov, 1.0f, 0.1f, 100.0f);
                    glm::mat4 lightViewMat = glm::lookAt(transform.translation, transform.translation + direction, glm::vec3(0.0f, 1.0f, 0.0f));
                    renderView.LightData.Lights[idx].LightSpaceMatrix = lightProjection * lightViewMat;

                    if (light.CastShadows) {
                        shadowIndex = renderView.DirectionalShadowCasters.size();
                        renderView.DirectionalShadowCasters.push_back({ renderView.LightData.Lights[idx].LightSpaceMatrix, {} });
                    }
                    break;
                }
            }
            renderView.LightData.Lights[idx].Params = glm::ivec4(shadowIndex, 0, 0, 0);
            renderView.LightData.LightCount++;
        }
    }
};

}
