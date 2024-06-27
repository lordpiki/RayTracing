// normal includes
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <tuple>
#include "shader.h"
#include <vector>

// imgui includes
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <stdio.h>
#define GL_SILENCE_DEPRECATION

// custom includes
#include "camera.h"

// using glm
using glm::vec3;
using glm::vec4;

// using std
using std::string;
using std::vector;
using std::tuple;
using std::cout;
using std::endl;
using std::tie;

struct Material
{
    vec4 color;
    vec3 emission;
    float emissionStrength;
};

struct Sphere
{
    vec3 center;
    float radius;
    Material material;
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

    //glfwSwapInterval(1); // Enable vsync
    glfwSetErrorCallback(glfw_error_callback);
    glViewport(0, 0, width, height);

    return window;
}

static int fps_counter(GLFWwindow* window, float& lastTime, int& nbFrames)
{
    double currentTime = glfwGetTime();
    nbFrames++;

    // If one second has passed, update the window title with the FPS
    if (currentTime - lastTime >= 1.0)
    //if (1)
    {
        int fps = double(nbFrames) / (currentTime - lastTime);
        std::string title = "Ray Tracing - FPS: " + std::to_string(fps);
        glfwSetWindowTitle(window, title.c_str());
        nbFrames = 0;
        lastTime = currentTime;
        return fps;
    }
    return double(nbFrames) / (currentTime - lastTime);
}

static void renderScene(GLuint shaderProgram, int width, int height, Camera camera, GLuint sphereBuffer, int numSpheres, int maxDepth, int raysPerPixel)
{
    glUseProgram(shaderProgram);

    // Set the uniform variables
    glUniform1i(glGetUniformLocation(shaderProgram, "width"), width);
    glUniform1i(glGetUniformLocation(shaderProgram, "height"), height);
    glUniform1i(glGetUniformLocation(shaderProgram, "numSpheres"), numSpheres);
    glUniform1i(glGetUniformLocation(shaderProgram, "maxDepth"), maxDepth);
    glUniform1i(glGetUniformLocation(shaderProgram, "raysPerPixel"), raysPerPixel);

    // Set the camera uniform variables
    glUniform3fv(glGetUniformLocation(shaderProgram, "center"), 1, &camera.center[0]);
    glUniform3fv(glGetUniformLocation(shaderProgram, "pixel00_loc"), 1, &camera.pixel00_loc[0]);
    glUniform3fv(glGetUniformLocation(shaderProgram, "pixel_delta_u"), 1, &camera.pixel_delta_u[0]);
    glUniform3fv(glGetUniformLocation(shaderProgram, "pixel_delta_v"), 1, &camera.pixel_delta_v[0]);

    // Bind the sphere buffer
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, sphereBuffer);

    // Bind the accumulation texture
    glActiveTexture(GL_TEXTURE0);

    // Draw a full-screen quad
    glBegin(GL_TRIANGLES);
    glVertex2f(-1.0f, -1.0f);
    glVertex2f(3.0f, -1.0f);
    glVertex2f(-1.0f, 3.0f);
    glEnd();

    // Copy the rendered image to the accumulation texture
    glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 0, 0, width, height, 0);

    glUseProgram(0);
}

static tuple<int, int>set_bounding_box()
{
    const float ratio = 16.0f / 9.0f;
    const int width = 1280;
    const int height = width / ratio;
	return { width, height };
}

static void cleanup(GLFWwindow* window)
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
}

static Camera createCamera(int width, int height)
{
	// Create a camera
	Camera camera;
	camera.init(width, height);
	return camera;
}

static void handleKeyboardAndMouse(GLFWwindow* window, Camera& camera, int fps, double& lastX, double& lastY)
{
    if (ImGui::Button("Reset camera pos"))
        camera.center = vec3(0.0f, 0.0f, 0.0f);

    vec3 center_movement = vec3(0.0f, 0.0f, 0.0f);

    glm::vec3 direction = camera.lookat - camera.center;

    // Calculate the angles in each axis

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        center_movement.z -= 0.1f;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        center_movement.z += 0.1f;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        center_movement.x -= 0.1f;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        center_movement.x += 0.1f;
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        center_movement.y -= 0.1f;
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        center_movement.y += 0.1f;

    camera.center += (center_movement / float(fps)) * 20.0f;

    // check if user is dragging the mouse
    if (ImGui::IsMouseDragging(1, 0.0f))
    {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        double dx = xpos - lastX;
        double dy = ypos - lastY;
        lastX = xpos;
        lastY = ypos;

        camera.dir.x += dx * 0.01f;
        camera.dir.y += dy * 0.01f;
    }
    else
    {
		glfwGetCursorPos(window, &lastX, &lastY);
	}

    camera.update_view();
}

static void showSphereEdit(bool& showSphereEdit, vector<Sphere>& spheres)
{
    ImGui::Begin("Spheres");

    if (ImGui::Button("Add Sphere"))
    {
        Sphere newSphere;
        newSphere.center = vec3(0.0f);
        newSphere.radius = 1.0f;
        newSphere.material = { vec4(1), vec3(0), 0 };
        spheres.push_back(newSphere);
    }

    for (int i = 0; i < spheres.size(); i++)
    {
        ImGui::Text("Sphere %d", i);
        ImGui::SliderFloat3(("Center##" + std::to_string(i)).c_str(), &spheres[i].center[0], -20.0f, 20.0f);
        ImGui::SliderFloat(("Radius##" + std::to_string(i)).c_str(), &spheres[i].radius, 0.1f, 20.0f);
        ImGui::ColorEdit3(("Color##" + std::to_string(i)).c_str(), &spheres[i].material.color[0]);
        ImGui::ColorEdit3(("Emission##" + std::to_string(i)).c_str(), &spheres[i].material.emission[0]);
        ImGui::SliderFloat(("Emission Strength##" + std::to_string(i)).c_str(), &spheres[i].material.emissionStrength, 0.0f, 1.0f);

        if (ImGui::Button(("Remove##" + std::to_string(i)).c_str()))
        {
            spheres.erase(spheres.begin() + i);
            i--;
        }
    }
    ImGui::End();
}

int main()
{
	int width, height;
	std::tie(width, height) = set_bounding_box();

    GLuint accumTexture;
    glGenTextures(1, &accumTexture);
    glBindTexture(GL_TEXTURE_2D, accumTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    GLFWwindow* window = glfw_setup(width, height);

    // Setup Dear ImGui context
    imgui_setup(window);

	// Setup camera
	Camera camera = createCamera(width, height);

    vector<Sphere> spheres = {
        {vec3(0.0f, 0.0f, -3.0f), 1.0f, {vec4(0.5, 1, 1, 1), vec3(0), 0}},
        {vec3(2.0f, 0.0f, -3.0f), 2.0f, {vec4(0.5, 0, 0.7, 1), vec3(1), 1}},
        {vec3(0.0f, 20.5f, -4.0f), 20.0f, {vec4(0.5, 0.9, 0.1, 1), vec3(0), 0}}

    };

    //glfwSetCursorPosCallback(window, cursor_position_callback);


    // Compile and link shaders
    GLuint shaderProgram = createShaderProgram("vertex_shader.glsl", "fragment_shader.glsl");

    // Variables for FPS calculation
    float lastTime = glfwGetTime();
    int nbFrames = 0;
    double lastX = 0, lastY = 0;

    bool showSphereEditBool = false;
    
    int maxDepth = 2;
    int raysPerPixel = 1;
    bool resume = false;
    // Main loop
    while (!glfwWindowShouldClose(window))
    {
        // Poll for events
        glfwPollEvents();
		imgui_start_loop();
		int fps = fps_counter(window, lastTime, nbFrames);

        {
            ImGui::Begin("Tracer Edit");



            handleKeyboardAndMouse(window, camera, fps, lastX, lastY);
            
            ImGui::Checkbox("Show spheres window", &showSphereEditBool);
            if (showSphereEditBool)
                showSphereEdit(showSphereEditBool, spheres);
            
            ImGui::SliderInt("Max Depth", &maxDepth, 2, 30);
            ImGui::SliderInt("Rays Per Pixel", &raysPerPixel, 1, 100);


            ImGui::End();
        }

            static vec3 lastCameraPos = camera.center;

        // Create and fill the sphere buffer
        GLuint sphereBuffer;
        glGenBuffers(1, &sphereBuffer);
        glBindBuffer(GL_UNIFORM_BUFFER, sphereBuffer);
        glBufferData(GL_UNIFORM_BUFFER, spheres.size() * sizeof(Sphere), spheres.data(), GL_STATIC_DRAW);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);

        // Render the scene
        renderScene(shaderProgram, width, height, camera, sphereBuffer, spheres.size(), maxDepth, raysPerPixel);
		imgui_end_loop(window);
    }

    // Cleanup
	cleanup(window);
    return 0;
}