#pragma once
#include "Texture.h"

namespace Engine {
    class DefaultTextures {
    public:
        static Shared<Texture2D> GetWhiteTexture() {
            static Shared<Texture2D> whiteTexture = nullptr;
            
            if (!whiteTexture) {
                uint32_t whiteTextureData = 0xffffffff; 
                whiteTexture = Texture2D::Create(&whiteTextureData, 1, 1);
            }
            
            return whiteTexture;
        }

        static Shared<Texture2D> GetNormalTexture() {
            static Shared<Texture2D> normalTexture = nullptr;
            
            if (!normalTexture) {
                uint32_t flatNormalData = 0xffff8080; // A B G R
                normalTexture = Texture2D::Create(&flatNormalData, 1, 1);
            }
            
            return normalTexture;
        }

        // CRITICAL: Call this before shutting down your OpenGL context!
        static void Shutdown() {
            // By resetting the shared pointers, we force the destructors 
            // to run while the GL context is still alive.
            GetWhiteTexture().reset();
            GetNormalTexture().reset();
        }
    };
}