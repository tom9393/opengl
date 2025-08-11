#pragma once

#include "../../lib/entt.hpp"
#include "../TimeStep.h"
#include "Components.h"

class Scene
{
    public:
    Scene();
    ~Scene();

    void OnUpdate(TimeStep ts);

    entt::entity CreateEntity();
    entt::registry &GetRegistry() { return m_Registry; }

    private:
    entt::registry m_Registry;
};
