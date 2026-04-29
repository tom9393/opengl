#pragma once

#include "Render/RenderGraph.h"

namespace Engine {

    class Preview {

        public:
        virtual ~Preview() = default;
        virtual uint32_t GetPreview() = 0;
        virtual uint32_t UpdatePreview(Shared<Asset> &asset) = 0;

        RenderGraph& GetRenderGraph() { return m_RenderGraph; }

        protected:
        RenderGraph m_RenderGraph{256, 256};
        Shared<DynamicBufferPool> m_InstancePool;

    };

}