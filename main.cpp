#include <iostream>
#include <memory>
#include <cfloat>

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <Logger/Logger.hpp>
#include <Logger/SpdLog/SpdLogLoggerAdapter.hpp>
#include <Window/GLFW/GLFWWindow.hpp>
#include <Resource/ResouceManager.hpp>
#include <Resource/Opengl/OpenglResourceManager.hpp>
#include <Renderer/IRendererCommand.hpp>
#include <Renderer/Opengl/OpenglRendererCommand.hpp>
#include <OBJ/ObjParser.h>
#include <OBJ/ObjMeshConverter.h>
#include <OBJ/ObjLogger.h>

using namespace InputProcessor::Logger;
using namespace InputProcessor::Window;
using namespace InputProcessor::Window::GLFW;
using namespace InputProcessor::Resource;
using namespace InputProcessor::Resource::Opengl;
using namespace InputProcessor::Renderer;
using namespace InputProcessor::Renderer::Opengl;

const char* vertexShaderSource = "#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"uniform mat4 uModel;\n"
"uniform mat4 uView;\n"
"uniform mat4 uProjection;\n"
"void main()\n"
"{\n"
"   gl_Position = uProjection * uView * uModel * vec4(aPos, 1.0);\n"
"}\0";

const char* fragmentShaderSource = "#version 330 core\n"
"out vec4 FragColor;\n"
"void main()\n"
"{\n"
"   FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
"}\0";

int main() {
    auto spdlogger = std::make_shared<SpdLog::SpdLogLoggerAdapter>("Main");
    Logger::SetEngineImplementation(spdlogger);

    std::unique_ptr<IWindow> window = std::make_unique<GLFWWindow>();
    WindowConfiguration config{ 800, 600, "Hello, World!" };
    window->Init(config);

    std::shared_ptr<ResourceManager> resourceManager = std::make_shared<OpenglResourceManager>();
    ResourceManager::SetInstance(resourceManager);

    ShaderData shaderData = ShaderData({ vertexShaderSource, fragmentShaderSource, "OK" });
    IShader* shader = ResourceManager::GetShaderFromSource(shaderData);

    // --- Nạp dữ liệu từ file OBJ ---
    ObjParser objParser("main.obj");
    objParser.Parse();
    MeshRenderData meshData = ObjMeshConverter::ConvertToMeshRenderData(objParser);

    IVertexBuffer* vertexBuffer = ResourceManager::CreateVertexBuffer();
    vertexBuffer->SetData(
        0,
        meshData.vertices.data(),
        meshData.vertices.size() * sizeof(float),
        3,
        3 * sizeof(float),
        BufferDataType::Float
    );

    IIndexBuffer* indexBuffer = ResourceManager::CreateIndexBuffer();
    indexBuffer->SetData(
        meshData.indices.data(),
        meshData.indices.size() * sizeof(unsigned int)
    );

    IRendererCommand* rendererCommand = new OpenglRendererCommand();

    // --- Tính bounding box để đặt camera nhìn trọn model ---
    glm::vec3 minBound(FLT_MAX, FLT_MAX, FLT_MAX);
    glm::vec3 maxBound(-FLT_MAX, -FLT_MAX, -FLT_MAX);

    for (size_t i = 0; i < meshData.vertices.size(); i += 3) {
        glm::vec3 v(meshData.vertices[i], meshData.vertices[i + 1], meshData.vertices[i + 2]);
        minBound = glm::min(minBound, v);
        maxBound = glm::max(maxBound, v);
    }

    glm::vec3 center = (minBound + maxBound) * 0.5f;
    float radius = glm::length(maxBound - minBound) * 0.5f;
    if (radius < 0.001f) radius = 1.0f;

    float fovDegrees = 45.0f;
    float distance = radius / sin(glm::radians(fovDegrees * 0.5f)) * 1.2f;

    glm::vec3 cameraPos = center + glm::vec3(0.0f, 0.0f, distance);

    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 view = glm::lookAt(cameraPos, center, glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 projection = glm::perspective(
        glm::radians(fovDegrees),
        800.0f / 600.0f,
        0.1f,
        distance * 3.0f
    );

    // --- Gán uniform một lần (camera cố định) ---
    shader->Use();

    GLint currentProgram = 0;
    glGetIntegerv(GL_CURRENT_PROGRAM, &currentProgram);

    GLint modelLoc = glGetUniformLocation(currentProgram, "uModel");
    GLint viewLoc = glGetUniformLocation(currentProgram, "uView");
    GLint projLoc = glGetUniformLocation(currentProgram, "uProjection");

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    glEnable(GL_DEPTH_TEST);

    while (!window->CheckShouldClose()) {
        rendererCommand->SetClearColor({ 1.f, 1.f, 1.f, 1.f });
        rendererCommand->ClearBuffers(BufferFlag::Color);
        glClear(GL_DEPTH_BUFFER_BIT);

        shader->Use();
        vertexBuffer->Bind();
        indexBuffer->Bind();
        rendererCommand->DrawIndex(
            InputProcessor::Renderer::RenderMode::Triangles,
            static_cast<unsigned int>(meshData.indices.size())
        );
        vertexBuffer->UnBind();

        window->SwapBuffers();
        window->PollEvents();
    }

    window->Close();
    delete rendererCommand;
    return 0;
}