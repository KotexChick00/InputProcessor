cmake_minimum_required(VERSION 3.21)
project(InputProcessor LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
if(MSVC)
    add_compile_options(/utf-8)
endif()

# Fail early with a readable message instead of cryptic compile/link errors.
if(WIN32 AND NOT MSVC AND VCPKG_TARGET_TRIPLET AND NOT VCPKG_TARGET_TRIPLET MATCHES "mingw")
    message(FATAL_ERROR
        "vcpkg triplet '${VCPKG_TARGET_TRIPLET}' builds libraries with MSVC, but the compiler is "
        "${CMAKE_CXX_COMPILER_ID} (${CMAKE_CXX_COMPILER}).\n"
        "Use the vcpkg-debug/vcpkg-release preset from Visual Studio, VS Code, or "
        "'Developer PowerShell for VS', then delete the build folder and configure again.")
endif()
include(CheckCXXSourceCompiles)
check_cxx_source_compiles("#include <format>\nint main() { return std::format(\"{}\", 1).size() == 1 ? 0 : 1; }"
    INPUTPROCESSOR_HAS_STD_FORMAT)
if(NOT INPUTPROCESSOR_HAS_STD_FORMAT)
    message(FATAL_ERROR
        "${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION} does not provide C++20 <format>. "
        "Use Visual Studio 2022/2026 (MSVC) or GCC 13+, then delete the build folder and configure again.")
endif()

find_package(OpenGL REQUIRED)
find_package(wxWidgets 3.2 CONFIG QUIET)
if(TARGET wx::core)
    set(WX_LIBRARIES wx::core wx::base wx::gl)
else()
    find_package(wxWidgets 3.2 REQUIRED COMPONENTS core base gl)
    include(${wxWidgets_USE_FILE})
    set(WX_LIBRARIES ${wxWidgets_LIBRARIES})
endif()
find_package(glfw3 CONFIG REQUIRED)
find_package(assimp CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_path(GLM_INCLUDE_DIR glm/glm.hpp REQUIRED)
find_path(STB_INCLUDE_DIR stb_image.h PATH_SUFFIXES stb REQUIRED)

file(GLOB ENGINE_SOURCES CONFIGURE_DEPENDS
    Core/Application/*.cpp Core/Input/*/*.cpp Core/Window/*/*.cpp
    Core/Renderer/Opengl/*.cpp Core/Logger/*.cpp Core/Logger/SpdLog/*.cpp
    Domain/Camera/OrbitCamera/*.cpp Domain/Camera/OrbitCamera/States/*.cpp
    Domain/Model/Importers/AssimpModelImporter/*.cpp)
add_library(engine STATIC ${ENGINE_SOURCES} Core/Window/WindowPlatformFactory.cpp
    Core/Renderer/RendererFactory.cpp Domain/Model/RenderModel.cpp Domain/Model/AdsMaterial.cpp Library/Glad/src/glad.c)
target_include_directories(engine PUBLIC Core Domain Library/Glad ${GLM_INCLUDE_DIR} PRIVATE ${STB_INCLUDE_DIR})
target_link_libraries(engine PUBLIC ${WX_LIBRARIES} OpenGL::GL glfw assimp::assimp spdlog::spdlog ${CMAKE_DL_LIBS})
add_executable(InputProcessor examples/gltf_viewer.cpp)
target_link_libraries(InputProcessor PRIVATE engine)
target_compile_definitions(InputProcessor PRIVATE INPUTPROCESSOR_SAMPLE_MODEL="${CMAKE_CURRENT_SOURCE_DIR}/tests/assets/textured.gltf")
add_executable(DomainDemo main.cpp)
target_link_libraries(DomainDemo PRIVATE engine)

enable_testing()
foreach(test_name IN ITEMS test_engine_logic test_wx_integration)
    add_executable(${test_name} tests/${test_name}.cpp)
    target_link_libraries(${test_name} PRIVATE engine)
    target_compile_definitions(${test_name} PRIVATE TEST_ASSET_DIR="${CMAKE_CURRENT_SOURCE_DIR}/tests/assets")
    if(MSVC)
        target_compile_options(${test_name} PRIVATE /UNDEBUG)
    else()
        target_compile_options(${test_name} PRIVATE -UNDEBUG)
    endif()
endforeach()
add_test(NAME engine_logic COMMAND test_engine_logic)
add_test(NAME wx_integration COMMAND test_wx_integration)
add_test(NAME gltf_viewer COMMAND InputProcessor "${CMAKE_CURRENT_SOURCE_DIR}/tests/assets/textured.gltf" --frames 3)
add_test(NAME glb_viewer COMMAND InputProcessor "${CMAKE_CURRENT_SOURCE_DIR}/tests/assets/embedded.glb" --frames 3)
add_test(NAME sample_viewer COMMAND InputProcessor --frames 3)
set_tests_properties(engine_logic wx_integration gltf_viewer glb_viewer sample_viewer PROPERTIES TIMEOUT 30)
