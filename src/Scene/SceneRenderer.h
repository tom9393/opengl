#pragma once

#include "Scene/Scene.h"
#include "MathTools.h"
#include <glm/glm.hpp>

namespace Engine {

    class SceneRenderer {
    public:
        SceneRenderer(Scene* scene);
        ~SceneRenderer() = default;

        void SetScene(Scene* scene) { m_Scene = scene; }
        
        void RenderEditor(const glm::mat4& viewProjection, const glm::vec3& cameraPosition);
        void RenderRuntime();

    private:
        void FlushDrawCalls(const glm::mat4& viewProjection, const glm::vec3& cameraPos);

    private:
        Scene* m_Scene;
    };

}
