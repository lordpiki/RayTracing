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
};
