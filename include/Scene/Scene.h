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

    Entity CreateEntity();

    private:
    entt::registry m_Registry;
    friend class Entity;
};
