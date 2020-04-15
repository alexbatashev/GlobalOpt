cmake_minimum_required(VERSION 3.16)
project(OptMethLab)

set(CMAKE_CXX_STANDARD 17)

option(USE_OPENGL ON)

if (USE_OPENGL)
    message(STATUS "Using OpenGL")
    #find_package(PkgConfig REQUIRED)
    #pkg_search_module(GLFW REQUIRED glfw3)
    find_package(glfw3 3.3 REQUIRED)
    set(LIBS ${GLUT_LIBRARY} glfw)
    set(SRCS
            ${CMAKE_SOURCE_DIR}/contrib/glad/src/glad.c
            ${CMAKE_SOURCE_DIR}/contrib/imgui/examples/imgui_impl_glfw.cpp
            ${CMAKE_SOURCE_DIR}/contrib/imgui/examples/imgui_impl_opengl3.cpp
            ${CMAKE_SOURCE_DIR}/main_opengl.cpp)
    include_directories(${GLFW_INCLUDE_DIRS} contrib/glad/include)
    add_definitions(-DIMGUI_IMPL_OPENGL_LOADER_GLAD=1)
endif()

add_executable(OptMethLab
        gui.cpp
        optimizer.cpp
        ${CMAKE_SOURCE_DIR}/contrib/imgui/imgui.cpp
        ${CMAKE_SOURCE_DIR}/contrib/imgui/imgui_draw.cpp
        ${CMAKE_SOURCE_DIR}/contrib/imgui/imgui_widgets.cpp
        ${CMAKE_SOURCE_DIR}/contrib/imgui_plot.cpp
        ${SRCS})

find_package(Threads REQUIRED)

target_link_libraries(OptMethLab PRIVATE ${LIBS} Threads::Threads)
if(UNIX)
    target_link_libraries(OptMethLab PRIVATE -Wl,-ldl)
endif()
target_include_directories(OptMethLab PRIVATE ${CMAKE_SOURCE_DIR}/contrib ${CMAKE_SOURCE_DIR}/contrib/imgui ${CMAKE_SOURCE_DIR}/contrib/imgui/examples ${CMAKE_SOURCE_DIR}/contrib/ChaiScript/include)
