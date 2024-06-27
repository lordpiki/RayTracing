#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// using glm
using glm::vec3;
using glm::vec4;


struct Material
{
    vec4 color;
    vec3 emission;
    float emissionStrength;
};

struct Sphere
{
    vec3 center;
    float radius;
    Material material;
};