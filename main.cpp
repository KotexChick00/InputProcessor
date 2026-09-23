#include <iostream>
#include <Application/Application.hpp>
#include <Model/Importers/AssimpModelImporter/AssimpModelImporter.hpp>
#include <Model/RenderModel.hpp>


#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

const char* vertexSrc = "#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"layout (location = 1) in vec3 aNormal;\n"
"layout (location = 2) in vec2 aTexCoords;\n"
"layout (location = 3) in vec3 aColor;\n"
"uniform mat4 uModel;\n"
"uniform mat4 uViewProjection;\n"
"out vec2 vTexCoords;\n"
"void main() {\n"
"   gl_Position = uViewProjection * uModel * vec4(aPos, 1.0f);\n"
"   vTexCoords = aTexCoords;\n"
"}\n";

const char* fragmentSource = "#version 330 core\n"
"in vec2 vTexCoords;\n"
"out vec4 color;\n"
"\n"
"struct Material {\n"
"   sampler2D diffuse[1];\n"      // phải có [1] để khớp với [0]
"};\n"
"uniform Material material;\n"
"\n"
"void main() {\n"
"   color = texture(material.diffuse[0], vTexCoords);\n"
"}\n";

class DemoApp : public CoreEngine::Application {
public:
    DemoApp(CoreEngine::ApplicationConfiguration& config) : Application(config) {}

protected:
    void OnInitClient() override {
        CoreEngine::Renderer::IRenderer* renderer = GetRenderer();
        CoreEngine::Renderer::RendererConfiguration config = renderer->GetConfig();
        config.ClearBufferColor.Red = 0.1f;
        config.DepthOptions.Enabled = true; // model 3D cần depth test, khác demo tam giác 2D ban đầu
        config.DepthOptions.Operation = CoreEngine::Renderer::DepthOperation::ReadAndWrite;
        renderer->Config(config);

        shader = renderer->GetResourceManager()->CreateShaderFromSources(vertexSrc, fragmentSource);
        if (!shader) {
            IP_ENGINE_ERROR("DemoApp::OnInitClient: Failed to create shader");
            return;
        }

        // wiring: AssimpModelImporter cần IResourceManager, lấy qua renderer đã có sẵn
        Model::AssimpModelImporter importer(renderer->GetResourceManager());
        model = importer.Import("./assets/models/cube-tex.obj"); // đường dẫn model test đơn giản trước

        if (!model) {
            IP_ENGINE_ERROR("DemoApp::OnInitClient: Failed to import model");
        }
    }

    void OnLoopClient() override {
        GetRenderer()->GetRendererCommand()->ClearBuffers(
            CoreEngine::Renderer::ClearBufferMasks::Color | CoreEngine::Renderer::ClearBufferMasks::Depth
        );

        if (model && shader) {
            shader->Use();

            // Đưa cube về giữa
            glm::mat4 modelMatrix = glm::mat4(1.0f);
            modelMatrix = glm::translate(modelMatrix, glm::vec3(-0.5f, -0.5f, -0.5f));

            glm::mat4 view = glm::lookAt(
                glm::vec3(3.0f, 3.0f, 3.0f),  // Đặt camera lệch đều cả 3 trục (X, Y, Z > 0)
                glm::vec3(0.0f, 0.0f, 0.0f),  // Nhìn vào tâm xúc xắc
                glm::vec3(0.0f, 1.0f, 0.0f)   // Trục Y hướng lên trên
            );

            glm::mat4 projection = glm::perspective(
                glm::radians(45.0f),
                1000.0f / 1000.0f,
                0.1f,
                100.0f
            );

            glm::mat4 viewProjection = projection * view;

            shader->SetUniformMatrix4fv("uModel", glm::value_ptr(modelMatrix));
            shader->SetUniformMatrix4fv("uViewProjection", glm::value_ptr(viewProjection));

            model->Render(GetRenderer(), shader);
        }
    }

    void OnShutdownClient() override {
        std::cout << "CLIENT SHUTDOWN" << std::endl;
    }

private:
    CoreEngine::Renderer::IShader* shader = nullptr;
    std::unique_ptr<Model::RenderModel> model = nullptr;
};

int main() {
    CoreEngine::ApplicationConfiguration config;
    config.Width = 1000;
    config.Height = 1000;
    config.Title = "Hello World!";
    config.WindowPlatformSpec = CoreEngine::WindowPlatformSpec::GLFW;
    config.RenderAPI = CoreEngine::RenderAPI::Opengl;

    CoreEngine::Application* application = new DemoApp(config);
    application->Run();
    delete application;
}