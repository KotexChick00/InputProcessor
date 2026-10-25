#include <iostream>
#include <cassert>
#include <cmath>
#include <string_view>
#include <memory>
#include <filesystem>
#include <stdexcept>
#include <wx/defs.h>
#include <Model/Importers/ModelImporter/ModelImporter.hpp>
#include <Model/Importers/AssimpModelImporter/AssimpBoundingBox.h>

// Math & Core
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Camera
#include <Camera/OrbitCamera/OrbitCamera.hpp>
#include <Camera/OrbitCamera/OrbitCameraController.hpp>

// Input
#include <Input/InputState.hpp>
#include <Input/Wx/WxMouseInput.hpp>
#include <Input/Wx/WxKeyboardInput.hpp>

// Importers
#include <Model/Importers/ModelImporter/ModelFormatHandler.hpp>
#include <Model/Importers/ModelImporter/GltfModelFormatHandler.hpp>

using OrbitCamera = Domain::Camera::OrbitCamera::OrbitCamera;
using OrbitCameraController = Domain::Camera::OrbitCamera::OrbitCameraController;
using namespace CoreEngine::Input;
using namespace CoreEngine::Input::Wx;
using namespace Model;

// Helper: check float equality with epsilon
bool FloatEqual(float a, float b, float eps = 1e-4f) {
    return std::fabs(a - b) < eps;
}

// Helper: check vec3 equality
bool Vec3Equal(const glm::vec3& a, const glm::vec3& b, float eps = 1e-3f) {
    return glm::length(a - b) < eps;
}

void TestOrbitCamera() {
    std::cout << "[RUN] Testing OrbitCamera...\n";

    glm::vec3 target(0.0f, 1.0f, 0.0f);
    float distance = 10.0f;
    OrbitCamera camera(target, distance);

    // Initial position: Azimuth=0, Elevation=0 -> x=0, y=target.y, z=target.z + distance
    glm::vec3 initialPos = camera.GetPosition();
    assert(FloatEqual(initialPos.x, 0.0f));
    assert(FloatEqual(initialPos.y, 1.0f));
    assert(FloatEqual(initialPos.z, 10.0f));

    // Forward direction must point from eye to target: target - eye = (0, 1, 0) - (0, 1, 10) = (0, 0, -10) -> normalized (0, 0, -1)
    glm::vec3 forward = camera.GetForward();
    assert(Vec3Equal(forward, glm::vec3(0.0f, 0.0f, -1.0f)));

    // Up vector must be (0, 1, 0)
    glm::vec3 up = camera.GetUp();
    assert(Vec3Equal(up, glm::vec3(0.0f, 1.0f, 0.0f)));

    // Right vector must be (1, 0, 0)
    glm::vec3 right = camera.GetRight();
    assert(Vec3Equal(right, glm::vec3(1.0f, 0.0f, 0.0f)));

    // Test Zoom
    camera.Zoom(2.0f); // Zoom in by 2 -> distance should be 8.0f
    glm::vec3 zoomedPos = camera.GetPosition();
    assert(FloatEqual(zoomedPos.z, 8.0f));

    // Test Zoom Clamp to Min (kMinZoomDistance = 0.1f)
    camera.Zoom(100.0f);
    assert(camera.GetPosition().z >= 0.1f);

    // Test Rotation: Rotate Azimuth by 90 degrees (pi/2)
    OrbitCamera rotCam(glm::vec3(0.0f), 5.0f);
    rotCam.Rotate(glm::half_pi<float>(), 0.0f);
    glm::vec3 rotPos = rotCam.GetPosition();
    // At azimuth = pi/2, x = 5 * sin(pi/2) = 5, z = 5 * cos(pi/2) = 0
    assert(FloatEqual(rotPos.x, 5.0f));
    assert(FloatEqual(rotPos.z, 0.0f));

    // Test Elevation Clamping: cannot go beyond +/- 90 degrees
    rotCam.Rotate(0.0f, 10.0f); // Try to rotate elevation way past pi/2
    glm::vec3 clampedPos = rotCam.GetPosition();
    assert(clampedPos.y < 5.0f); // Y cannot equal or exceed distance because of kElevationEpsilon clamp

    // Matrices are valid and non-zero
    const glm::mat4& viewMat = rotCam.GetViewMatrix();
    assert(viewMat[3][3] == 1.0f);
    const glm::mat4& projMat = rotCam.GetProjectionMatrix();
    assert(FloatEqual(projMat[2][3], -1.0f)); // Standard perspective projection (row 3, col 2 in column-major)

    std::cout << "  -> OrbitCamera PASS!\n";
}

void TestOrbitCameraController() {
    std::cout << "[RUN] Testing OrbitCameraController...\n";

    OrbitCamera camera(glm::vec3(0.0f), 5.0f);

    WxMouseInput mouse;
    WxKeyboardInput keyboard;
    InputState state{ &keyboard, &mouse };
    OrbitCameraController controller(camera, state);

    // Case 1: Mouse moves with NO button pressed -> camera must NOT rotate
    mouse.SetDelta(50.0f, 20.0f);
    controller.Update(0.016f);
    glm::vec3 posUnpressed = camera.GetPosition();
    assert(Vec3Equal(posUnpressed, glm::vec3(0.0f, 0.0f, 5.0f)));

    // Case 2: Mouse Left button pressed + mouse delta -> camera MUST rotate
    mouse.NotifyButtonDown(MouseButton::ButtonLeft);
    mouse.SetDelta(10.0f, 0.0f); // Drag horizontally
    controller.Update(0.016f);
    controller.Update(0.016f); // The team controller enters the gesture at the end of Update.
    glm::vec3 posRotated = camera.GetPosition();
    assert(!Vec3Equal(posRotated, posUnpressed));

    // Case 3: Continuous dragging across frames (Held state test)
    mouse.Update(); // End of Frame 1
    // In Frame 2, button is still held, mouse moves further
    assert(mouse.CheckIsPressed(MouseButton::ButtonLeft) == true);
    mouse.SetDelta(10.0f, 0.0f);
    controller.Update(0.016f);
    glm::vec3 posRotated2 = camera.GetPosition();
    assert(!Vec3Equal(posRotated2, posRotated)); // Rotated even further!

    // Case 4: Zoom via mouse scroll
    mouse.SetScroll(0.0f, 2.0f); // Scroll up
    controller.Update(0.016f);
    // Camera zoomed in -> distance reduced
    assert(glm::length(camera.GetPosition()) < glm::length(posRotated2));

    // Case 5: Null mouse safety
    InputState nullState{ nullptr, nullptr };
    OrbitCameraController nullController(camera, nullState);
    nullController.Update(0.016f); // Must not crash!

    std::cout << "  -> OrbitCameraController PASS!\n";
}

void TestWxInputStateMachine() {
    std::cout << "[RUN] Testing WxInput State Machine...\n";

    // --- Mouse Tests ---
    WxMouseInput mouse;

    // Initially idle
    assert(mouse.CheckIsPressed(MouseButton::ButtonLeft) == false);
    assert(mouse.CheckIsJustPressed(MouseButton::ButtonLeft) == false);
    assert(mouse.CheckIsReleased(MouseButton::ButtonLeft) == true);

    // Frame 1: Button Pressed down
    mouse.NotifyButtonDown(MouseButton::ButtonLeft);
    assert(mouse.CheckIsPressed(MouseButton::ButtonLeft) == true);
    assert(mouse.CheckIsJustPressed(MouseButton::ButtonLeft) == true);
    assert(mouse.GetMouseButtonState(MouseButton::ButtonLeft) == MouseButtonState::Pressed);

    // Frame 1 ends: Update() called
    mouse.SetDelta(12.0f, -8.0f);
    mouse.SetScroll(0.0f, 1.0f);
    mouse.Update();

    // Frame 2: Still held!
    assert(mouse.CheckIsPressed(MouseButton::ButtonLeft) == true);      // Must STILL be true!
    assert(mouse.CheckIsJustPressed(MouseButton::ButtonLeft) == false); // Not "just pressed" anymore!
    assert(mouse.GetDeltaX() == 0.0f); // Delta reset
    assert(mouse.GetScrollY() == 0.0f); // Scroll reset

    // Frame 2: Button Released
    mouse.NotifyButtonUp(MouseButton::ButtonLeft);
    assert(mouse.CheckIsPressed(MouseButton::ButtonLeft) == false);
    assert(mouse.CheckIsReleased(MouseButton::ButtonLeft) == true);
    assert(mouse.GetMouseButtonState(MouseButton::ButtonLeft) == MouseButtonState::Released);

    // Frame 2 ends: Update() called
    mouse.Update();
    assert(mouse.CheckIsPressed(MouseButton::ButtonLeft) == false);
    assert(mouse.GetMouseButtonState(MouseButton::ButtonLeft) == MouseButtonState::None);

    // --- Keyboard Tests ---
    WxKeyboardInput keyboard;
    assert(keyboard.CheckIsPressed(KeyboardKey::Space) == false);

    // Key Down
    keyboard.NotifyKeyDown(32); // ' ' ASCII = Space
    assert(keyboard.CheckIsPressed(KeyboardKey::Space) == true);
    assert(keyboard.CheckIsJustPressed(KeyboardKey::Space) == true);

    keyboard.Update();
    // Still held next frame
    assert(keyboard.CheckIsPressed(KeyboardKey::Space) == true);
    assert(keyboard.CheckIsJustPressed(KeyboardKey::Space) == false);

    // Key Up
    keyboard.NotifyKeyUp(32);
    assert(keyboard.CheckIsPressed(KeyboardKey::Space) == false);
    assert(keyboard.CheckIsReleased(KeyboardKey::Space) == true);

    keyboard.Update();
    assert(keyboard.GetKeyState(KeyboardKey::Space) == KeyState::None);

    std::cout << "  -> WxInput State Machine PASS!\n";
}

// Mock handler to test Chain of Responsibility forwarding
class MockFallbackHandler : public ModelFormatHandler {
public:
    bool wasCalled = false;
    std::string handledPath;

    RenderModel* Handle(const char* file) override {
        wasCalled = true;
        handledPath = file;
        return nullptr;
    }
};

void TestGltfFormatChain() {
    std::cout << "[RUN] Testing GltfModelFormatHandler Chain of Responsibility...\n";

    // Test chain delegation when non-gltf file is passed
    // GltfModelFormatHandler calls Next(file) for non-gltf files
    GltfModelFormatHandler gltfHandler(nullptr);
    MockFallbackHandler fallback;
    gltfHandler.SetNext(&fallback);

    // Non-gltf file should bypass gltf handler and reach fallback handler
    gltfHandler.Handle("house.obj");
    assert(fallback.wasCalled == true);
    assert(fallback.handledPath == "house.obj");

    fallback.wasCalled = false;
    gltfHandler.Handle("scene.fbx");
    assert(fallback.wasCalled == true);
    assert(fallback.handledPath == "scene.fbx");

    std::cout << "  -> GltfModelFormatHandler Chain PASS!\n";
}

#ifndef TEST_ASSET_DIR
#define TEST_ASSET_DIR "tests/assets"
#endif

class TestTexture : public CoreEngine::Renderer::ITexture {
public:
    void Config(const CoreEngine::Renderer::TextureConfiguration&) override {}
    CoreEngine::Renderer::TextureID GetTextureId() const override { return 1; }
};

class TestResources : public CoreEngine::Renderer::IResourceManager {
public:
    int externalCount = 0, embeddedCount = 0;
    TestTexture texture;
    CoreEngine::Renderer::IVertexBuffer* GetVertexBuffer(unsigned int) override { return nullptr; }
    CoreEngine::Renderer::IIndexBuffer* GetIndexBuffer(unsigned int) override { return nullptr; }
    CoreEngine::Renderer::IShader* GetShader(unsigned int) override { return nullptr; }
    CoreEngine::Renderer::ITexture* GetTexture(unsigned int) override { return nullptr; }
    CoreEngine::Renderer::ICubeMap* GetCubeMap(unsigned int) override { return nullptr; }
    CoreEngine::Renderer::IUniformBuffer* GetUniformBuffer(unsigned int) override { return nullptr; }
    CoreEngine::Renderer::IVertexBuffer* CreateVertexBuffer() override { return nullptr; }
    CoreEngine::Renderer::IIndexBuffer* CreateIndexBuffer() override { return nullptr; }
    CoreEngine::Renderer::IShader* CreateShaderFromSources(const std::string&, const std::string&) override { return nullptr; }
    CoreEngine::Renderer::IShader* CreateShaderFromFiles(const std::string&, const std::string&) override { return nullptr; }
    CoreEngine::Renderer::ICubeMap* CreateCubeMap(const CoreEngine::Renderer::CubemapTextureFiles&) override { return nullptr; }
    CoreEngine::Renderer::IUniformBuffer* CreateUniformBuffer() override { return nullptr; }
    CoreEngine::Renderer::IFrameBuffer* CreateColorFrameBuffer(unsigned int, unsigned int) override { return nullptr; }
    CoreEngine::Renderer::ITexture* CreateTexture(const std::string& path) override {
        assert(std::filesystem::exists(path));
        assert(std::filesystem::path(path).filename() == "texture.png");
        ++externalCount;
        return &texture;
    }
    CoreEngine::Renderer::ITexture* CreateTextureFromMemory(const unsigned char* data, unsigned int size) override {
        assert(size > 8 && data[0] == 0x89 && data[1] == 'P' && data[2] == 'N' && data[3] == 'G');
        ++embeddedCount;
        return &texture;
    }
};

void TestRealGltfImports() {
    std::cout << "[RUN] Reading real GLTF and GLB assets...\n";
    TestResources resources;
    GltfModelFormatHandler handler(&resources);
    ModelImporter importer(&handler);
    auto load = [&](const char* name) {
        return importer.Import((std::filesystem::path(TEST_ASSET_DIR) / name).string().c_str());
    };
    for (const char* name : {"triangle.gltf", "textured.gltf", "embedded.glb"}) {
        auto model = load(name);
        assert(model && model->GetMeshes().size() == 1);
        const auto& mesh = model->GetMeshes().front();
        assert(mesh.Vertices.size() == 3 && mesh.Indices.size() == 3);
        Model::AssimpBoundingBox bounds;
        for (const auto& v : mesh.Vertices) {
            bounds.expand(v.Position);
            assert(std::isfinite(v.Normal.x) && FloatEqual(glm::length(v.Normal), 1.0f));
        }
        assert(Vec3Equal(bounds.getMin(), {2, 3, 4}));
        assert(Vec3Equal(bounds.getMax(), {4, 5, 4}));
        // Every corner must fit inside the projected viewport, including a tall viewport.
        auto fit = Model::FitCameraToBox(bounds, 45, 0.4f);
        for (const auto& v : mesh.Vertices) {
            glm::vec4 clip = fit.ViewProjection() * glm::vec4(v.Position, 1);
            glm::vec3 ndc = glm::vec3(clip) / clip.w;
            assert(std::abs(ndc.x) < 1 && std::abs(ndc.y) < 1 && std::abs(ndc.z) < 1);
        }
    }
    assert(resources.externalCount == 1 && resources.embeddedCount == 1);
    resources.embeddedCount = 0;
    auto shared = load("shared-image.glb");
    assert(shared && shared->GetMeshes().size() == 2);
    assert(resources.embeddedCount == 1); // Shared images must be uploaded once per import.
    shared = load("shared-image.glb");
    assert(shared && resources.embeddedCount == 2); // Cache keys must not outlive an Assimp scene.
    assert(!load("invalid.gltf"));
    assert(!load("missing.glb"));
    assert(!load("unknown.obj"));
    assert(!importer.Import(nullptr));
    assert(!importer.Import(""));
    MockFallbackHandler fallback;
    handler.SetNext(&fallback);
    handler.Handle("missing.GLTF");
    assert(!fallback.wasCalled); // Uppercase extensions route through the importer.
    ModelImporter empty(nullptr);
    assert(!empty.Import("triangle.gltf"));
    std::cout << "  -> Real GLTF/GLB imports PASS!\n";
}

void TestCameraBoundariesAndKeyboard() {
    OrbitCamera camera(glm::vec3(0), 5);
    auto projection = camera.GetProjectionMatrix();
    camera.SetViewportSize(100, 0);
    assert(camera.GetProjectionMatrix() == projection);
    camera.SetZoomLimits(0.001f, 1e6f);
    camera.SetDistance(10000);
    camera.CaptureAsInitial();
    camera.Zoom(5000);
    camera.SetTarget({1, 2, 3});
    camera.Reset();
    assert(FloatEqual(camera.GetDistance(), 10000) && Vec3Equal(camera.GetTarget(), {0, 0, 0}));
    bool rejected = false;
    try { camera.SetZoomLimits(10, 1); } catch (const std::invalid_argument&) { rejected = true; }
    assert(rejected);
    OrbitCamera limited({0, 0, 0}, 5);
    limited.SetZoomLimits(10, 20);
    limited.Reset();
    assert(FloatEqual(limited.GetDistance(), 10));
    limited.CaptureAsInitial();
    limited.SetZoomLimits(20, 30);
    limited.Reset();
    assert(FloatEqual(limited.GetDistance(), 20));
    camera.SetPerspective(0, 0.1f, 10); // Team API ignores invalid projection parameters.
    assert(camera.GetProjectionMatrix() == projection);

    WxKeyboardInput keyboard;
    InputState input{&keyboard, nullptr};
    OrbitCamera one(glm::vec3(0), 5), split(glm::vec3(0), 5);
    OrbitCameraController oneController(one, input), splitController(split, input);
    keyboard.NotifyKeyDown(WXK_RIGHT);
    oneController.Update(0);
    splitController.Update(0);
    oneController.Update(1);
    splitController.Update(0.5f);
    splitController.Update(0.5f);
    assert(Vec3Equal(one.GetPosition(), split.GetPosition()));
    assert(!Vec3Equal(one.GetPosition(), {0, 0, 5}));
    keyboard.NotifyKeyUp(WXK_RIGHT);
    keyboard.NotifyKeyDown(WXK_PAGEUP);
    oneController.Update(0);
    auto distance = one.GetDistance();
    oneController.Update(0.5f);
    assert(one.GetDistance() < distance);
    keyboard.NotifyKeyUp(WXK_PAGEUP);
    keyboard.NotifyKeyDown('D');
    oneController.Update(0);
    auto before = one.GetTarget();
    oneController.Update(0.5f);
    assert(glm::dot(one.GetTarget() - before, one.GetRight()) > 0);
    keyboard.NotifyKeyUp('D');
    keyboard.Update();
    before = one.GetTarget();
    oneController.Update(0.5f);
    assert(Vec3Equal(one.GetTarget(), before));
    std::cout << "  -> Camera boundaries and keyboard PASS!\n";
}

void TestCameraBindingsAndGestures() {
    using Action = Domain::Camera::OrbitCamera::OrbitCameraAction;
    // All keyboard directions use the same state machine and preserve time scaling.
    const int keys[] = {WXK_LEFT, WXK_RIGHT, WXK_UP, WXK_DOWN, 'A', 'D', 'W', 'S', WXK_PAGEUP, WXK_PAGEDOWN};
    for (int key : keys) {
        WxKeyboardInput keyboard;
        keyboard.NotifyKeyDown(key);
        InputState input{&keyboard, nullptr};
        OrbitCamera one({0, 0, 0}, 5), split({0, 0, 0}, 5);
        OrbitCameraController first(one, input), second(split, input);
        first.Update(0); second.Update(0);
        first.Update(0.2f); second.Update(0.1f); second.Update(0.1f);
        assert(!Vec3Equal(one.GetPosition(), {0, 0, 5}));
        assert(Vec3Equal(one.GetPosition(), split.GetPosition()));
        keyboard.NotifyKeyUp(key);
        auto position = one.GetPosition();
        first.Update(0.2f);
        assert(Vec3Equal(position, one.GetPosition()));
    }

    WxKeyboardInput keyboard;
    WxMouseInput mouse;
    InputState input{&keyboard, &mouse};
    OrbitCamera camera({0, 0, 0}, 5);
    OrbitCameraController controller(camera, input);
    controller.GetInputMap().BindAction(Action::OrbitRight, KeyboardKey::L);
    keyboard.NotifyKeyDown('L');
    controller.Update(0);
    mouse.SetDelta(100, 100); // Hover must not become a drag during keyboard control.
    controller.Update(0.1f);
    assert(FloatEqual(camera.GetAzimuth(), 0.1f) && FloatEqual(camera.GetElevation(), 0));
    keyboard.Update(); mouse.Update();

    // A newly pressed pan gesture overrides orbit; a held orbit resumes after releasing pan.
    mouse.NotifyButtonDown(MouseButton::ButtonRight);
    controller.Update(0);
    auto azimuth = camera.GetAzimuth();
    mouse.SetDelta(10, 0);
    controller.Update(0.1f);
    assert(FloatEqual(camera.GetAzimuth(), azimuth));
    assert(!Vec3Equal(camera.GetTarget(), {0, 0, 0}));
    mouse.NotifyButtonUp(MouseButton::ButtonRight);
    mouse.Update();
    controller.Update(0); // Pan -> Idle
    controller.Update(0); // Idle -> held Orbit
    controller.Update(0.1f);
    assert(camera.GetAzimuth() > azimuth);
    keyboard.NotifyKeyUp('L');
    keyboard.NotifyKeyDown('R');
    controller.Update(0);
    assert(Vec3Equal(camera.GetPosition(), {0, 0, 5}));

    auto map = Domain::Camera::OrbitCamera::CreateDefaultOrbitCameraInputMap();
    InputState empty;
    assert(!map.IsJustPressed(Action::Reset, empty));
    assert(!map.IsContinuousPressed(Action::Orbit, empty));
    assert(!map.IsReleased(Action::Reset, empty));
    std::cout << "  -> Camera bindings and gesture arbitration PASS!\n";
}

int main() {
    std::cout << "========================================\n";
    std::cout << "   RUNNING UNIT TESTS FOR TASKS LOGIC   \n";
    std::cout << "========================================\n";

    TestOrbitCamera();
    TestOrbitCameraController();
    TestWxInputStateMachine();
    TestGltfFormatChain();
    TestRealGltfImports();
    TestCameraBoundariesAndKeyboard();
    TestCameraBindingsAndGestures();

    std::cout << "========================================\n";
    std::cout << "   ALL LOGIC TESTS PASSED SUCCESSFULLY! \n";
    std::cout << "========================================\n";
    return 0;
}
