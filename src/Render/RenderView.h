#pragma once

#include "DrawCommand.h"
#include "ShadowCasters.h"
#include "Light.h"
#include "EnvironmentMap.h"

namespace Engine {

    enum class RenderFlags : uint32_t
    {
        None      = 0,
        Shadows   = 1 << 0,
        Wireframe = 1 << 1,
    };

    inline RenderFlags operator|(RenderFlags a, RenderFlags b) { return RenderFlags(uint32_t(a) | uint32_t(b)); }
    inline RenderFlags operator&(RenderFlags a, RenderFlags b) { return RenderFlags(uint32_t(a) & uint32_t(b)); }
    inline bool HasFlag(RenderFlags flags, RenderFlags flag)   { return (flags & flag) == flag; }

    class Texture2D;

    // GPU layout for one billboard; must match the std430 SSBO in particle.vert
    struct ParticleInstance
    {
        glm::vec3 Position;
        float Size;
        glm::vec4 Color;
    };

    // One draw per emitter: instances share a texture and blend mode
    struct ParticleBatch
    {
        Shared<Texture2D> Texture; // null = procedural soft circle
        bool Additive = true;
        glm::vec3 EmitterPosition = glm::vec3(0.0f); // batch sort key
        std::vector<ParticleInstance> Instances;
    };

    struct CameraData
    {
        glm::mat4 ViewProjection;
        glm::vec3 CameraPosition;
        float padding;
    };

    struct RenderView {
        CameraData Camera;
        float NearPlane = 0.1f;
        float FarPlane  = 1000.0f;
        glm::uvec2 ViewportSize = { 1280, 720 };
        RenderFlags Flags = RenderFlags::Shadows;
        std::vector<DrawCommand> OpaqueItems;
        std::vector<DrawCommand> TransparentItems;
        std::vector<ParticleBatch> ParticleBatches;
        std::vector<ShadowCasterDirectional> DirectionalShadowCasters;
        std::vector<ShadowCasterCone> ConeShadowCasters;
        std::vector<ShadowCasterPoint> PointShadowCasters;
        SceneLightData LightData;
        EnvironmentRenderData Environment;
        PostProcessSettings Post;
    };

}