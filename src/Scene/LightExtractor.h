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

            switch (light.Type)
            {
                case LightComponent::LightType::Point:       shadowIndex = ExtractPoint(light, transform, idx, renderView);       break;
                case LightComponent::LightType::Directional: shadowIndex = ExtractDirectional(light, transform, idx, renderView); break;
                case LightComponent::LightType::Cone:        shadowIndex = ExtractCone(light, transform, idx, renderView);        break;
            }

            renderView.LightData.Lights[idx].Params = glm::ivec4(shadowIndex, 0, 0, 0);
            renderView.LightData.LightCount++;
        }

        // Cone shadow indices are local (0-based within ConeShadowCasters). Offset them
        // by the number of directional casters so both share a single texture array in order.
        int dirCount = (int)renderView.DirectionalShadowCasters.size();
        if (dirCount > 0) {
            for (int i = 0; i < renderView.LightData.LightCount; i++) {
                GPULight& l = renderView.LightData.Lights[i];
                if ((int)l.PositionType.w == (int)LightComponent::LightType::Cone && l.Params.x >= 0)
                    l.Params.x += dirCount;
            }
        }
    }

private:
    static int ExtractPoint(const LightComponent& light, const TransformComponent& transform, int idx, RenderView& renderView)
    {
        float typeFloat = static_cast<float>(light.Type);
        renderView.LightData.Lights[idx].PositionType    = glm::vec4(transform.translation, typeFloat);
        renderView.LightData.Lights[idx].ColorInner      = glm::vec4(light.Color * light.Intensity, 0.0f);
        renderView.LightData.Lights[idx].DirectionOuter  = glm::vec4(0.0f);
        renderView.LightData.Lights[idx].LightSpaceMatrix = glm::mat4(1.0f);

        if (!light.CastShadows) return -1;
        int shadowIndex = renderView.PointShadowCasters.size();
        renderView.PointShadowCasters.push_back({ transform.translation, {} });
        return shadowIndex;
    }

    static int ExtractDirectional(const LightComponent& light, const TransformComponent& transform, int idx, RenderView& renderView)
    {
        float typeFloat = static_cast<float>(light.Type);
        glm::vec3 direction = transform.GetForward();

        float orthoRange = 40.0f;
        glm::mat4 lightProjection = glm::ortho(-orthoRange, orthoRange, -orthoRange, orthoRange, 1.0f, 400.0f);
        glm::mat4 lightViewMat = glm::lookAt(transform.translation, transform.translation + direction, glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 lightSpaceMatrix = lightProjection * lightViewMat;

        renderView.LightData.Lights[idx].PositionType    = glm::vec4(0.0f, 0.0f, 0.0f, typeFloat);
        renderView.LightData.Lights[idx].ColorInner      = glm::vec4(light.Color * light.Intensity, 0.0f);
        renderView.LightData.Lights[idx].DirectionOuter  = glm::vec4(direction, 0.0f);
        renderView.LightData.Lights[idx].LightSpaceMatrix = lightSpaceMatrix;

        if (!light.CastShadows) return -1;
        int shadowIndex = renderView.DirectionalShadowCasters.size();
        renderView.DirectionalShadowCasters.push_back({ lightSpaceMatrix, {} });
        return shadowIndex;
    }

    static int ExtractCone(const LightComponent& light, const TransformComponent& transform, int idx, RenderView& renderView)
    {
        float typeFloat = static_cast<float>(light.Type);
        glm::vec3 direction = transform.GetForward();

        float fov = glm::acos(light.OuterCutOff) * 2.0f;
        glm::mat4 lightProjection = glm::perspective(fov, 1.0f, 0.1f, 100.0f);
        glm::mat4 lightViewMat = glm::lookAt(transform.translation, transform.translation + direction, glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 lightSpaceMatrix = lightProjection * lightViewMat;

        renderView.LightData.Lights[idx].PositionType    = glm::vec4(transform.translation, typeFloat);
        renderView.LightData.Lights[idx].ColorInner      = glm::vec4(light.Color * light.Intensity, light.InnerCutOff);
        renderView.LightData.Lights[idx].DirectionOuter  = glm::vec4(direction, light.OuterCutOff);
        renderView.LightData.Lights[idx].LightSpaceMatrix = lightSpaceMatrix;

        if (!light.CastShadows) return -1;
        int shadowIndex = renderView.ConeShadowCasters.size();
        renderView.ConeShadowCasters.push_back({ lightSpaceMatrix, {} });
        return shadowIndex;
    }
};

}
