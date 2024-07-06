#pragma once

#include "Material.h"

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
};

