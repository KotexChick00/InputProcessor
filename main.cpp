#include <iostream>
// GLAD must be included before GLFW
#include <glad/glad.h>
#include <GLFW/glfw3.h>

int main() {
    // 1. Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    // 2. Configure GLFW (Requesting OpenGL 3.3 Core Profile as a baseline)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // 3. Create a window and its OpenGL context
    GLFWwindow* window = glfwCreateWindow(800, 600, "OpenGL Version Check", NULL, NULL);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    // Make the context current (CRITICAL: glGetString won't work without this)
    glfwMakeContextCurrent(window);

    // 4. Load all OpenGL function pointers using GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // 5. Query and print the working OpenGL version
    // Method A: Get the full version string (e.g., "4.6.0 NVIDIA 555.99")
    const GLubyte* versionString = glGetString(GL_VERSION);
    std::cout << "Working OpenGL Version (String): " << versionString << std::endl;

    // Method B: Get integers for major/minor versions (Requires OpenGL 3.0+)
    GLint major, minor;
    glGetIntegerv(GL_MAJOR_VERSION, &major);
    glGetIntegerv(GL_MINOR_VERSION, &minor);
    std::cout << "Working OpenGL Version (Ints): " << major << "." << minor << std::endl;

    // 6. Clean up and exit
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
