#include <iostream>
#include <memory>

#include <glad/glad.h>

#include <Logger/Logger.hpp>
#include <Logger/SpdLog/SpdLogLoggerAdapter.hpp>
#include <Window/GLFW/GLFWWindow.hpp>
#include <Resource/ResouceManager.hpp>
#include <Resource/Opengl/OpenglResourceManager.hpp>
#include <Renderer/IRendererCommand.hpp>
#include <Renderer/Opengl/OpenglRendererCommand.hpp>

using namespace InputProcessor::Logger;
using namespace InputProcessor::Window;
using namespace InputProcessor::Window::GLFW;
using namespace InputProcessor::Resource;
using namespace InputProcessor::Resource::Opengl;
using namespace InputProcessor::Renderer;
using namespace InputProcessor::Renderer::Opengl;

const char* vertexShaderSource = "#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"void main()\n"
"{\n"
"   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
"}\0";

const char* fragmentShaderSource = "#version 330 core\n"
"out vec4 FragColor;\n"
"void main()\n"
"{\n"
"   FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
"}\0";

float vertices[] = {
    -0.5f, -0.5f, 0.0f,
     0.5f, -0.5f, 0.0f,
     0.0f,  0.5f, 0.0f
};

unsigned int indices[] = { 0,1,2 };

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

    IVertexBuffer* vertexBuffer = ResourceManager::CreateVertexBuffer();
    vertexBuffer->SetData(0, vertices, sizeof(vertices), 3, 3 * sizeof(float), BufferDataType::Float);

    IIndexBuffer* indexBuffer = ResourceManager::CreateIndexBuffer();
    indexBuffer->SetData(indices, sizeof(indices));

    IRendererCommand* rendererCommand = new OpenglRendererCommand();

    while (!window->CheckShouldClose()) {
        rendererCommand->SetClearColor({ 1.f, 1.f, 1.f, 1.f });
        rendererCommand->ClearBuffers(BufferFlag::Color);
        glClear(GL_COLOR_BUFFER_BIT);

        shader->Use();
        vertexBuffer->Bind();
        indexBuffer->Bind();
        rendererCommand->DrawIndex(InputProcessor::Renderer::RenderMode::Triangles, 3);
        vertexBuffer->UnBind();

        window->SwapBuffers();
        window->PollEvents();
    }

    window->Close();
    delete rendererCommand;
    return 0;
}