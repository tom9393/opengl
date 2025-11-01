#include "../../pch.h"
#include "../../include/Texture.h"

namespace Engine
{

    class OpenGLTexture2D : public Texture2D
    {
        public:
        OpenGLTexture2D(const std::string &path, TextureType type);
        virtual ~OpenGLTexture2D();

        unsigned int GetWidth() const override { return m_Width; }
        unsigned int GetHeight() const override { return m_Height; }

        unsigned int GetID() const override;
        std::string &GetPath() override { return m_Path; }
        TextureType GetType() const override { return m_Type; }

        void Bind(unsigned int slot) const override;

        private:
        std::string m_Path;
        TextureType m_Type;
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
