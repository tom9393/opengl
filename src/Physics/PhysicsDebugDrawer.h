#pragma once

#include <LinearMath/btIDebugDraw.h>
#include "../Render/Renderer.h"

namespace Engine {
    class PhysicsDebugDrawer : public btIDebugDraw {
    public:
        void drawLine(const btVector3& from, const btVector3& to, const btVector3& color) override;

        // Bullet needs these but we can keep them empty for now
        void drawContactPoint(const btVector3&, const btVector3&, btScalar, int, const btVector3&) override {}
        void reportErrorWarning(const char* warningString) override;
        void draw3dText(const btVector3&, const char*) override {}
        
        void setDebugMode(int debugMode) override { m_DebugMode = debugMode; }
        int getDebugMode() const override { return m_DebugMode; }

    private:
        int m_DebugMode = DBG_DrawWireframe;
    };
}
