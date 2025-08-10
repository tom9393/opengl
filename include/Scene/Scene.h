#pragma once

#include "../../lib/entt.hpp"

class Scene
{
    public:
    Scene();
    ~Scene();

    private:
    entt::registry m_Registry;
};
