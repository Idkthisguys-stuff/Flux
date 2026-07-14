#pragma once

#include <glm/glm.hpp>
#include <string>

namespace Flux {
    struct Material {
        unsigned int albedoMap = 0;
        std::string albedoPath;
        unsigned int normalMap = 0;
        std::string normalPath;
        unsigned int metallicMap = 0;
        std::string metallicPath;
        unsigned int roughnessMap = 0;
        std::string roughnessPath;
        unsigned int aoMap = 0;
        std::string aoPath;

        glm::vec3 baseColor = glm::vec3(1.0f); 
        float metallic = 0.0f;
        float roughness = 0.5f;
    };
}