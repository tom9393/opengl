#include "EnvironmentMap.h"

#include "Shader.h"
#include "Renderer.h"

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

        unsigned int s_CaptureFBO = 0;
        unsigned int s_CaptureRBO = 0;
        unsigned int s_CubeVAO    = 0;
        unsigned int s_CubeVBO    = 0;
        unsigned int s_BRDFLUT    = 0;
        unsigned int s_Fallback   = 0;

        const glm::mat4 kCaptureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
        const glm::mat4 kCaptureViews[6] = {
            glm::lookAt(glm::vec3(0.0f), glm::vec3( 1, 0, 0), glm::vec3(0, -1,  0)),
            glm::lookAt(glm::vec3(0.0f), glm::vec3(-1, 0, 0), glm::vec3(0, -1,  0)),
            glm::lookAt(glm::vec3(0.0f), glm::vec3( 0, 1, 0), glm::vec3(0,  0,  1)),
            glm::lookAt(glm::vec3(0.0f), glm::vec3( 0,-1, 0), glm::vec3(0,  0, -1)),
            glm::lookAt(glm::vec3(0.0f), glm::vec3( 0, 0, 1), glm::vec3(0, -1,  0)),
            glm::lookAt(glm::vec3(0.0f), glm::vec3( 0, 0,-1), glm::vec3(0, -1,  0)),
        };

        void EnsureCube()
        {
            if (s_CubeVAO) return;

            float vertices[] = {
                // back face
                -1, -1, -1,  1,  1, -1,  1, -1, -1,  1,  1, -1, -1, -1, -1, -1,  1, -1,
                // front face
                -1, -1,  1,  1, -1,  1,  1,  1,  1,  1,  1,  1, -1,  1,  1, -1, -1,  1,
                // left face
                -1,  1,  1, -1,  1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  1, -1,  1,  1,
                // right face
                 1,  1,  1,  1, -1, -1,  1,  1, -1,  1, -1, -1,  1,  1,  1,  1, -1,  1,
                // bottom face
                -1, -1, -1,  1, -1, -1,  1, -1,  1,  1, -1,  1, -1, -1,  1, -1, -1, -1,
                // top face
                -1,  1, -1,  1,  1,  1,  1,  1, -1,  1,  1,  1, -1,  1, -1, -1,  1,  1,
            };

            glGenVertexArrays(1, &s_CubeVAO);
            glGenBuffers(1, &s_CubeVBO);
            glBindVertexArray(s_CubeVAO);
            glBindBuffer(GL_ARRAY_BUFFER, s_CubeVBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
            glBindVertexArray(0);
        }

        void RenderCube()
        {
            glBindVertexArray(s_CubeVAO);
            glDrawArrays(GL_TRIANGLES, 0, 36);
            glBindVertexArray(0);
        }

        void EnsureCapture()
        {
            if (s_CaptureFBO) return;
            glGenFramebuffers(1, &s_CaptureFBO);
            glGenRenderbuffers(1, &s_CaptureRBO);
        }

        // Allocate a cubemap; mipmapped when requested (needed for prefilter sampling).
        unsigned int CreateCubemap(unsigned int size, bool mipmapped)
        {
            unsigned int id;
            glGenTextures(1, &id);
            glBindTexture(GL_TEXTURE_CUBE_MAP, id);
            for (unsigned int i = 0; i < 6; ++i)
                glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, size, size, 0, GL_RGB, GL_FLOAT, nullptr);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER,
                            mipmapped ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
            if (mipmapped)
                glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
            return id;
        }
    }

    EnvironmentMap::~EnvironmentMap()
    {
        if (m_EnvironmentCubemap) glDeleteTextures(1, &m_EnvironmentCubemap);
        if (m_IrradianceMap)      glDeleteTextures(1, &m_IrradianceMap);
        if (m_PrefilterMap)       glDeleteTextures(1, &m_PrefilterMap);
    }

    unsigned int EnvironmentMap::GetFallbackCubemap()
    {
        if (s_Fallback) return s_Fallback;
        glGenTextures(1, &s_Fallback);
        glBindTexture(GL_TEXTURE_CUBE_MAP, s_Fallback);
        float black[3] = { 0.0f, 0.0f, 0.0f };
        for (unsigned int i = 0; i < 6; ++i)
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 1, 1, 0, GL_RGB, GL_FLOAT, black);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        return s_Fallback;
    }

    unsigned int EnvironmentMap::GetBRDFLUT()
    {
        if (s_BRDFLUT) return s_BRDFLUT;

        EnsureCapture();

        glGenTextures(1, &s_BRDFLUT);
        glBindTexture(GL_TEXTURE_2D, s_BRDFLUT);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, BRDF_SIZE, BRDF_SIZE, 0, GL_RG, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glBindFramebuffer(GL_FRAMEBUFFER, s_CaptureFBO);
        glBindRenderbuffer(GL_RENDERBUFFER, s_CaptureRBO);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, BRDF_SIZE, BRDF_SIZE);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, s_CaptureRBO);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, s_BRDFLUT, 0);

        glViewport(0, 0, BRDF_SIZE, BRDF_SIZE);
        ShaderLibrary::Get("BRDF")->Bind();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        Renderer::DrawFullscreenQuad();

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        return s_BRDFLUT;
    }

    Shared<EnvironmentMap> EnvironmentMap::Load(const std::string& hdrPath)
    {
        stbi_set_flip_vertically_on_load(true);
        int width, height, channels;
        float* data = stbi_loadf(hdrPath.c_str(), &width, &height, &channels, 3);
        if (!data)
        {
            std::cerr << "EnvironmentMap: failed to load HDR '" << hdrPath << "'" << std::endl;
            return nullptr;
        }

        EnsureCube();
        EnsureCapture();
        GetBRDFLUT(); // build the shared LUT once

        // Upload the equirectangular source as a float texture.
        unsigned int hdrTexture;
        glGenTextures(1, &hdrTexture);
        glBindTexture(GL_TEXTURE_2D, hdrTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, data);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        stbi_image_free(data);

        // The bake renders a cube from the inside, so cull/depth must not discard it.
        GLboolean cullWasEnabled  = glIsEnabled(GL_CULL_FACE);
        GLboolean depthWasEnabled = glIsEnabled(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);
        glDisable(GL_DEPTH_TEST);

        auto result = MakeShared<EnvironmentMap>();
        result->m_Path = hdrPath;
        result->m_PrefilterMips = PREFILTER_MIPS;

        glBindFramebuffer(GL_FRAMEBUFFER, s_CaptureFBO);

        // --- 1. Equirectangular -> cubemap ---
        result->m_EnvironmentCubemap = CreateCubemap(ENV_SIZE, true);
        {
            glBindRenderbuffer(GL_RENDERBUFFER, s_CaptureRBO);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, ENV_SIZE, ENV_SIZE);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, s_CaptureRBO);

            Shared<Shader> shader = ShaderLibrary::Get("EquirectToCube");
            shader->Bind();
            shader->SetInt("u_EquirectangularMap", 0);
            shader->SetMat4("u_Projection", kCaptureProjection);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, hdrTexture);

            glViewport(0, 0, ENV_SIZE, ENV_SIZE);
            for (unsigned int i = 0; i < 6; ++i)
            {
                shader->SetMat4("u_View", kCaptureViews[i]);
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                       GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, result->m_EnvironmentCubemap, 0);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                RenderCube();
            }
            // Generate mips so the prefilter pass can sample lower-frequency levels.
            glBindTexture(GL_TEXTURE_CUBE_MAP, result->m_EnvironmentCubemap);
            glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
        }

        // --- 2. Diffuse irradiance convolution ---
        result->m_IrradianceMap = CreateCubemap(IRRADIANCE_SIZE, false);
        {
            glBindRenderbuffer(GL_RENDERBUFFER, s_CaptureRBO);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, IRRADIANCE_SIZE, IRRADIANCE_SIZE);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, s_CaptureRBO);

            Shared<Shader> shader = ShaderLibrary::Get("IrradianceConvolution");
            shader->Bind();
            shader->SetInt("u_EnvironmentMap", 0);
            shader->SetMat4("u_Projection", kCaptureProjection);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_CUBE_MAP, result->m_EnvironmentCubemap);

            glViewport(0, 0, IRRADIANCE_SIZE, IRRADIANCE_SIZE);
            for (unsigned int i = 0; i < 6; ++i)
            {
                shader->SetMat4("u_View", kCaptureViews[i]);
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                       GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, result->m_IrradianceMap, 0);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                RenderCube();
            }
        }

        // --- 3. Specular prefilter (roughness mip chain) ---
        result->m_PrefilterMap = CreateCubemap(PREFILTER_SIZE, true);
        {
            Shared<Shader> shader = ShaderLibrary::Get("Prefilter");
            shader->Bind();
            shader->SetInt("u_EnvironmentMap", 0);
            shader->SetMat4("u_Projection", kCaptureProjection);
            shader->SetFloat("u_Resolution", (float)ENV_SIZE);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_CUBE_MAP, result->m_EnvironmentCubemap);

            for (unsigned int mip = 0; mip < PREFILTER_MIPS; ++mip)
            {
                unsigned int mipSize = (unsigned int)(PREFILTER_SIZE * std::pow(0.5, mip));
                glBindRenderbuffer(GL_RENDERBUFFER, s_CaptureRBO);
                glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipSize, mipSize);
                glViewport(0, 0, mipSize, mipSize);

                float roughness = (float)mip / (float)(PREFILTER_MIPS - 1);
                shader->SetFloat("u_Roughness", roughness);
                for (unsigned int i = 0; i < 6; ++i)
                {
                    shader->SetMat4("u_View", kCaptureViews[i]);
                    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                           GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, result->m_PrefilterMap, mip);
                    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                    RenderCube();
                }
            }
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glDeleteTextures(1, &hdrTexture);

        if (cullWasEnabled)  glEnable(GL_CULL_FACE);
        if (depthWasEnabled) glEnable(GL_DEPTH_TEST);

        return result;
    }

}
