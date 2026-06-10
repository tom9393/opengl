#include "Render/Shader.h"
#include "Render/Renderer.h"
#include "OpenGL/OpenGLShader.h"

namespace Engine
{

    Shared<Shader> Shader::Create(const std::string &name)
    {
        switch (Renderer::Get()) {
            case RendererAPI::API::None: return nullptr;
            case RendererAPI::API::OpenGL: return std::make_shared<OpenGLShader>(name);
            }
        return nullptr;
    };

    std::unordered_map<std::string, Shared<Shader>> ShaderLibrary::s_Shaders;

    void ShaderLibrary::Init()
    {
        Shared<Shader> geometryShader = Shader::Create("GBufferShader");
        const std::string gbuffervert = "/home/tom/Coding/c++/opengl/shaders/main.vert";
        const std::string gbufferfrag = "/home/tom/Coding/c++/opengl/shaders/gbuffer.frag";
        geometryShader->AddShaders({gbuffervert, gbufferfrag});
        Add(geometryShader);

        Shared<Shader> pointshadowShader = Shader::Create("PointShadowShader");
        const std::string psvert = "/home/tom/Coding/c++/opengl/shaders/shadow_point.vert";
        const std::string psgeom = "/home/tom/Coding/c++/opengl/shaders/shadow_point.geom";
        const std::string psfrag = "/home/tom/Coding/c++/opengl/shaders/shadow_point.frag";
        pointshadowShader->AddShaders({psvert, psgeom, psfrag});
        Add(pointshadowShader);

        Shared<Shader> shadowShader = Shader::Create("ShadowShader");
        const std::string svert = "/home/tom/Coding/c++/opengl/shaders/shadow.vert";
        const std::string sfrag = "/home/tom/Coding/c++/opengl/shaders/shadow.frag";
        shadowShader->AddShaders({svert, sfrag});
        Add(shadowShader);

        Shared<Shader> deferredlightingShader = Shader::Create("DeferredLighting");
        const std::string dlvert = "/home/tom/Coding/c++/opengl/shaders/deferred_lighting.vert";
        const std::string dlfrag = "/home/tom/Coding/c++/opengl/shaders/deferred_lighting.frag";
        deferredlightingShader->AddShaders({dlvert, dlfrag});
        Add(deferredlightingShader);

        const std::string shaderDir = "/home/tom/Coding/c++/opengl/shaders/";
        const std::string fullscreenVert = dlvert; // shared NDC fullscreen-quad vertex stage
        const std::string cubeVert = shaderDir + "cubemap.vert";

        // --- Image-based lighting bake shaders ---
        Shared<Shader> equirectShader = Shader::Create("EquirectToCube");
        equirectShader->AddShaders({cubeVert, shaderDir + "equirect_to_cube.frag"});
        Add(equirectShader);

        Shared<Shader> irradianceShader = Shader::Create("IrradianceConvolution");
        irradianceShader->AddShaders({cubeVert, shaderDir + "irradiance_convolution.frag"});
        Add(irradianceShader);

        Shared<Shader> prefilterShader = Shader::Create("Prefilter");
        prefilterShader->AddShaders({cubeVert, shaderDir + "prefilter.frag"});
        Add(prefilterShader);

        Shared<Shader> brdfShader = Shader::Create("BRDF");
        brdfShader->AddShaders({fullscreenVert, shaderDir + "brdf.frag"});
        Add(brdfShader);

        // --- Post processing shaders ---
        Shared<Shader> bloomDownShader = Shader::Create("BloomDownsample");
        bloomDownShader->AddShaders({fullscreenVert, shaderDir + "bloom_down.frag"});
        Add(bloomDownShader);

        Shared<Shader> bloomUpShader = Shader::Create("BloomUpsample");
        bloomUpShader->AddShaders({fullscreenVert, shaderDir + "bloom_up.frag"});
        Add(bloomUpShader);

        Shared<Shader> postShader = Shader::Create("PostProcess");
        postShader->AddShaders({fullscreenVert, shaderDir + "post.frag"});
        Add(postShader);

        // --- Particles ---
        Shared<Shader> particleShader = Shader::Create("Particle");
        particleShader->AddShaders({shaderDir + "particle.vert", shaderDir + "particle.frag"});
        Add(particleShader);

        Shared<Shader> copyShader = Shader::Create("FullscreenCopy");
        copyShader->AddShaders({fullscreenVert, shaderDir + "copy.frag"});
        Add(copyShader);
    }

    void ShaderLibrary::Shutdown()
    {

    }

    void ShaderLibrary::Add(const Shared<Shader> &shader)
    {
        auto &name = shader->GetName();
        s_Shaders[name] = shader;
    };

    Shared<Shader> ShaderLibrary::Get(const std::string &name)
    {
        return s_Shaders[name];
    };

    void ShaderLibrary::AddToShader(const std::string &shader, const std::initializer_list<std::string> &paths)
    {
        s_Shaders[shader]->AddShaders(paths);
    };

    ShaderLibrary::~ShaderLibrary()
    {
    }

};
