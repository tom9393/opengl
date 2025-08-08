#pragma once

#include "Batch.h"
#include "Buffer.h"
#include "RenderCommand.h"
#include "RendererAPI.h"
#include "Shader.h"
#include "Camera.h"
#include "Texture.h"
#include "mesh.h"
#include "../platform/OpenGL/OpenGLTexture.h"

struct RenderStorage
{
    ShaderLibrary* shaderLibrary;
    std::shared_ptr<SSBOBuffer> matrixBuffer;
    std::shared_ptr<SSBOBuffer> textureBuffer;
    std::vector<GLuint64> textures;
    std::vector<std::shared_ptr<OpenGLBindlessTexture>> openglBindlessTextures;
    Batch* batch = nullptr;
    bool set = false;
};

class Renderer
{
    public:

    static void Init();
    static void OnWindowResize(unsigned int width, unsigned int height);

    static void ToggleWireframe();

    static void BeginScene(std::shared_ptr<Camera> camera);
    static void EndScene();

    static void SetTexture(const std::shared_ptr<Texture> &texture, const std::unordered_map<std::string, std::shared_ptr<Texture2D>> &textures);
    static void Submit(const std::shared_ptr<Mesh> &mesh, const glm::mat4 &transform);

    static RendererAPI::API Get() { return RendererAPI::GetAPI(); }

    private:
    struct SceneData
    {
        glm::mat4 ViewProjectionMatrix;
    };

    static SceneData* m_SceneData;
    static RenderStorage* m_Data;

};
