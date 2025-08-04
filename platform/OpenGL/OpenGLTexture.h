#include "../../include/Texture.h"

class OpenGLTexture2D : public Texture2D
{
    public:
    OpenGLTexture2D(const std::string &path, TextureType type);
    virtual ~OpenGLTexture2D();

    unsigned int GetWidth() const override { return m_Width; }
    unsigned int GetHeight() const override { return m_Height; }

    unsigned int GetID() const override;
    TextureType GetType() const { return m_Type; }

    void Bind(unsigned int slot) const override;

    private:
    std::string m_Path;
    TextureType m_Type;
    unsigned int m_Width, m_Height;
    unsigned int m_RendererID;
};
