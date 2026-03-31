#include "../../pch.h"
#include "../../src/Asset/Texture.h"

namespace Engine
{

    class OpenGLTexture2D : public Texture2D
    {
        public:
        OpenGLTexture2D(const std::filesystem::path &path);
        virtual ~OpenGLTexture2D();

        unsigned int GetWidth() const override { return m_Width; }
        unsigned int GetHeight() const override { return m_Height; }

        unsigned int GetID() const override;
        std::filesystem::path &GetPath() override { return m_Path; }

        void Bind(unsigned int slot) override;
        void Initialize();

        private:
        std::filesystem::path m_Path;
        unsigned char* m_RawData;
        unsigned int m_DataFormat, m_InternalFormat;
        unsigned int m_Width, m_Height, m_Channels;
        unsigned int m_RendererID;
    };

    class OpenGLBindlessTexture : public BindlessTexture
    {
        public:
        OpenGLBindlessTexture();

        void MakeHandle(const std::shared_ptr<OpenGLTexture2D> &texture);
        GLuint64 GetHandle() const { return m_RendererID; }
        void Bind() const override;
        void Unbind() const override;

        private:
        GLuint64 m_RendererID;
        std::shared_ptr<OpenGLTexture2D> m_Texture;
    };

};
