#include "PhysicsDebugDrawer.h"
#include "Renderer.h"
#include <iostream>

namespace Engine {

    void PhysicsDebugDrawer::drawLine(const btVector3& from, const btVector3& to, const btVector3& color) 
    {
        // Convert Bullet vectors to GLM and send to your Line Renderer
        Renderer::DrawLine(
            { from.x(), from.y(), from.z() }, 
            { to.x(), to.y(), to.z() }, 
            { color.x(), color.y(), color.z(), 1.0f }
        );
    }

    void PhysicsDebugDrawer::reportErrorWarning(const char* warningString) 
    {
        std::cout << "[Physics] " << warningString << std::endl; 
    }

}