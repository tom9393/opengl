#include "EnvironmentMap.h"

#include "Shader.h"
#include "RenderCommand.h"
#include "Asset/Texture.h"
#include "Asset/TextureImporter.h"

#include <glm/gtc/matrix_transform.hpp>

namespace Engine
{

    namespace
    {
        constexpr unsigned int ENV_SIZE        = 512;
        constexpr unsigned int IRRADIANCE_SIZE = 32;
        constexpr unsigned int PREFILTER_SIZE  = 128;
        constexpr unsigned int PREFILTER_MIPS  = 5;
        constexpr unsigned int BRDF_SIZE       = 512;

        Shared<RenderTexture> s_BRDFLUT;
        Shared<RenderTexture> s_Fallback;

        const glm::mat4 kCaptureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
        const glm::mat4 kCaptureViews[6] = {
            glm::lookAt(glm::vec3(0.0f), glm::vec3( 1, 0, 0), glm::vec3(0, -1,  0)),
            glm::lookAt(glm::vec3(0.0f), glm::vec3(-1, 0, 0), glm::vec3(0, -1,  0)),
            glm::lookAt(glm::vec3(0.0f), glm::vec3( 0, 1, 0), glm::vec3(0,  0,  1)),
            glm::lookAt(glm::vec3(0.0f), glm::vec3( 0,-1, 0), glm::vec3(0,  0, -1)),
            glm::lookAt(glm::vec3(0.0f), glm::vec3( 0, 0, 1), glm::vec3(0, -1,  0)),
            glm::lookAt(glm::vec3(0.0f), glm::vec3( 0, 0,-1), glm::vec3(0, -1,  0)),
        };

        Shared<RenderTexture> CreateCubemap(unsigned int size, unsigned int mips)
        {
            RenderTextureSpecification spec;
            spec.width  = size;
            spec.height = size;
            spec.type   = RenderTextureType::Cubemap;
            spec.format = RenderTextureFormat::RGBA16F;
            spec.mips   = mips;
            return RenderTexture::Create(spec);
        }
    }

    unsigned int EnvironmentMap::GetFallbackCubemap()
    {
        if (s_Fallback) return s_Fallback->GetID();

        s_Fallback = CreateCubemap(1, 1);

        // Clear so that samplers never read NaN when no environment is set
        RenderCommand::SetClearColor({ 0.0f, 0.0f, 0.0f, 1.0f });
        for (unsigned int i = 0; i < 6; ++i)
        {
            s_Fallback->BindAsTarget(i, 0);
            RenderCommand::Clear();
        }
        s_Fallback->Unbind();
        return s_Fallback->GetID();
    }

    unsigned int EnvironmentMap::GetBRDFLUT()
    {
        if (s_BRDFLUT) return s_BRDFLUT->GetID();

        RenderTextureSpecification spec;
        spec.width  = BRDF_SIZE;
        spec.height = BRDF_SIZE;
        spec.type   = RenderTextureType::Texture2D;
        spec.format = RenderTextureFormat::RG16F;
        spec.mips   = 1;
        s_BRDFLUT = RenderTexture::Create(spec);

        s_BRDFLUT->BindAsTarget();
        RenderCommand::SetClearColor({ 0.0f, 0.0f, 0.0f, 1.0f });
        RenderCommand::Clear();
        ShaderLibrary::Get("BRDF")->Bind();
        RenderCommand::DrawFullscreenQuad();
        s_BRDFLUT->Unbind();

        return s_BRDFLUT->GetID();
    }

    void EnvironmentMap::Shutdown()
    {
        s_BRDFLUT.reset();
        s_Fallback.reset();
    }

    Shared<EnvironmentMap> EnvironmentMap::Load(const std::string& hdrPath)
    {
        Shared<Texture2D> equirect = TextureImporter::LoadHDR(hdrPath);
        if (!equirect)
        {
            std::cerr << "EnvironmentMap: failed to load HDR '" << hdrPath << "'" << std::endl;
            return nullptr;
        }

        // Ensure the shared LUT exists
        GetBRDFLUT();

        RenderCommand::SetFaceCulling(false);
        RenderCommand::SetDepthTest(false);

        auto result = MakeShared<EnvironmentMap>();
        result->m_Path = hdrPath;
        result->m_PrefilterMips = PREFILTER_MIPS;

        // --- 1. Equirectangular -> cubemap ---
        {
            unsigned int envMips = (unsigned int)std::floor(std::log2((float)ENV_SIZE)) + 1;
            result->m_EnvironmentCubemap = CreateCubemap(ENV_SIZE, envMips);

            Shared<Shader> shader = ShaderLibrary::Get("EquirectToCube");
            shader->Bind();
            shader->SetInt("u_EquirectangularMap", 0);
            shader->SetMat4("u_Projection", kCaptureProjection);
            equirect->Bind(0);

            for (unsigned int i = 0; i < 6; ++i)
            {
                shader->SetMat4("u_View", kCaptureViews[i]);
                result->m_EnvironmentCubemap->BindAsTarget(i, 0);
                RenderCommand::Clear();
                RenderCommand::DrawCube();
            }
            // Generate mips so the prefilter pass can sample lower-frequency levels.
            result->m_EnvironmentCubemap->GenerateMips();
        }

        // --- 2. Diffuse irradiance convolution ---
        {
            result->m_IrradianceMap = CreateCubemap(IRRADIANCE_SIZE, 1);

            Shared<Shader> shader = ShaderLibrary::Get("IrradianceConvolution");
            shader->Bind();
            shader->SetInt("u_EnvironmentMap", 0);
            shader->SetMat4("u_Projection", kCaptureProjection);
            result->m_EnvironmentCubemap->BindAsTexture(0);

            for (unsigned int i = 0; i < 6; ++i)
            {
                shader->SetMat4("u_View", kCaptureViews[i]);
                result->m_IrradianceMap->BindAsTarget(i, 0);
                RenderCommand::Clear();
                RenderCommand::DrawCube();
            }
        }

        // --- 3. Specular prefilter (roughness mip chain) ---
        {
            result->m_PrefilterMap = CreateCubemap(PREFILTER_SIZE, PREFILTER_MIPS);

            Shared<Shader> shader = ShaderLibrary::Get("Prefilter");
            shader->Bind();
            shader->SetInt("u_EnvironmentMap", 0);
            shader->SetMat4("u_Projection", kCaptureProjection);
            shader->SetFloat("u_Resolution", (float)ENV_SIZE);
            result->m_EnvironmentCubemap->BindAsTexture(0);

            for (unsigned int mip = 0; mip < PREFILTER_MIPS; ++mip)
            {
                float roughness = (float)mip / (float)(PREFILTER_MIPS - 1);
                shader->SetFloat("u_Roughness", roughness);
                for (unsigned int i = 0; i < 6; ++i)
                {
                    shader->SetMat4("u_View", kCaptureViews[i]);
                    result->m_PrefilterMap->BindAsTarget(i, mip);
                    RenderCommand::Clear();
                    RenderCommand::DrawCube();
                }
            }
        }

        result->m_PrefilterMap->Unbind();

        RenderCommand::SetDepthTest(true);

        return result;
    }

}
