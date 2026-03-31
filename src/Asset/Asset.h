#pragma once

#include "../Core/UUID.h"

namespace Engine {

    enum class AssetType
    {
        None = 0,
        Texture2D,
        Texture3D,
        Scene,
        Model
    };

    class Asset
    {
        public:
        UUID GetHandle() { return m_Handle; };
        AssetType GetType() { return m_Type; };

        protected:
        bool m_Loaded = false;
        UUID m_Handle;
        AssetType m_Type;
    };
}
