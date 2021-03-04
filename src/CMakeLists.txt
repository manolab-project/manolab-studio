cmake_minimum_required(VERSION 3.5)

project(manolab-gui LANGUAGES C CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_VERBOSE_MAKEFILE on )

if (WIN32)
    link_directories(${CMAKE_CURRENT_SOURCE_DIR}/windows/SDL2/lib/x64)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/windows/SDL2/include)

endif (WIN32)


if (MSVC)
     add_compile_options("/FS")
endif (MSVC)

# The second parameter is output directory for the results of the targets from that subdirectory.
add_subdirectory(manolab)
add_subdirectory(manolab/lib/icl)

find_package(OpenGL REQUIRED)


if(UNIX AND NOT APPLE)
    add_definitions(-DUSE_UNIX_OS)
endif()

if(WIN32)
    add_definitions(-DUSE_WINDOWS_OS)
endif()

add_definitions(-DUNICODE)

set(SRCS
    gl3w/GL/gl3w.c
    main.cpp
    WindowBase.h
    ConsoleWindow.cpp
    ConsoleWindow.h

    TaskListWindow.cpp
    TaskListWindow.h

    ImageWindow.cpp
    ImageWindow.h

    MainWindow.cpp
    MainWindow.h

    TableWindow.cpp
    TableWindow.h

    Gui.h
    Gui.cpp

    CodeEditor.cpp
    CodeEditor.h

    imgui/imgui.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_tables.cpp
    imgui/imgui_widgets.cpp
    imgui/backends/imgui_impl_sdl.cpp
    imgui/backends/imgui_impl_opengl3.cpp
    ImGuiColorTextEdit/TextEditor.cpp
    ImGuiColorTextEdit/TextEditor.h
    ImGuiFileDialog/ImGuiFileDialog.cpp
    
)

if(WIN32)
    list(APPEND
        SRCS
        manolab/icon.rc
    )
endif()

add_executable(manolab-gui
    ${SRCS}
)

# add_definitions(-DUSE_UNIX_OS)
add_link_options(-static-libgcc -static-libstdc++)

add_definitions(-DIMGUI_IMPL_OPENGL_LOADER_GL3W)
add_definitions(-Dcimg_display=0)

if(LINUX)
target_link_libraries(manolab-gui
    manolab
    icl
    SDL2
    pthread
    udev
    OpenGL::GL
)
elseif(WIN32)
target_link_libraries(manolab-gui
    manolab
    icl
    SDL2
    SDL2main.lib
    OpenGL::GL
    ws2_32.lib psapi.lib setupapi.lib cfgmgr32.lib advapi32.lib Dbghelp.lib
)
set_target_properties(manolab-gui PROPERTIES
LINK_FLAGS /SUBSYSTEM:CONSOLE
)   
endif()

target_include_directories(
    manolab-gui
    PUBLIC
    /usr/include/SDL2
    ${CMAKE_CURRENT_SOURCE_DIR}/gl3w
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends
    ${CMAKE_CURRENT_SOURCE_DIR}/ImGuiColorTextEdit
    ${CMAKE_CURRENT_SOURCE_DIR}/ImGuiFileDialog
    ${CMAKE_CURRENT_SOURCE_DIR}/manolab
)


