#pragma once
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include "imgui.h"
#include <entt/entt.hpp>
#include <glm/glm.hpp>

class Engine;

enum EditorState {
    Menu,
    Project
};

class EditorLayer {
public:

private:
    EditorState state = EditorState::Menu;
    Engine *m_Engine = nullptr;
    GLFWwindow *window = nullptr;
    char projectPath[256];
    entt::entity selectedEntity = entt::null;
    int isRunning = 0;
    bool takeBackUp = false;
    entt::registry temp;
    glm::vec3 game_camera_pos;

public:
    int is_running() const;

    void Init(GLFWwindow *window, Engine *engine);

    void Update(int SCR_WIDTH, int SCR_HEIGHT);

    void renderMenuBar();

    void renderMenu();

    void renderViewport(int SCR_WIDTH, int SCR_HEIGHT);

    void renderInspector();

    void renderHeiarchy();

    void processRuntimeInput();
};
