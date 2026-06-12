#pragma once

#include "pch.h"
#include "ParsedScene.h"

namespace Engine
{
    class Prefab;

    class PmSceneSerializer
    {
        public:
        static void Serialize(const Shared<Prefab>& prefab, const std::string& path);
        static std::optional<ParsedScene> Deserialize(const std::string& path);
    };
}
