// Use project IWindow and GLFWWindow to create a black 800x600 window titled "Hello Window"

#include <memory>
#include <glad/glad.h>
#include <Window/GLFW/GLFWWindow.hpp>
#include <Logger/SpdLog/SpdLogLoggerAdapter.hpp>
#include <Renderer/Opengl/OpenglRenderer.hpp>


#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
// do not include <imgui_impl_opengl3_loader.h> because glad is already included

using namespace InputProcessor::Logger::SpdLog;

using namespace InputProcessor::Window;
using namespace InputProcessor::Window::GLFW;
using namespace InputProcessor::Renderer::Opengl;
using namespace InputProcessor::Renderer::Resource::Opengl;
using namespace InputProcessor::Renderer;
using namespace InputProcessor::Renderer::Resource;

float vertices[] = {
     0.5f,  0.5f, 0.0f,  // top right
     0.5f, -0.5f, 0.0f,  // bottom right
    -0.5f, -0.5f, 0.0f,  // bottom left
    -0.5f,  0.5f, 0.0f   // top left 
};

unsigned int indices[] = {  // note that we start from 0!
    0, 1, 3,   // first triangle
    1, 2, 3    // second triangle
};

const char* vertexShaderSource = "#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"void main()\n"
"{\n"
"   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
"}\0";

const char* fragmentShaderSource = "#version 330 core\n"
"out vec4 FragColor;\n"
"\n"
"void main()\n"
"{\n"
"   FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
"}\0";

int main() {
    Logger::SetEngineImplementation(std::make_shared<SpdLogLoggerAdapter>("IP_ENGINE"));
    std::unique_ptr<IWindow> window = std::make_unique<GLFWWindow>();

    WindowConfiguration config{ 800, 600, "Hello Window" };
    window->Init(config);

    IRenderer* renderer = OpenglRenderer::GetInstance();
    RendererConfiguration rendererConfig = renderer->GetConfig();
    rendererConfig.ClearBufferColor.Red = 0.2f;
    rendererConfig.ClearBufferColor.Green = 0.3f;
    rendererConfig.ClearBufferColor.Blue = 0.3f;
    rendererConfig.ClearBufferColor.Alpha = 1.0f;
    rendererConfig.ViewPortOptions.Width = 800;
    rendererConfig.ViewPortOptions.Height = 600;

    renderer->Config(rendererConfig);
    
    IShader* shader = OpenglShader::FromSource(vertexShaderSource, fragmentShaderSource);
    IVertexBuffer* vertexBuffer = OpenglVertexBuffer::Create();
    vertexBuffer->SetData(0, vertices, sizeof(vertices), 3, 3 * sizeof(float));

    IIndexBuffer* indexBuffer = OpenglIndexBuffer::Create();
    indexBuffer->SetData(indices, sizeof(indices));


	// Init ImGui
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(static_cast<GLFWwindow*>(window->GetNativeWindow()), true);    
	ImGui_ImplOpenGL3_Init("#version 460");







    // Main loop

    bool show_demo_window = true;
    while (!window->CheckShouldClose()) {
        // Poll events first
        window->PollEvents();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Example UI: demo window and a simple window
        if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);

        ImGui::Begin("Simple Window");
        ImGui::Text("Hello from ImGui integrated into the app");
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::End();

        // Render scene (triangle)
        renderer->GetRendererCommand()->ClearBuffers(ClearBufferMasks::Color);
        shader->Use();
        vertexBuffer->Bind();
        indexBuffer->Bind();
        renderer->GetRendererCommand()->DrawIndex(RenderMode::Triangles, 6);

        // Rendering ImGui
        ImGui::Render();
        int display_w = rendererConfig.ViewPortOptions.Width;
        int display_h = rendererConfig.ViewPortOptions.Height;
        // If window size can change, query it from GLFW; here the project uses a fixed size
        glViewport(0, 0, display_w, display_h);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        window->SwapBuffers();
    }

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

    window->Close();
    OpenglRenderer::Free();
    return 0;
}
