#pragma once

#include "Scene.h"
#include "../Render/RenderGraph.h"
#include "../Render/RenderContext.h"
#include "../Utils/MathTools.h"
#include <glm/glm.hpp>

namespace Engine {

    class Material;
    class Mesh;

    struct GPULight {
        glm::vec4 PositionType;   // xyz = position, w = type (0=Point, 1=Directional, 2=Cone)
        glm::vec4 ColorInner;     // xyz = color, w = inner cutoff
        glm::vec4 DirectionOuter; // xyz = direction, w = outer cutoff
        glm::mat4 LightSpaceMatrix;
    };

    struct SceneLightData {
        GPULight Lights[64];
        int LightCount = 0;
    };

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
        Scene* m_Scene;
        RenderItems m_RenderItems;
        unsigned int m_ViewportWidth = 1024;
        unsigned int m_ViewportHeight = 1024;
        RenderGraph m_RenderGraph;
        const uint32_t m_MaxInstances = 10000;
        Shared<ShaderStorageBuffer> m_InstanceSSBO;
        std::vector<glm::mat4> m_Transforms;

        SceneLightData m_LightData;

        glm::mat4 m_DirLightSpaceMatrix = glm::mat4(1.0f);
        glm::vec3 m_PointLightPos = glm::vec3(0.0f);
        float m_PointLightFarPlane;
    };

}
