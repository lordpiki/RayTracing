#pragma once
#include <iostream>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// using glm
using std::vector;
using glm::vec3;
using glm::vec4;


struct Material
{
    vec4 color;
    vec3 emission;
    float emissionStrength;
    bool operator==(const Material& other) const
    {
        return color == other.color && emission == other.emission && emissionStrength == other.emissionStrength;
    }
};
