#pragma once

#include "Material.h"

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

struct Sphere2
{
	vec3 center;
	float radius;
	Material material;

	bool operator==(const Sphere2& other) const
	{
		return center == other.center && radius == other.radius && material == other.material;
	}
};