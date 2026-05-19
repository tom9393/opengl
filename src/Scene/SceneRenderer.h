#pragma once

#include "pch.h"
#include "Scene.h"
#include "Render/RenderGraph.h"
#include "Render/RenderContext.h"
#include "Render/DynamicBufferPool.h"
#include "Utils/MathTools.h"

namespace Engine {

    class Material;
    class Mesh;

    class SceneRenderer {
    public:
        SceneRenderer(Scene* scene);
        ~SceneRenderer() = default;

        void SetScene(Scene* scene) { m_Scene = scene; }

        void RenderEditor(const glm::mat4& viewProjection, const glm::vec3& cameraPosition, float nearPlane, float farPlane);
        void RenderRuntime();

        void OnViewportResize(unsigned int width, unsigned int height);

        RenderGraph &GetRenderGraph() { return m_RenderGraph; }

    private:
        Scene* m_Scene;
        unsigned int m_ViewportWidth = 1024;
        unsigned int m_ViewportHeight = 1024;
        RenderGraph m_RenderGraph;

        Shared<DynamicBufferPool> m_InstancePool;
    };

}
