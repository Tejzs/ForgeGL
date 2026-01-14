#include "Engine.h"

uint32_t Engine::objectCounter = 0;

void Engine::setup_imgui(GLFWwindow *window) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void) io;
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 460");
    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
}

void Engine::processInput(GLFWwindow *window) {
    ImGuiIO &io = ImGui::GetIO();
    if (!io.WantCaptureKeyboard) {
        float moveSpeed = 5.0f;
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
            cameraPos += cameraFront * moveSpeed;
        }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
            cameraPos -= cameraFront * moveSpeed;
        }
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
            cameraPos -= glm::normalize(glm::cross(cameraUp, cameraFront)) * moveSpeed;
        }
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
            cameraPos += glm::normalize(glm::cross(cameraUp, cameraFront)) * moveSpeed;
        }
    }
}

void Engine::mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    Engine* engine = static_cast<Engine*>(glfwGetWindowUserPointer(window));
    if (!engine)
        return;


    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) != GLFW_PRESS) {
        engine->firstMouse = true;
        return;
    }

    if (engine->editorLayer.is_running())
        return;

    if (engine->firstMouse) {
        engine->lastX = (float)xpos;
        engine->lastY = (float)ypos;
        engine->firstMouse = false;
        return;
    }

    float xoffset = (float)xpos - engine->lastX;
    float yoffset = engine->lastY - (float)ypos;

    engine->lastX = (float)xpos;
    engine->lastY = (float)ypos;

    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    engine->yaw   += xoffset;
    engine->pitch += yoffset;
    engine->pitch = glm::clamp(engine->pitch, -89.0f, 89.0f);

    glm::vec3 direction;
    direction.x = cos(glm::radians(engine->yaw)) * cos(glm::radians(engine->pitch));
    direction.y = sin(glm::radians(engine->pitch));
    direction.z = sin(glm::radians(engine->yaw)) * cos(glm::radians(engine->pitch));

    engine->cameraFront = glm::normalize(direction);
}


void Engine::loadSaved(const char *path) {
    std::ifstream in(path);
    if (in.is_open()) {
        json scene;
        in >> scene;
        for (const auto &obj: scene) {
            entt::entity entity = world.createEntity(obj["name"].get<std::string>());

            glm::vec3 pos = {
                obj["transform"]["position"][0], obj["transform"]["position"][1], obj["transform"]["position"][2]
            };
            glm::vec3 rot = {
                obj["transform"]["rotation"][0], obj["transform"]["rotation"][1], obj["transform"]["rotation"][2]
            };
            glm::vec3 scale = {
                obj["transform"]["scale"][0], obj["transform"]["scale"][1], obj["transform"]["scale"][2]
            };

            world.addComponent<components::Transform>(entity, pos, rot, scale);

            world.getComponent<components::Tag>(entity).Tag = obj["Tag"];

            if (obj.contains("meshRenderer")) {
                string modelPath = obj["meshRenderer"]["model_path"];
                cout << modelPath;
                string fragShaderPath = obj["meshRenderer"]["frag_shader_path"];
                string vertShaderPath = obj["meshRenderer"]["vert_shader_path"];

                Model *model = new Model(modelPath);
                Shader *mShader = new Shader("../shaders/color.vert", "../shaders/color.frag");

                world.addComponent<components::MeshRenderer>(entity, mShader, model);
            }

            if (obj.contains("Camera")) {
                glm::vec4 camF = {
                    obj["Camera"]["CamersFront"][0], obj["Camera"]["CamersFront"][1], obj["Camera"]["CamersFront"][2],
                    obj["Camera"]["CamersFront"][3]
                };
                glm::vec4 CamUp = {
                    obj["Camera"]["CamersUp"][0], obj["Camera"]["CamersUp"][1], obj["Camera"]["CamersUp"][2],
                    obj["Camera"]["CamersUp"][3]
                };
                world.addComponent<components::Camera>(entity, camF, CamUp);
            }

            if (obj.contains("rigidbody") && obj["rigidbody"]["rb"] == true) {
                world.addComponent<components::RigidBody>(entity);
                physicssystem.setup(world.Registry());
            }
        }
        isProjectLoaded = true;
    }
}

void Engine::saveProject(const char *path) {
    entt::registry &registry = world.Registry();
    json scene = json::array();

    for (auto entity: registry.view<components::Transform>()) {
        json obj;
        obj["id"] = (uint64_t) entity;

        auto &n = registry.get<components::Name>(entity);
        obj["name"] = n.name;

        auto &tag = registry.get<components::Tag>(entity);
        obj["Tag"] = tag.Tag;

        // Transform Component
        auto &t = registry.get<components::Transform>(entity);
        obj["transform"] = {
            {"position", {t.position.x, t.position.y, t.position.z}},
            {"rotation", {t.rotation.x, t.rotation.y, t.rotation.z}},
            {"scale", {t.scale.x, t.scale.y, t.scale.z}}
        };

        // MeshRenderer Component
        if (world.Registry().all_of<components::MeshRenderer>(entity)) {
            auto &mesh = registry.get<components::MeshRenderer>(entity);
            obj["meshRenderer"] = {
                {"model_path", mesh.model->pathToModel},
                {"frag_shader_path", mesh.shader->fragShaderSourcePath},
                {"vert_shader_path", mesh.shader->vertShaderSourcePath},
            };
        }

        // Camera Component
        if (world.Registry().all_of<components::Camera>(entity)) {
            auto &cam = registry.get<components::Camera>(entity);
            obj["Camera"] = {
                {"CamersFront", {cam.CamersFront.x, cam.CamersFront.y, cam.CamersFront.z, cam.CamersFront.w}},
                {"CamersUp", {cam.CamersUp.x, cam.CamersUp.y, cam.CamersUp.z, cam.CamersUp.w}},
            };
        }

        // RigidBody Component
        if (world.Registry().all_of<components::RigidBody>(entity)) {
            if (world.Registry().all_of<components::RigidBody>(entity)) {
                obj["rigidbody"] = {
                    {"rb", true},
                };
            }
        }

        scene.push_back(obj);
    }

    std::ofstream file(path);
    file << scene;
}

void Engine::newProject(const char *path) {
    world.Registry().clear();
    saveProject(path);
    loadSaved(path);
}

void Engine::SetupFramebuffer(int width, int height) {
    framebufferWidth = width;
    framebufferHeight = height;

    if (fbo) {
        glDeleteFramebuffers(1, &fbo);
        glDeleteTextures(1, &fboTexture);
        glDeleteRenderbuffers(1, &rboDepth);
    }

    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    glGenTextures(1, &fboTexture);
    glBindTexture(GL_TEXTURE_2D, fboTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0,
                 GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D, fboTexture, 0);

    glGenRenderbuffers(1, &rboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);

    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
                              GL_RENDERBUFFER, rboDepth);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cerr << "ERROR: Framebuffer is not complete!" << std::endl;

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Engine::EditorSetup(GLFWwindow *window) {
    editorLayer.Init(window, this);
}

void Engine::UpdateEditor(int SCR_WIDTH, int SCR_HEIGHT) {
    editorLayer.Update(SCR_WIDTH, SCR_HEIGHT);
}

void cloneR(entt::registry &src, entt::registry &dst) {
}


void Engine::run(GLFWwindow *window, int SCR_WIDTH, int SCR_HEIGHT) {
    EditorSetup(window);
    SetupFramebuffer(SCR_WIDTH, SCR_HEIGHT);
    physicssystem.setup(world.Registry());

    // Lighting
    glGenVertexArrays(1, &lightVAO);
    glGenBuffers(1, &lightVBO);

    glBindVertexArray(lightVAO);
    glBindBuffer(GL_ARRAY_BUFFER, lightVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float),
                          (void *) 0); // Position
    glEnableVertexAttribArray(0);

    while (!glfwWindowShouldClose(window)) {
        // Input
        static char projectPath[256] = "";

        // Bind the framebuffer for offscreen rendering
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glViewport(0, 0, framebufferWidth, framebufferHeight);
        glEnable(GL_DEPTH_TEST);

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        UpdateEditor(SCR_WIDTH, SCR_HEIGHT);

        glfwPollEvents();
        glfwSwapBuffers(window);
    }
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}
