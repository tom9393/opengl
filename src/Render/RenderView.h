#pragma once

#include "DrawCommand.h"
#include "ShadowCasters.h"
#include "Light.h"

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
        std::vector<ShadowCasterDirectional> DirectionalShadowCasters;
        std::vector<ShadowCasterCone> ConeShadowCasters;
        std::vector<ShadowCasterPoint> PointShadowCasters;
        SceneLightData LightData;
    };

}