#include "pch.h"
#include "Asset/Texture.h"
#include "Asset/TextureImporter.h"

namespace Engine
{

    class OpenGLTexture2D : public Texture2D
    {
        public:
        OpenGLTexture2D(const TextureLoadData &data);
        virtual ~OpenGLTexture2D();

        void Copy(uint32_t rendererID, uint32_t width, uint32_t height) override;
        void ResizeCopy(uint32_t rendererID, uint32_t width, uint32_t height) override;

        unsigned int GetWidth() const override { return m_Width; }
        unsigned int GetHeight() const override { return m_Height; }

        unsigned int GetID() const override;
        std::filesystem::path &GetPath() override { return m_Path; }

        void Bind(unsigned int slot) override;

        private:
        std::filesystem::path m_Path;
        unsigned int m_DataFormat, m_InternalFormat;
        unsigned int m_Width, m_Height;
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
