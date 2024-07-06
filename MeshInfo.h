#pragma once

#include "Material.h"
#include <glm/glm.hpp>
#include <vector>

using std::vector;

struct Triangle {
    vec3 posA, posB, posC;
    vec3 normalA, normalB, normalC;
};

struct MeshInfo {
    vec3 boundsMin;
    float triangleCount;
    Material material;
    vec3 boundsMax;
    float triangleIndex;

    bool operator==(const MeshInfo& other) const
	{
		return boundsMin == other.boundsMin && triangleCount == other.triangleCount && material == other.material && boundsMax == other.boundsMax && triangleIndex == other.triangleIndex;
	}

    void addTriangle(const Triangle& triangle)
	{
		triangleCount++;
		boundsMin = triangle.posA;
		boundsMax = triangle.posA;

		boundsMin = glm::min(boundsMin, triangle.posB);
		boundsMin = glm::min(boundsMin, triangle.posC);

		boundsMax = glm::max(boundsMax, triangle.posB);
		boundsMax = glm::max(boundsMax, triangle.posC);

	}

	void addTriangleVec(const vector<Triangle>& triangles)
	{
		for (const Triangle& triangle : triangles)
		{
			addTriangle(triangle);
		}
	}
};

