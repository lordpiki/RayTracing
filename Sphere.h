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
    bool operator==(const Material& other) const
	{
		return color == other.color && emission == other.emission && emissionStrength == other.emissionStrength;
	}
};

struct Sphere
{
    vec3 center;
    float radius;
    Material material;

    bool operator==(const Sphere& other) const
    {
        return center == other.center && radius == other.radius && material == other.material;
    }
};