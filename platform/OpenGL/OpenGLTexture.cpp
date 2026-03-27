#include "OpenGLTexture.h"
#include "../../pch.h"
#include "ktx.h"

namespace Engine
{

    OpenGLTexture2D::OpenGLTexture2D(const std::filesystem::path &path) : m_Path(path)
    {
        std::string ext = std::filesystem::path(path).extension().string();

        // -------------------------------------------------------------
        // FAST PATH: Load Pre-Cooked GPU Textures
        // -------------------------------------------------------------
        if (ext == ".ktx" || ext == ".ktx2")
        {
            ktxTexture* kTexture;
            KTX_error_code result;

            // Read the file directly into KTX memory
            result = ktxTexture_CreateFromNamedFile(path.c_str(), KTX_TEXTURE_CREATE_NO_FLAGS, &kTexture);
            if (result != KTX_SUCCESS) {
                std::cout << "Failed to load KTX texture: " << path << std::endl;
                return;
            }
            if (kTexture->classId == ktxTexture2_c)
            {
                ktxTexture2* k2Texture = (ktxTexture2*)kTexture;
                
                if (ktxTexture2_NeedsTranscoding(k2Texture))
                {
                    // Transcode to BC7 (Standard for modern Desktop Linux/Windows GPUs)
                    // Note: If you port to Android later, you would use KTX_TTF_ASTC_4x4_RGBA here instead!
                    result = ktxTexture2_TranscodeBasis(k2Texture, KTX_TTF_BC7_RGBA, 0);
                    
                    if (result != KTX_SUCCESS) {
                        std::cout << "Failed to transcode KTX2 texture: " << path << " Error: " << result << std::endl;
                        ktxTexture_Destroy(kTexture);
                        return;
                    }
                }
            }

            m_Width = kTexture->baseWidth;
            m_Height = kTexture->baseHeight;

            // This single function creates the GL handle, uploads the compressed bytes, 
            // and handles all the mipmaps automatically!
            GLenum target;
            GLenum glerror;
            result = ktxTexture_GLUpload(kTexture, &m_RendererID, &target, &glerror);

            if (result != KTX_SUCCESS) {
                std::cout << "Failed to upload KTX to GPU: " << path << std::endl;
                std::cout << result << std::endl;
            }

            // Cleanup the CPU RAM (The texture is now safely on the GPU)
            ktxTexture_Destroy(kTexture);
            
            m_InternalFormat = 0; // KTX handles this, but you can query it if needed
            m_DataFormat = 0;
            m_Loaded = true;
            return; // We are done!
        }

        // -------------------------------------------------------------
        // SLOW PATH: Legacy stb_image loading for PNG/JPG
        // -------------------------------------------------------------
        int width, height, channels;
        stbi_set_flip_vertically_on_load(1);
        m_RawData = stbi_load(path.c_str(), &width, &height, &channels, 0);

        m_Width = width;
        m_Height = height;
        m_Channels = channels;

        GLenum internalFormat = 0, dataFormat = 0;
        if (channels == 4)
        {
            internalFormat = GL_RGBA8;
            dataFormat = GL_RGBA;
        }
        else if (channels == 3)
        {
            internalFormat = GL_RGB8;
            dataFormat = GL_RGB;
        }
        else if (channels == 1)
        {
            internalFormat = GL_RED;
            dataFormat = GL_RED;
        }

        m_InternalFormat = internalFormat;
        m_DataFormat = dataFormat;
        m_Loaded = true;
    };

    OpenGLTexture2D::~OpenGLTexture2D()
    {
        glDeleteTextures(1, &m_RendererID);
    };

    void OpenGLTexture2D::Initialize()
    {
        glGenTextures(1, &m_RendererID);

        glBindTexture(GL_TEXTURE_2D, m_RendererID);
        glTexImage2D(GL_TEXTURE_2D, 0, m_DataFormat, m_Width, m_Height, 0, m_DataFormat, GL_UNSIGNED_BYTE, m_RawData);
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
