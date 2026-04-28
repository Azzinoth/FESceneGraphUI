cmake_minimum_required(VERSION 3.10)
project(FESceneGraphUI)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

option(FE_SCENE_GRAPH_UI_BUILD_SHARED_LIBS "Build FESceneGraphUI as a shared library" OFF)
option(FE_SCENE_GRAPH_UI_USE_STATIC_RUNTIME "Use static runtime (/MT) instead of dynamic (/MD) for FESceneGraphUI" ON)
option(FE_SCENE_GRAPH_UI_WITH_FOCAL_BACKEND "Include the default Focal Engine backend" OFF)

set(DEAR_IMGUI_INCLUDE_DIR "" CACHE PATH "Path to Dear ImGui directory.")

if(NOT DEAR_IMGUI_INCLUDE_DIR)
    message(FATAL_ERROR
        "FESceneGraphUI requires DEAR_IMGUI_INCLUDE_DIR to be set "
        "(path to the Dear ImGui source directory).")
endif()

# Turn on the ability to create folders to organize projects (.vcproj)
# It creates "CMakePredefinedTargets" folder by default and adds CMake
# defined projects like INSTALL.vcproj and ZERO_CHECK.vcproj
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if(MSVC)
    add_compile_options(/wd4251)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
endif()

file(GLOB FESceneGraphUI_CORE_SRC
    "BackendInterface.cpp"
    "BackendInterface.h"
	"TreeView.cpp"
	"TreeView.h"
)

if(FE_SCENE_GRAPH_UI_BUILD_SHARED_LIBS)
    add_library(FESceneGraphUI SHARED ${FESceneGraphUI_CORE_SRC})
    target_compile_definitions(FESceneGraphUI PRIVATE FESCENEGRAPHUI_EXPORTS)
    target_compile_definitions(FESceneGraphUI PUBLIC FESCENEGRAPHUI_SHARED)

    # Sometimes /GL conflicts with WINDOWS_EXPORT_ALL_SYMBOLS
	set_target_properties(FESceneGraphUI PROPERTIES COMPILE_OPTIONS "/GL-")
	set_target_properties(FESceneGraphUI PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)

    # Disable C4251 warning
    target_compile_options(FESceneGraphUI PRIVATE /wd4251)

    option(FE_SCENE_GRAPH_UI_USE_EXTERNAL_IMGUI "Link against an external ImGui library instead of vendoring sources" OFF)

    if(FE_SCENE_GRAPH_UI_USE_EXTERNAL_IMGUI)
        set(EXTERNAL_IMGUI_LIBRARY "" CACHE FILEPATH "Path to external ImGui library")
        if(NOT EXTERNAL_IMGUI_LIBRARY)
            message(FATAL_ERROR
                "EXTERNAL_IMGUI_LIBRARY must be set when "
                "FE_SCENE_GRAPH_UI_USE_EXTERNAL_IMGUI is ON")
        endif()
        target_link_libraries(FESceneGraphUI PRIVATE ${EXTERNAL_IMGUI_LIBRARY})
    else()
        # Get ImGui sources into the DLL.
        set(FE_SCENE_GRAPH_UI_GLFW_INCLUDE_DIR "" CACHE PATH "Path to GLFW include directory.")
        set(FE_SCENE_GRAPH_UI_GLFW_LIBRARY     "" CACHE PATH "Path to GLFW library.")
        target_include_directories(FESceneGraphUI PRIVATE ${FE_SCENE_GRAPH_UI_GLFW_INCLUDE_DIR})
        target_link_libraries(FESceneGraphUI PRIVATE ${FE_SCENE_GRAPH_UI_GLFW_LIBRARY})

        file(GLOB IMGUI_SOURCES
            "${DEAR_IMGUI_INCLUDE_DIR}/*.cpp"
            "${DEAR_IMGUI_INCLUDE_DIR}/*.h")
        target_sources(FESceneGraphUI PRIVATE ${IMGUI_SOURCES})
        source_group("Source Files/ThirdParty/ImGui" FILES ${IMGUI_SOURCES})
    endif()
else()
    add_library(FESceneGraphUI STATIC ${FESceneGraphUI_CORE_SRC})
endif()

if(FE_SCENE_GRAPH_UI_WITH_FOCAL_BACKEND)
    # With that option FESceneGraphUI requires FocalEngine to already exist as a target.
    if(NOT TARGET FocalEngine)
        message(FATAL_ERROR
            "FE_SCENE_GRAPH_UI_WITH_FOCAL_BACKEND=ON requires the FocalEngine "
            "target. Add FocalEngine via add_subdirectory() before this one.")
    endif()

    # Header-only backend, INTERFACE library just propagates deps.
    add_library(FESceneGraphUI_FocalBackend INTERFACE)
    target_sources(FESceneGraphUI_FocalBackend INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/FocalEngineBackend.h)
    target_link_libraries(FESceneGraphUI_FocalBackend INTERFACE FESceneGraphUI FocalEngine)
    target_include_directories(FESceneGraphUI_FocalBackend INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
endif()

source_group("Source Files" FILES ${FESceneGraphUI_CORE_SRC})

include_directories(
	${DEAR_IMGUI_INCLUDE_DIR}
)

# Expose our own include directory to consumers
target_include_directories(FESceneGraphUI PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Export for parent scope
set(FE_SCENE_GRAPH_UI_DIR ${CMAKE_CURRENT_SOURCE_DIR} PARENT_SCOPE)

add_custom_command(TARGET FESceneGraphUI PRE_BUILD
    COMMAND ${CMAKE_COMMAND}
        -D PROJECT_VERSION_PREFIX=FE_SCENE_GRAPH_UI_
        -D PROJECT_VERSION_MAJOR=0
        -D PROJECT_VERSION_MINOR=0
        -D PROJECT_VERSION_PATCH=7
        -D PROJECT_VERSION_DIR=${CMAKE_CURRENT_SOURCE_DIR}/VersionInfo
        -P ${CMAKE_CURRENT_SOURCE_DIR}/VersionInfo/UpdateProjectVersion.cmake
)