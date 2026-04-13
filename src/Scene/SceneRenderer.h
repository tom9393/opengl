#pragma once

#include "Scene.h"
#include "../Render/RenderGraph.h"
#include "../Render/RenderContext.h"
#include "../Utils/MathTools.h"
#include <glm/glm.hpp>

namespace Engine {

    class Material;
    class Mesh;

    struct DrawCommand
    {
        Shared<Mesh> TargetMesh;
        Shared<Material> TargetMaterial;
        glm::mat4 Transform;
    };

    struct RenderItems
    {
        std::vector<DrawCommand> Opaque;
        std::vector<DrawCommand> Shadows;
        std::vector<DrawCommand> Transparent;
    };

    class SceneRenderer {
    public:
        SceneRenderer(Scene* scene);
        ~SceneRenderer() = default;

        void SetScene(Scene* scene) { m_Scene = scene; }

        void PrepareScene(const RenderContext &context);
        
        void RenderEditor(const glm::mat4& viewProjection, const glm::vec3& cameraPosition);
        void RenderRuntime();

        void OnViewportResize(unsigned int width, unsigned int height);

        RenderGraph &GetRenderGraph() { return m_RenderGraph; }

    private:
        void SubmitBatchedMeshes(std::vector<DrawCommand> &commands);

    private:
        Scene* m_Scene;
        RenderItems m_RenderItems;
        unsigned int m_ViewportWidth;
        unsigned int m_ViewportHeight;
        RenderGraph m_RenderGraph;
        const uint32_t m_MaxInstances = 10000;
        Shared<ShaderStorageBuffer> m_InstanceSSBO;
        std::vector<glm::mat4> m_Transforms;
    };

}
