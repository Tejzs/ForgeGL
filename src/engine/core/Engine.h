#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "../../src/engine/core/World.h"
#include <glm/glm.hpp>
#include "../../src/engine/renderer/Shader.h"
#include "../../src/engine/core/RenderSystem.h"
#include "../../src/engine/core/PhysicsSystem.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include "imgui.h"
#include <nlohmann/json.hpp>
#include "../../src/engine/editor/EditorLayer.h"

using json = nlohmann::json;

class Engine {
private:
    World world;
    RenderSystem renderSystem;
    PhysicsSystem physicssystem;
    EditorLayer editorLayer;
    entt::entity camera_entity = getWorld().findGameObjectsWithName("Camera");

    static uint32_t objectCounter;

    glm::vec3 cameraPos = glm::vec3(0.0f, -7.0f, 10.0f);
    glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 lightPos = glm::vec3(-2.5f, 1.0f, 1.5f);
    Shader lightShader = Shader("../shaders/light.vert", "../shaders/light.frag");
    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 view;
    glm::mat4 projection;
    entt::entity selectedEntity = entt::null;
    bool isProjectLoaded = false;

    float lastX = 400, lastY = 300;
    float firstMouse = true;
    float yaw = -90.0f;
    float pitch = 0.0f;

public:
    unsigned int lightVAO, lightVBO;
    float vertices[256] = {
        -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 1.0f,
        1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, -1.0f, 1.0f, -1.0f,
        1.0f, -1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f,
        1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f,
        1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f,
        1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f,
        1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f,
        1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f,
        1.0f, 1.0f, 1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 1.0f, -1.0f, 1.0f
    };
    GLuint fbo = 0;
    GLuint fboTexture = 0;
    GLuint rboDepth = 0;
    int framebufferWidth = 0;
    int framebufferHeight = 0;

    World &getWorld() { return world; };

    glm::vec3 getCameraPos() { return cameraPos; };
    glm::vec3 getCameraFront() { return cameraFront; };
    void setCameraFront(glm::vec3 camFront) { cameraFront = camFront; };
    glm::vec3 getCameraUp() { return cameraUp; };
    void setCameraUp(glm::vec3 camUp) { cameraUp = camUp; };

    glm::vec3 getLightPos() { return lightPos; };
    glm::mat4 getModel() { return model; }
    glm::mat4 getView() { return view; }
    glm::mat4 getProjection() { return projection; };
    void setObjectCounter(int objCounter) { objectCounter = objCounter; };
    uint32_t getObjectCounter() { return objectCounter; };

    PhysicsSystem &getPhysicsSystem() { return physicssystem; };
    RenderSystem &getRenderSystem() { return renderSystem; };

    Shader &getLightShader() { return lightShader; };

    void EditorSetup(GLFWwindow *window);

    void UpdateEditor(int SCR_WIDTH, int SCR_HEIGHT);

    void setup_imgui(GLFWwindow *window);

    void processInput(GLFWwindow *window);
    static void mouse_callback(GLFWwindow *window, double xpos, double ypos);

    void loadSaved(const char *path);

    void saveProject(const char *path);

    void newProject(const char *path);

    void SetupFramebuffer(int width, int height);

    void cloneR(entt::registry &src, entt::registry &dst);

    void run(GLFWwindow *window, int SCR_WIDTH, int SCR_HEIGHT);
};
