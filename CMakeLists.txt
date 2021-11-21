cmake_minimum_required(VERSION 3.15)
project(vulkan_test)

set(CMAKE_CXX_STANDARD 20)

# Set the name of the ouput binary
set(
        BIN_NAME
        "vulkan_test"
)

# Add all files from a src folder
file(GLOB SOURCES "src/*.cpp")

find_package(Vulkan REQUIRED FATAL_ERROR)
find_program(glslc_executable NAMES glslc HINTS Vulkan::glslc)

set(SHADER_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/shaders)
set(SHADER_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/src/shaders)


file(GLOB SHADERS
        ${SHADER_SOURCE_DIR}/*.vert
        ${SHADER_SOURCE_DIR}/*.frag
        ${SHADER_SOURCE_DIR}/*.comp
        ${SHADER_SOURCE_DIR}/*.geom
        ${SHADER_SOURCE_DIR}/*.glsl
        )

add_custom_command(
        COMMAND
        ${CMAKE_COMMAND} -E make_directory ${SHADER_BINARY_DIR}
        OUTPUT ${SHADER_BINARY_DIR}
        COMMENT "Creating ${SHADER_BINARY_DIR}"
)


foreach (source IN LISTS SHADERS)
    get_filename_component(FILENAME ${source} NAME)
    add_custom_command(
            COMMAND
            ${glslc_executable}
            -MD -MF ${SHADER_BINARY_DIR}/${FILENAME}.d
            -o ${SHADER_BINARY_DIR}/${FILENAME}.spv
            ${source}
            OUTPUT ${SHADER_BINARY_DIR}/${FILENAME}.spv
            DEPENDS ${source} ${SHADER_BINARY_DIR}
            COMMENT "Compiling ${FILENAME}"
    )
    list(APPEND SPV_SHADERS ${SHADER_BINARY_DIR}/${FILENAME}.spv)
endforeach ()

add_custom_target(shaders ALL DEPENDS ${SPV_SHADERS})

# Setup binary output
add_executable(${BIN_NAME} ${SOURCES} ${SPV_SHADERS})

set(CMAKE_CXX_FLAGS_DEBUG_INIT "-Wall")

# Link dependencies
target_link_libraries(${BIN_NAME} glfw)
target_link_libraries(${BIN_NAME} vulkan)