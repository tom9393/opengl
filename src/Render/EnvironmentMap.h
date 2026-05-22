#pragma once

#include "pch.h"

namespace Engine
{

    // Editable post-processing parameters. Lives on the Scene (serialized) and is
    // snapshotted into the RenderView each frame for the PostProcessPass.
    struct PostProcessSettings
    {
        bool  BloomEnabled        = true;
        float BloomIntensity      = 0.04f; // blend weight of bloom into the scene
        float BloomThreshold      = 1.0f;  // luminance above which pixels bloom
        float Exposure            = 1.0f;
        float VignetteStrength    = 0.0f;
        float ChromaticAberration = 0.0f;
    };

    // Snapshot of the environment's GL texture handles consumed by render passes
    // for one frame. Plain ids so render-side headers stay free of EnvironmentMap.
    struct EnvironmentRenderData
    {
        unsigned int Skybox        = 0;
        unsigned int Irradiance    = 0;
        unsigned int Prefilter     = 0;
        unsigned int BRDFLUT       = 0;
        unsigned int PrefilterMips = 1;
        float        Intensity     = 1.0f;
        bool         Valid         = false;
    };

    // Builds and owns the image-based-lighting cubemaps for a single
    // equirectangular .hdr map: the environment cubemap (skybox), a diffuse
    // irradiance map, and a roughness-mipped specular prefilter map. The
    // env-independent BRDF integration LUT is built once and shared statically.
    class EnvironmentMap
    {
    public:
        ~EnvironmentMap();

        // Load and bake an equirectangular .hdr file. Returns null on failure.
        // Must be called on the thread owning the GL context.
        static Shared<EnvironmentMap> Load(const std::string& hdrPath);

        unsigned int GetSkybox() const { return m_EnvironmentCubemap; }
        unsigned int GetIrradiance() const { return m_IrradianceMap; }
        unsigned int GetPrefilter() const { return m_PrefilterMap; }
        unsigned int GetPrefilterMips() const { return m_PrefilterMips; }
        const std::string& GetPath() const { return m_Path; }

        // Env-independent BRDF integration LUT, built lazily and shared.
        static unsigned int GetBRDFLUT();
        // 1x1 cubemap bound to IBL samplers when no environment is set, so the
        // samplers always reference a complete texture.
        static unsigned int GetFallbackCubemap();

    private:
        std::string  m_Path;
        unsigned int m_EnvironmentCubemap = 0;
        unsigned int m_IrradianceMap      = 0;
        unsigned int m_PrefilterMap       = 0;
        unsigned int m_PrefilterMips      = 5;
    };

}
