#pragma once
#include "Texture.h"

namespace Engine {
    class DefaultTextures {
    public:
        static Shared<Texture2D> GetWhiteTexture() {
            static Shared<Texture2D> whiteTexture = nullptr;
            
            if (!whiteTexture) {
                unsigned char data[4] = { 255, 255, 255, 255 };
                whiteTexture = Texture2D::Create(data, 1, 1);
            }
            
            return whiteTexture;
        }

        static Shared<Texture2D> GetBlackTexture() {
            static Shared<Texture2D> blackTexture = nullptr;
            if (!blackTexture) {
                unsigned char data[4] = { 0, 0, 0, 255 };
                blackTexture = Texture2D::Create(data, 1, 1);
            }
            return blackTexture;
        }

        static Shared<Texture2D> GetNormalTexture() {
            static Shared<Texture2D> normalTexture = nullptr;
            
            if (!normalTexture) {
                unsigned char data[4] = { 255, 128, 128, 255 };
                normalTexture = Texture2D::Create(data, 1, 1);
            }
            
            return normalTexture;
        }

        // CRITICAL: Call this before shutting down your OpenGL context!
        static void Shutdown() {
            // By resetting the shared pointers, we force the destructors 
            // to run while the GL context is still alive.
            GetWhiteTexture().reset();
            GetBlackTexture().reset();
            GetNormalTexture().reset();
        }
    };
}