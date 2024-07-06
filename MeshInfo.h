#pragma once

#include "Material.h"
#include <glm/glm.hpp>
#include <vector>
#include <tuple>

using std::vector;

struct Triangle {
    vec3 posA, posB, posC;
    //vec3 normalA, normalB, normalC;
};

struct MeshInfo {
    vec3 boundsMin;
    unsigned int triangleCount;
    Material material;
    vec3 boundsMax;
	unsigned int triangleIndex;

    bool operator==(const MeshInfo& other) const
	{
		return boundsMin == other.boundsMin && triangleCount == other.triangleCount && material == other.material && boundsMax == other.boundsMax && triangleIndex == other.triangleIndex;
	}

	bool inBoundingBox(const vec3& boundsMin, const vec3& boundsMax, const vec3& p)
	{
		return p.x >= boundsMin.x && p.x <= boundsMax.x &&
			p.y >= boundsMin.y && p.y <= boundsMax.y &&
			p.z >= boundsMin.z && p.z <= boundsMax.z;
	}

	std::tuple<vec3, vec3> getBoundingBox(const Triangle& triangle)
	{
		vec3 boundsMin;
		vec3 boundsMax;

		boundsMin.x = glm::min(triangle.posA.x, glm::min(triangle.posB.x, triangle.posC.x));
		boundsMin.y = glm::min(triangle.posA.y,glm::min(triangle.posB.y, triangle.posC.y));
		boundsMin.z = glm::min(triangle.posA.z,glm::min(triangle.posB.z, triangle.posC.z));

		boundsMax.x = glm::max(triangle.posA.x,glm::max(triangle.posB.x, triangle.posC.x));
		boundsMax.y = glm::max(triangle.posA.y,glm::max(triangle.posB.y, triangle.posC.y));
		boundsMax.z = glm::max(triangle.posA.z,glm::max(triangle.posB.z, triangle.posC.z));
			
		return std::make_tuple(boundsMin, boundsMax);
	}
	

    void addTriangle(const Triangle& triangle)
	{
		vec3 boundsMin, boundsMax;
		std::tie(boundsMin, boundsMax) = getBoundingBox(triangle);

		if (triangleCount == 0)
		{
			this->boundsMin = boundsMin;
			this->boundsMax = boundsMax;
		}
		else
		{
			this->boundsMin = glm::min(this->boundsMin, boundsMin);
			this->boundsMax = glm::max(this->boundsMax, boundsMax);
		}

		triangleCount++;
	}

	void addTriangleVec(const vector<Triangle>& triangles)
	{
		for (const Triangle& triangle : triangles)
		{
			addTriangle(triangle);
		}
	}
};

