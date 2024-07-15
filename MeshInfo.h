#pragma once

#include "Material.h"
#include <glm/glm.hpp>
#include <vector>
#include <tuple>

using std::vector;

struct Triangle {
	vec3 posA;
	float padd1;
	vec3 posB;
	float padd2;
	vec3 posC;
	float padd3;


    //vec3 normalA, normalB, normalC;

	Triangle() : posA(0), posB(0), posC(0), padd1(0), padd2(0), padd3(0) {}
	Triangle(vec3 pos1, vec3 pos2, vec3 pos3) : posA(pos1), posB(pos2), posC(pos3), padd1(0), padd2(0), padd3(0) {}
	bool operator==(const Triangle& other) const
	{
		return posA == other.posA && posB == other.posB && posC == other.posC;
	}
};

struct MeshInfo {
    vec3 boundsMin;
    unsigned int triangleCount;
    Material material;
    vec3 boundsMax;
	unsigned int triangleIndex;

	MeshInfo() : boundsMin(0), triangleCount(0), material(), boundsMax(0), triangleIndex(0) {}

	void updateBoundingBox(const vector<Triangle>& triangles)
	{
		// Go over all triangles, and update the bounding box
		triangleCount = 0;
		addTriangleVec(triangles);
	}

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

	static vec3 rotatePoint(vec3& point, float radians, int axis)
	{
		vec3 newPoint;
		if (axis == 0)
		{
			newPoint.x = point.x;
			newPoint.y = point.y * cos(radians) - point.z * sin(radians);
			newPoint.z = point.y * sin(radians) + point.z * cos(radians);
		}
		else if (axis == 1)
		{
			newPoint.x = point.x * cos(radians) + point.z * sin(radians);
			newPoint.y = point.y;
			newPoint.z = -point.x * sin(radians) + point.z * cos(radians);
		}
		else if (axis == 2)
		{
			newPoint.x = point.x * cos(radians) - point.y * sin(radians);
			newPoint.y = point.x * sin(radians) + point.y * cos(radians);
			newPoint.z = point.z;
		}
		return newPoint;

	}

	void scaleMesh(vector<Triangle>& triangles, float scale)
	{
		// scaling the triangles
		for (Triangle& tri : triangles)
		{
			tri.posA *= scale;
			tri.posB *= scale;
			tri.posC *= scale;
		}

		// scaling the bounding box
		boundsMin *= scale;
		boundsMax *= scale;
	}

	// axis = 0 -> x, axis = 1 -> y, axis = 2 -> z
	void rotateMesh(vector<Triangle>& triangles, int degrees, int axis)
	{
		float radians = glm::radians((float)degrees);
		// rotating the triangles
		for (Triangle& tri : triangles)
		{
			tri.posA = rotatePoint(tri.posA, radians, axis);
			tri.posB = rotatePoint(tri.posB, radians, axis);
			tri.posC = rotatePoint(tri.posC, radians, axis);
		}

		std::cout << "bounding box before rotation: " << boundsMin.x << " " << boundsMin.y << " " << boundsMin.z << " " << boundsMax.x << " " << boundsMax.y << " " << boundsMax.z << std::endl;
		//addTriangleVec(triangles);

		// rotating the bounding box
		boundsMin = rotatePoint(boundsMin, radians, axis);
		boundsMax = rotatePoint(boundsMax, radians, axis);
		std::cout << "bounding box after rotation: " << boundsMin.x << " " << boundsMin.y << " " << boundsMin.z << " " << boundsMax.x << " " << boundsMax.y << " " << boundsMax.z << std::endl;
	}
	

	static std::tuple<vec3, vec3> getBoundingBox(const Triangle& tri)
	{
		vec3 boundsMin, boundsMax;

		boundsMin.x = std::min(tri.posA.x, std::min(tri.posB.x, tri.posC.x));
		boundsMin.y = std::min(tri.posA.y, std::min(tri.posB.y, tri.posC.y));
		boundsMin.z = std::min(tri.posA.z, std::min(tri.posB.z, tri.posC.z));

		boundsMax.x = std::max(tri.posA.x, std::max(tri.posB.x, tri.posC.x));
		boundsMax.y = std::max(tri.posA.y, std::max(tri.posB.y, tri.posC.y));
		boundsMax.z = std::max(tri.posA.z, std::max(tri.posB.z, tri.posC.z));

		return std::make_tuple(boundsMin, boundsMax);
	}

	static std::tuple<vec3, vec3> getNewBoundingBox(const vec3& oldBoundsMin, const vec3& oldBoundsMax, const vec3& newBoundsMin, const vec3& newBoundsMax)
	{
		vec3 boundsMin, boundsMax;

		boundsMin.x = std::min(oldBoundsMin.x, newBoundsMin.x);
		boundsMin.y = std::min(oldBoundsMin.y, newBoundsMin.y);
		boundsMin.z = std::min(oldBoundsMin.z, newBoundsMin.z);

		boundsMax.x = std::max(oldBoundsMax.x, newBoundsMax.x);
		boundsMax.y = std::max(oldBoundsMax.y, newBoundsMax.y);
		boundsMax.z = std::max(oldBoundsMax.z, newBoundsMax.z);

		return std::make_tuple(boundsMin, boundsMax);
	}
	

    void addTriangle(const Triangle& triangle)
	{
		vec3 triBoundsMin, triBoundsMax;
		std::tie(triBoundsMin, triBoundsMax) = getBoundingBox(triangle);

		if (triangleCount == 0)
		{
			this->boundsMin = triBoundsMin;
			this->boundsMax = triBoundsMax;
		}
		else
		{
			vec3 newBoundsMin, newBoundsMax;
			std::tie(newBoundsMin, newBoundsMax) = getNewBoundingBox(this->boundsMin, this->boundsMax, triBoundsMin, triBoundsMax);
			this->boundsMin = newBoundsMin;
			this->boundsMax = newBoundsMax;
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

