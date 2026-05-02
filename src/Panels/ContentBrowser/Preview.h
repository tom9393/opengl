#pragma once

#include "Render/RenderGraph.h"
#include "Render/Renderer.h"

namespace Engine {

    class Scene;

    class Preview {

        public:
        virtual ~Preview() = default;
        virtual void SetAsset(Shared<Asset> &asset) = 0;
        virtual void UpdatePreview(RenderGraph &graph, DynamicBufferPool *pool) = 0;

        uint32_t GetCachedTextureID() const { return m_Texture ? m_Texture->GetID() : 0; }

        protected:
        Shared<Texture2D> m_Texture;
    };

}