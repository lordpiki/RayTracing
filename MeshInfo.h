#pragma once

#include "Material.h"

struct Triangle {
    vec3 posA, posB, posC;
    vec3 normalA, normalB, normalC;


};

struct MeshInfo {
    unsigned int triangleCount;
    unsigned int materialIndex;
    vec3 boundsMin;
    vec3 boundsMax;
    Material material;

    MeshInfo() : triangleCount(0), materialIndex(0), boundsMin(0), boundsMax(0) {}

    void addTriangle(const Triangle& triangle) {
		boundsMin = min(boundsMin, min(triangle.posA, min(triangle.posB, triangle.posC)));
		boundsMax = max(boundsMax, max(triangle.posA, max(triangle.posB, triangle.posC)));
		triangleCount++;
	}

    void addTriangleVec(const vector<Triangle>& triangles)
    {
		for (const Triangle& triangle : triangles) {
			addTriangle(triangle);
		}
	}
};
