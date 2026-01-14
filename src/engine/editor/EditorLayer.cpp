#include "EditorLayer.h"
#include "../../src/engine/core/Engine.h"

int EditorLayer::is_running() const {
    return isRunning;
}

void EditorLayer::Init(GLFWwindow *window, Engine *engine) {
    m_Engine = engine;
    this->window = window;
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

void EditorLayer::Update(int SCR_WIDTH, int SCR_HEIGHT) {
    renderMenuBar();
    switch (state) {
        case EditorState::Menu:
            renderMenu();
            break;

        case EditorState::Project:
            renderViewport(SCR_WIDTH, SCR_HEIGHT);
            renderInspector();
            renderHeiarchy();
            break;
    }
    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
    glDisable(GL_DEPTH_TEST);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    ImGui::Render();
    ImGui::UpdatePlatformWindows();
    ImGui::RenderPlatformWindowsDefault();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void EditorLayer::renderMenu() {
    ImGui::Begin("Menu");
    ImGui::InputText("Saved Path", projectPath, sizeof(projectPath));
    World &world = m_Engine->getWorld();

    if (ImGui::Button("Open Saved")) {
        auto viewgroup = world.Registry().view<entt::entity>();
        for (auto entity: viewgroup) {
            world.Registry().destroy(entity);
        }
        m_Engine->loadSaved(projectPath);
        if (world.findGameObjectsWithName("Camera") == entt::null) {
            int c = m_Engine->getObjectCounter();
            entt::entity newEntity = world.createEntity("Camera");
            m_Engine->setObjectCounter(c);
            world.addComponent<components::Transform>(newEntity);
            world.addComponent<components::Camera>(newEntity);
        }
        state = EditorState::Project;
    }

    if (ImGui::Button("New")) {
        state = EditorState::Menu;
    }
    ImGui::End();
}

void EditorLayer::renderViewport(int SCR_WIDTH, int SCR_HEIGHT) {
    World &world = m_Engine->getWorld();
    RenderSystem renderSystem = m_Engine->getRenderSystem();
    PhysicsSystem &physicssystem = m_Engine->getPhysicsSystem();

    glm::vec3 cameraPos = m_Engine->getCameraPos();
    glm::vec3 cameraFront = m_Engine->getCameraFront();
    glm::vec3 cameraUp = m_Engine->getCameraUp();
    glm::vec3 lightPos = m_Engine->getLightPos();
    Shader lightShader = m_Engine->getLightShader();
    glm::mat4 model = m_Engine->getModel();
    glm::mat4 view = m_Engine->getView();
    glm::mat4 projection = m_Engine->getProjection();

    projection =
            glm::perspective(glm::degrees(100.0f),
                             (float) SCR_WIDTH / (float) SCR_HEIGHT, 0.1f, 100.0f);
    glm::mat4 rotation =
            glm::rotate(glm::mat4(1.0f),
                        glm::radians(
                            world.getComponent<components::Transform>(world.findGameObjectsWithName("Camera")).rotation.
                            x), glm::vec3(0, 0, 1)) *
            glm::rotate(glm::mat4(1.0f),
                        glm::radians(
                            world.getComponent<components::Transform>(world.findGameObjectsWithName("Camera")).rotation.
                            y), glm::vec3(0, 1, 0)) *
            glm::rotate(glm::mat4(1.0f),
                        glm::radians(
                            world.getComponent<components::Transform>(world.findGameObjectsWithName("Camera")).rotation.
                            z), glm::vec3(1, 0, 0));
    if (isRunning == 0) {
        m_Engine->processInput(window);
    }
    if (isRunning == 1) {
        processRuntimeInput();
        entt::entity camENT = m_Engine->getWorld().findGameObjectsWithName("Camera");
        cameraPos = world.getComponent<components::Transform>(camENT).position;
        cameraFront = glm::normalize(glm::vec3(rotation * world.getComponent<components::Camera>(camENT).CamersFront));
        cameraUp = glm::normalize(glm::vec3(rotation * world.getComponent<components::Camera>(camENT).CamersUp));

        auto viewGroup = world.Registry().view<entt::entity>();

        if (takeBackUp) {
            for (auto entity: viewGroup) {
                string name = world.getComponent<components::Name>(entity).name;
                entt::entity t_entity = temp.create();
                temp.emplace<components::Name>(t_entity, name);

                auto &t = world.Registry().get<components::Transform>(entity);
                temp.emplace<components::Transform>(t_entity, t);

                if (world.Registry().all_of<components::Camera>(entity)) {
                    auto &cam = world.Registry().get<components::Camera>(entity);
                    temp.emplace<components::Camera>(t_entity, cam.CamersFront, cam.CamersUp);
                }

                if (world.Registry().all_of<components::RigidBody>(entity)) {
                    temp.emplace<components::RigidBody>(t_entity);
                }

                if (world.Registry().all_of<components::MeshRenderer>(entity)) {
                    auto &mesh = world.Registry().get<components::MeshRenderer>(entity);
                    temp.emplace<components::MeshRenderer>(t_entity, mesh.shader, mesh.model);
                }
            }
            takeBackUp = false;
        }

        physicssystem.update(world.Registry());
        selectedEntity = entt::null;
    } else if (isRunning == -1) {
        world.Registry().clear();
        auto viewGroup = temp.view<entt::entity>();

        for (auto entity: viewGroup) {
            string name = temp.get<components::Name>(entity).name;
            entt::entity m_entity = world.createEntity(name);

            auto &t = temp.get<components::Transform>(entity);
            world.Registry().emplace<components::Transform>(m_entity, t);

            if (temp.all_of<components::Camera>(entity)) {
                auto &cam = temp.get<components::Camera>(entity);
                world.Registry().emplace<components::Camera>(m_entity, cam.CamersFront, cam.CamersUp);
            }

            if (temp.all_of<components::RigidBody>(entity)) {
                world.Registry().emplace<components::RigidBody>(m_entity);
            }

            if (temp.all_of<components::MeshRenderer>(entity)) {
                auto &mesh = temp.get<components::MeshRenderer>(entity);
                world.Registry().emplace<components::MeshRenderer>(m_entity, mesh.shader, mesh.model);
            }
        }
        physicssystem.setup(world.Registry());
        temp.clear();
        isRunning = 0;
    }
    view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
    view = glm::rotate(view, glm::radians(180.0f), glm::vec3(0.0f, 0.0f, 1.0f));

    renderSystem.update(world.Registry(), model, view, projection, lightPos, cameraPos);

    // Lights
    lightShader.use();
    glUniformMatrix4fv(glGetUniformLocation(lightShader.ID, "projection"), 1,
                       GL_FALSE, &projection[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(lightShader.ID, "view"), 1,
                       GL_FALSE, &view[0][0]);
    model = glm::mat4(1.0f);
    model = glm::translate(model, lightPos);
    model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));
    model = glm::rotate(model, glm::radians(0.0f), glm::vec3(0, 1, 0));
    glUniformMatrix4fv(glGetUniformLocation(lightShader.ID, "model"), 1,
                       GL_FALSE, &model[0][0]);
    glBindVertexArray(m_Engine->lightVAO);
    glDrawArrays(GL_TRIANGLES, 0, sizeof(m_Engine->vertices));

    ImGui::Begin("Viewport");
    if (ImGui::BeginChild("child")) {
        if (ImGui::Button("Run") && isRunning != 1) {
            isRunning = 1;
            takeBackUp = true;
        }
        ImGui::SameLine();
        if (ImGui::Button("Stop") && isRunning == 1) {
            isRunning = -1;
        }

        ImVec2 availSize = ImGui::GetContentRegionAvail();
        ImGui::Image((void *) (intptr_t) m_Engine->fboTexture, availSize, ImVec2(0, 1), ImVec2(1, 0));

        ImGui::EndChild();
    }
    ImGui::End();
}

void EditorLayer::renderInspector() {
    World &world = m_Engine->getWorld();
    PhysicsSystem &physicssystem = m_Engine->getPhysicsSystem();
    static char pathInput[256] = "";
    ImGui::Begin("Inspector");
    if (selectedEntity != entt::null) {
        auto &transform = world.Registry().get<components::Transform>(selectedEntity);
        float pos[3] = {transform.position.x, transform.position.y, transform.position.z};
        float rot[3] = {transform.rotation.x, transform.rotation.y, transform.rotation.z};
        float scale[3] = {transform.scale.x, transform.scale.y, transform.scale.z};

        static char objTag[256];
        std::strncpy(objTag, world.getComponent<components::Tag>(selectedEntity).Tag.c_str(),
                     sizeof(world.getComponent<components::Tag>(selectedEntity).Tag));
        ImGui::Text("Tag: ");
        ImGui::SameLine();
        if (ImGui::InputText("##input", objTag, sizeof(objTag)) && objTag[0] != NULL) {
            world.addTag(selectedEntity, objTag);
        }


        static char objName[256];
        std::strncpy(objName, world.getComponent<components::Name>(selectedEntity).name.c_str(),
                     sizeof(world.getComponent<components::Name>(selectedEntity).name));
        ImGui::Text("Name: ");
        ImGui::SameLine();
        if (ImGui::InputText("##input", objName, sizeof(objName)) && objName[0] != NULL) {
            world.getComponent<components::Name>(selectedEntity).name = objName;
        }

        if (ImGui::CollapsingHeader("Transform")) {
            if (ImGui::DragFloat3("Position ", pos)) {
                transform.position = glm::vec3(pos[0], pos[1], pos[2]);
                physicssystem.updateWorld(world.Registry());
            }
            if (ImGui::DragFloat3("Rotation ", rot)) {
                transform.rotation = glm::vec3(rot[0], rot[1], rot[2]);
                physicssystem.updateWorld(world.Registry());
            }
            if (ImGui::DragFloat3("Scale ", scale)) {
                transform.scale = glm::vec3(scale[0], scale[1], scale[2]);
                physicssystem.updateWorld(world.Registry());
            }
        }

        if (world.Registry().all_of<components::MeshRenderer>(selectedEntity)) {
            if (ImGui::CollapsingHeader("Mesh Renderer")) {
                auto &mesh = world.Registry().get<components::MeshRenderer>(selectedEntity);
                ImGui::Text("Path: %s", mesh.model->dir.c_str());
            }
        }
        if (ImGui::Button("Add RigidBody Component")) {
            if (!world.Registry().all_of<components::RigidBody>(selectedEntity)) {
                world.addComponent<components::RigidBody>(selectedEntity);
                physicssystem.updateWorld(world.Registry());
            }
        }
        ImGui::InputText("Model Path", pathInput, sizeof(pathInput));

        if (ImGui::Button("Add Mesh Component")) {
            if (!world.Registry().all_of<components::MeshRenderer>(selectedEntity)) {
                Model *model = new Model(pathInput);
                Shader *planeShader = new Shader("../shaders/color.vert", "../shaders/color.frag");
                world.addComponent<components::MeshRenderer>(selectedEntity, planeShader, model);
            }
        }
    }
    ImGui::End();
}

void EditorLayer::renderHeiarchy() {
    World &world = m_Engine->getWorld();
    auto viewGroup = world.Registry().view<components::Transform>();
    char pathInput[256];

    ImGui::Begin("Scene");
    for (auto entity: viewGroup) {
        if (ImGui::Button(world.getComponent<components::Name>(entity).name.c_str(), ImVec2(100, 20))) {
            selectedEntity = entity;
            memset(pathInput, 0, sizeof(pathInput));
        }
    }
    if (ImGui::Button("Create new gameobject")) {
        int c = m_Engine->getObjectCounter();
        entt::entity newEntity = world.createEntity("GameObject " + to_string(c++));
        m_Engine->setObjectCounter(c);
        world.addComponent<components::Transform>(newEntity);
    }
    ImGui::End();
}

void EditorLayer::processRuntimeInput()  {
    ImGuiIO& io = ImGui::GetIO();
    if (io.WantCaptureKeyboard)
        return;

    World& world = m_Engine->getWorld();
    entt::entity camENT = world.findGameObjectsWithName("Camera");
    if (camENT == entt::null)
        return;

    auto& transform = world.getComponent<components::Transform>(camENT);
    auto& camera    = world.getComponent<components::Camera>(camENT);

    float speed = 5.0f;

    // Build orientation from rotation
    glm::mat4 rotation =
        glm::rotate(glm::mat4(1.0f), glm::radians(transform.rotation.x), glm::vec3(0, 0, 1)) *
        glm::rotate(glm::mat4(1.0f), glm::radians(transform.rotation.y), glm::vec3(0, 1, 0)) *
        glm::rotate(glm::mat4(1.0f), glm::radians(transform.rotation.z), glm::vec3(1, 0, 0));

    glm::vec3 forward = glm::normalize(glm::vec3(rotation * camera.CamersFront));
    glm::vec3 up      = glm::normalize(glm::vec3(rotation * camera.CamersUp));
    glm::vec3 right   = glm::normalize(glm::cross(forward, up));

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        transform.position += forward * speed;

    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        transform.position -= forward * speed;

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        transform.position -= right * speed;

    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        transform.position += right * speed;
}


void EditorLayer::renderMenuBar() {
    World &world = m_Engine->getWorld();
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;

    const ImGuiViewport *viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

    window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

    ImGui::Begin("DockSpace Demo", nullptr, window_flags);

    ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("New")) {
                state = EditorState::Menu;
            }
            if (ImGui::MenuItem("Save")) {
                m_Engine->saveProject(projectPath);
            }
            if (ImGui::MenuItem("Open")) {
                state = EditorState::Menu;
            }
            if (ImGui::MenuItem("Exit")) {
                glfwSetWindowShouldClose(window, true);
            }
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
    ImGui::End();
    ImGui::PopStyleVar(2);
}
