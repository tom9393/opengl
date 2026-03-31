#pragma once

#include "Scene.h"
#include "../Utils/MathTools.h"
#include <glm/glm.hpp>

namespace Engine {

    struct DrawCommand
    {
        Shared<Mesh> TargetMesh;
        glm::mat4 Transform;
        uintptr_t SortKey;
    };

    class SceneRenderer {
    public:
        SceneRenderer(Scene* scene);
        ~SceneRenderer() = default;

        void SetScene(Scene* scene) { m_Scene = scene; }
        
        void RenderEditor(const glm::mat4& viewProjection, const glm::vec3& cameraPosition);
        void RenderRuntime();

        void OnViewportResize(unsigned int width, unsigned int height);

    private:
        void FlushDrawCalls(const glm::mat4& viewProjection, const glm::vec3& cameraPos);

    private:
        Scene* m_Scene;
        std::vector<DrawCommand> m_OpaqueDrawList;
        unsigned int m_ViewportWidth;
        unsigned int m_ViewportHeight;
    };

}
