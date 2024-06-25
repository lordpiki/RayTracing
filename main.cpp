// normal includes
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include "shader.h"
#include <vector>

// imgui includes
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <stdio.h>
#define GL_SILENCE_DEPRECATION

using std::string;
using std::vector;
using glm::vec3;
using glm::vec4;

using std::cout;
using std::endl;

struct Sphere
{
    vec3 center;
    float radius;
    vec3 color;
    float pad; // padding to align with GLSL struct
};

static void imgui_end_loop(GLFWwindow* window)
{
    ImGui::Render();
    int display_w, display_h;
    glfwGetFramebufferSize(window, &display_w, &display_h);
    //glViewport(0, 0, display_w, display_h);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glfwSwapBuffers(window);
    glClear(GL_COLOR_BUFFER_BIT);
}

static void imgui_start_loop()
{
    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

static void imgui_setup(GLFWwindow* window)
{
    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);
}

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

static GLFWwindow* glfw_setup(int width, int height)
{
    // Setup window
    if (!glfwInit())
        return NULL;

    // Create window with graphics context
    GLFWwindow* window = glfwCreateWindow(width, height, "RayTracing", NULL, NULL);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return NULL;
    }

    glfwMakeContextCurrent(window);
    // Initialize GLEW
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        return NULL;
    }

    glfwSwapInterval(1); // Enable vsync
    glfwSetErrorCallback(glfw_error_callback);
    glViewport(0, 0, width, height);

    return window;
}

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
int main()
{

    const float ratio = 16.0f / 9.0f;
    const int width = 1280;
    const int height = width / ratio;

    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    GLFWwindow* window = glfw_setup(width, height);


    // Setup Dear ImGui context
    imgui_setup(window);

    // Create a vector for the spheres
    vector<Sphere> spheres = {
        {vec3(0.0f, 0.0f, -3.0f), 1.0f, vec3(1.0f, 1.0f, 0.0f), 0.0f},
        {vec3(2.0f, 0.0f, -3.0f), 2.0f, vec3(0.0f, 1.0f, 1.0f), 0.0f},
        {vec3(-2.0f, 0.0f, -4.0f), 1.0f, vec3(1.0f, 0.0f, 1.0f), 0.0f}

    };

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
    while (!glfwWindowShouldClose(window))
    {
        // Poll for events
        glfwPollEvents();
		imgui_start_loop();
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


        // demo window
        {
            static float f = 0.0f;
            static int counter = 0;

            ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

            ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
            //ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
            //ImGui::Checkbox("Another Window", &show_another_window);

            //ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
            //ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

            //if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
            //    counter++;
            //ImGui::SameLine();
            //ImGui::Text("counter = %d", counter);

            //ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
            ImGui::End();
        }

        // Render the scene
        renderScene(shaderProgram, width, height, sphereBuffer, spheres.size());


        // Rendering
		imgui_end_loop(window);
    }

    // Cleanup
    glDeleteProgram(shaderProgram);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}