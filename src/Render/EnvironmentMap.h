#pragma once

#include "pch.h"
#include "RenderTexture.h"

namespace Engine
{

    // Lives on the Scene, snapshotted into RenderView each frame
    struct PostProcessSettings
    {
        bool  BloomEnabled        = true;
        float BloomIntensity      = 0.04f; // Blend weight of bloom into the scene
        float BloomThreshold      = 1.0f;  // Luminance above which pixels bloom
        float Exposure            = 1.0f;
        float VignetteStrength    = 0.0f;
        float ChromaticAberration = 0.0f;
    };

    // Snapshot of the environment's GL texture handles consumed by render passes for one frame
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

    class EnvironmentMap
    {
    public:
        // .hdr file
        static Shared<EnvironmentMap> Load(const std::string& hdrPath);

        unsigned int GetSkybox() const { return m_EnvironmentCubemap ? m_EnvironmentCubemap->GetID() : 0; }
        unsigned int GetIrradiance() const { return m_IrradianceMap ? m_IrradianceMap->GetID() : 0; }
        unsigned int GetPrefilter() const { return m_PrefilterMap ? m_PrefilterMap->GetID() : 0; }
        unsigned int GetPrefilterMips() const { return m_PrefilterMips; }
        const std::string& GetPath() const { return m_Path; }

        static unsigned int GetBRDFLUT();
        static unsigned int GetFallbackCubemap();

        static void Shutdown();

    private:
        std::string           m_Path;
        Shared<RenderTexture> m_EnvironmentCubemap;
        Shared<RenderTexture> m_IrradianceMap;
        Shared<RenderTexture> m_PrefilterMap;
        unsigned int          m_PrefilterMips = 5;
    };

}
