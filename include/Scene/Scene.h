#pragma once

#include "../../lib/entt.hpp"
#include "../TimeStep.h"
#include "Components.h"

class Entity;

class Scene
{
    public:
    Scene();
    ~Scene();

    void OnUpdate(TimeStep ts);
    void OnViewportResize(unsigned int width, unsigned int height);

    Entity CreateEntity();

    private:
    entt::registry m_Registry;
    unsigned int m_ViewportWidth = 0;
    unsigned int m_ViewportHeight = 0;

    friend class Entity;
};
