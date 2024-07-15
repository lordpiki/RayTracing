#pragma once
#include <GL/glew.h>
#include "ShaderProgram.h"
#include "camera.h"
#include <vector>
#include "Sphere.h"
#include "MeshInfo.h"

using std::vector;

class RayTracer {
public:
    RayTracer(int width, int height);
    ~RayTracer();

    bool initialize();
    void render(int frameNum, float randomSeed);
    void updateCamera(const Camera& camera);
    void setMaxDepth(int depth);
    void setRaysPerPixel(int rays);

    void updateMeshes(const vector<MeshInfo>& meshes);
    void updateSpheres(const vector<Sphere>& spheres);
    void updateTriangles(const vector<Triangle>& triangles);

    void setDimentions(int width, int height);

private:
    bool loadComputeShader();
    bool loadRenderShader();

    int m_width;
    int m_height;
    ShaderProgram m_computeShader;
    ShaderProgram m_renderShader;

    GLuint m_imgAccumulationTexture;


    GLuint m_outputTexture;
    GLuint m_quadVAO;
    GLuint m_quadVBO;

    GLuint m_sphereBuffer;
    GLuint m_meshBuffer;
    GLuint m_triangleBuffer;

    int m_maxDepth;
    int m_raysPerPixel;
};