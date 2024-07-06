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
    void updateSpheres(const vector<Sphere>& spheres);
    void updateSpheres2(const vector<Sphere2>& spheres2);
    void updateMeshes(const vector<MeshInfo>& meshes);
    void setMaxDepth(int depth);
    void setRaysPerPixel(int rays);

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
    GLuint m_sphereBuffer2;
    GLuint m_meshBuffer;

    int m_maxDepth;
    int m_raysPerPixel;
};