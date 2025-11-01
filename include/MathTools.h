#pragma once

namespace Engine
{

    class MathTools
    {
        public:
        static bool DecomposeMatrix(const glm::mat4 &transform, glm::vec3 &translation, glm::vec3 &rotation, glm::vec3 &scale);
    };

};
