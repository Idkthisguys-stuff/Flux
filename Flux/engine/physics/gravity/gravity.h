#pragma once

#include "mechanics/Scenenode.h"
#include "glm/glm.hpp"

namespace Flux {
    class Gravity {
        public:
            static glm::vec3 CalculateVelocity(glm::vec3 currentVelocity, float deltaTime) {
                const glm::vec3 gravAccel = glm::vec3(0, -9.81f, 0);
                const float termVelo = -50.0f;

                glm::vec3 newVelo = currentVelocity + (gravAccel * deltaTime);

                if (newVelo.y < termVelo) {
                    newVelo.y = termVelo;
                }

                return newVelo;
            }
    };
}