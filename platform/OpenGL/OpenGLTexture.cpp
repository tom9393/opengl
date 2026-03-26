#include "OpenGLTexture.h"
#include "../../pch.h"

namespace Engine
{

    OpenGLTexture2D::OpenGLTexture2D(const std::filesystem::path &path) : m_Path(path)
    {
        int width, height, channels;
        m_RawData = stbi_load(path.c_str(), &width, &height, &channels, 0);
        m_Width = width;
        m_Height = height;
        m_Channels = channels;
    };

    OpenGLTexture2D::~OpenGLTexture2D()
    {
        glDeleteTextures(1, &m_RendererID);
    };

    void OpenGLTexture2D::Initialize()
    {
        glGenTextures(1, &m_RendererID);

        GLenum format = 0;

        if (m_Channels == 4) format = GL_RGBA;
        else if (m_Channels == 3) format = GL_RGB;
        else if (m_Channels == 1) format = GL_RED;

        glBindTexture(GL_TEXTURE_2D, m_RendererID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, m_Width, m_Height, 0, format, GL_UNSIGNED_BYTE, m_RawData);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(m_RawData);
    }

    void OpenGLTexture2D::Bind(unsigned int slot)
    {
        if (!m_RendererID)
            Initialize();

        glActiveTexture(GL_TEXTURE0 + slot);
        glBindTexture(GL_TEXTURE_2D, m_RendererID);
    };

    unsigned int OpenGLTexture2D::GetID() const
    {
        return m_RendererID;
    };

    OpenGLBindlessTexture::OpenGLBindlessTexture()
    {
    };

    void OpenGLBindlessTexture::Bind() const
    {
        glMakeTextureHandleResidentARB(m_RendererID);
    };

    void OpenGLBindlessTexture::Unbind() const
    {
        glMakeTextureHandleNonResidentARB(m_RendererID);
    };

    void OpenGLBindlessTexture::MakeHandle(const std::shared_ptr<OpenGLTexture2D> &texture)
    {
        m_RendererID = glGetTextureHandleARB(texture->GetID());
    };

};
