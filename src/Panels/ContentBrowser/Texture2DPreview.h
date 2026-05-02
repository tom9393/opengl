#pragma once

#include "pch.h"
#include "Preview.h"

namespace Engine {

    class Texture2DPreview : public Preview
    {
        public:
        void SetAsset(Shared<Asset> &asset) override
        {
            TextureLoadData data;
            data.Height = 256;
            data.Width = 256;
            data.clamp = true;
            m_Texture = Texture2D::Create(data);

            Shared<Texture2D> texture = std::static_pointer_cast<Texture2D>(asset);

            m_Texture->ResizeCopy(texture->GetID(), texture->GetWidth(), texture->GetHeight());
        }

        void UpdatePreview(RenderGraph &graph, DynamicBufferPool *pool) override
        {
        }
    };

}