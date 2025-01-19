cmake_minimum_required(VERSION 3.11.0)

set(NAME MvGame)
set(CMAKE_CXX_STANDARD 23)

project(${NAME})

# Vulkan and glslang
find_package(Vulkan COMPONENTS shaderc_combined REQUIRED)



add_subdirectory("vendor/glfw")
add_subdirectory("vendor/glm")

file(GLOB_RECURSE SOURCES source/*.cpp)
file(GLOB_RECURSE HEADERS source/include/*.h source/include/*.hpp)

message(STATUS "Sources: ${SOURCES} Headers: ${HEADERS}")

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})
target_include_directories(
            ${PROJECT_NAME} PRIVATE
        source/include
            vendor/stb
)
target_link_libraries(${PROJECT_NAME} glfw glm Vulkan::Vulkan Vulkan::shaderc_combined)

add_custom_command(TARGET ${NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/textures $<TARGET_FILE_DIR:${NAME}>/textures)

add_custom_command(TARGET ${NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/shaders $<TARGET_FILE_DIR:${NAME}>/shaders)