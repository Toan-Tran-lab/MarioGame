cmake_minimum_required(VERSION 3.16)
project(MarioGame VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)

find_package(raylib QUIET)
if(NOT raylib_FOUND)
    FetchContent_Declare(
        raylib
        GIT_REPOSITORY https://github.com/raysan5/raylib.git
        GIT_TAG 5.5
    )
    FetchContent_MakeAvailable(raylib)
endif()

FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
)
FetchContent_MakeAvailable(nlohmann_json)

file(GLOB_RECURSE ALL_SRCS CONFIGURE_DEPENDS "src/*.cpp")
file(GLOB_RECURSE PHYSICS_SRCS CONFIGURE_DEPENDS "src/physics/*.cpp")

list(REMOVE_ITEM ALL_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp")

# --- Main Game Target ---
add_executable(${PROJECT_NAME} ${ALL_SRCS} src/main.cpp)
target_include_directories(${PROJECT_NAME} PRIVATE src ${CMAKE_SOURCE_DIR}/..)
target_link_libraries(${PROJECT_NAME} PRIVATE raylib nlohmann_json::nlohmann_json)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/assets
)

# --- Physics Sandbox Target ---
add_executable(PhysicsSandbox
    sandbox/SandboxMain.cpp
    ${PHYSICS_SRCS}
    src/World/TileMap.cpp
    src/TextureManager/TextureManager.cpp
)

target_include_directories(PhysicsSandbox PRIVATE src)
target_link_libraries(PhysicsSandbox PRIVATE raylib nlohmann_json::nlohmann_json)
