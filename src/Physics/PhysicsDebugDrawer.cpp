#include "PhysicsDebugDrawer.h"
#include "pch.h"

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

    void PhysicsDebugDrawer::drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color)
    {
        // You can leave this empty for now if you don't need to see contact points
    }

    void PhysicsDebugDrawer::draw3dText(const btVector3& location, const char* textString)
    {
        // Leave empty for now
    }

    void PhysicsDebugDrawer::setDebugMode(int debugMode)
    {
        m_DebugMode = debugMode;
    }

    int PhysicsDebugDrawer::getDebugMode() const
    {
        return m_DebugMode;
    }

    void PhysicsDebugDrawer::reportErrorWarning(const char* warningString) 
    {
        std::cout << "[Physics] " << warningString << std::endl; 
    }

}