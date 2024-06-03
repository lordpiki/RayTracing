#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <iostream>
#include "shader.h"
#include <vector>

using std::vector;
using glm::vec3;
using glm::vec4;

struct Material
{
    vec3 color;
	float emission_strength;
	vec3 emmision_color;
	float reflection_strength;
};

struct Sphere
{
    vec3 center;
    float radius;
	Material material;
};

// Function to render the scene using the shader program
void renderScene(GLuint shaderProgram, int width, int height, GLuint sphereBuffer, int numSpheres)
{
    glUseProgram(shaderProgram);

    // Set the uniform variables
    glUniform1i(glGetUniformLocation(shaderProgram, "width"), width);
    glUniform1i(glGetUniformLocation(shaderProgram, "height"), height);
    glUniform1i(glGetUniformLocation(shaderProgram, "numSpheres"), numSpheres);

    // Bind the sphere buffer
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, sphereBuffer);

    // Draw a full-screen quad
    glBegin(GL_TRIANGLES);
    glVertex2f(-1.0f, -1.0f);
    glVertex2f(1.0f, -1.0f);
    glVertex2f(1.0f, 1.0f);
    glVertex2f(-1.0f, -1.0f);
    glVertex2f(1.0f, 1.0f);
    glVertex2f(-1.0f, 1.0f);
    glEnd();

    glUseProgram(0);
}

int main() {
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    // Create a GLFW window
    GLFWwindow* window = glfwCreateWindow(1200, 900, "Ray Tracing - FPS: ", NULL, NULL);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    // Make the OpenGL context current
    glfwMakeContextCurrent(window);

    // Initialize GLEW
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        return -1;
    }

    // Set the viewport
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);

    std::cout << "Sphere size: " << sizeof(Sphere) << std::endl;

	Sphere sun = { vec3(50.0f, -101.0f, -105.0f), 100.0f, vec3(0.0f, 0.0f, 1.0f), 5.0f, vec3(1.0f, 1.0f, 1.0f),  0.5f};

    // Create a vector for the spheres
    vector<Sphere> spheres = {
        {vec3(0.0f, 0.0f, -3.0f), 1.0f, vec3(1.0f, 0.0f, 0.0f), 0.0f, vec3(0, 0, 0),  0.0f},
		{vec3(-1.5f, 0.0f, -2.0f), 0.5f, vec3(1.0f, 1.0f, 0.0f), 0.0f, vec3(0, 0, 0),  0.0f},
		{vec3(-2.0f, 10.5f, -4.0f), 10.0f, vec3(0.128f, 0.0f, 0.128f), 0.0f, vec3(0, 0, 0),  0.0f},
    };

	spheres.push_back(sun);

    // Create and fill the sphere buffer
    GLuint sphereBuffer;
    glGenBuffers(1, &sphereBuffer);
    glBindBuffer(GL_UNIFORM_BUFFER, sphereBuffer);
    glBufferData(GL_UNIFORM_BUFFER, spheres.size() * sizeof(Sphere), spheres.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    // Compile and link shaders
    GLuint shaderProgram = createShaderProgram("vertex_shader.glsl", "fragment_shader.glsl");

    // Variables for FPS calculation
    double lastTime = glfwGetTime();
    int nbFrames = 0;

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        // Measure the time
        double currentTime = glfwGetTime();
        nbFrames++;

        // If one second has passed, update the window title with the FPS
        if (currentTime - lastTime >= 1.0) {
            int fps = double(nbFrames) / (currentTime - lastTime);
            std::string title = "Ray Tracing - FPS: " + std::to_string(fps);
            glfwSetWindowTitle(window, title.c_str());
            nbFrames = 0;
            lastTime = currentTime;
        }

        // Render the scene
        renderScene(shaderProgram, width, height, sphereBuffer, spheres.size());

        // Swap buffers
        glfwSwapBuffers(window);

        // Poll for events
        glfwPollEvents();
    }

    // Cleanup
    glDeleteProgram(shaderProgram);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
