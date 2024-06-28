#pragma once
#include <GL/glew.h>
#include "ShaderProgram.h"
#include "camera.h"
#include <vector>
#include "Sphere.h"

class RayTracer {
public:
    RayTracer(int width, int height);
    ~RayTracer();

    bool initialize();
    void render(int frameNum, float randomSeed);
    void updateCamera(const Camera& camera);
    void updateSpheres(const std::vector<Sphere>& spheres);
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

    int m_maxDepth;
    int m_raysPerPixel;
};