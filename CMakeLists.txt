cmake_minimum_required(VERSION 3.10)
project(FESceneGraphUI)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

option(FE_SCENE_GRAPH_UI_BUILD_SHARED_LIBS "Build FESceneGraphUI as a shared library" OFF)
option(FE_SCENE_GRAPH_UI_USE_STATIC_RUNTIME "Use static runtime (/MT) instead of dynamic (/MD) for FESceneGraphUI" ON)

# Turn on the ability to create folders to organize projects (.vcproj)
# It creates "CMakePredefinedTargets" folder by default and adds CMake
# defined projects like INSTALL.vcproj and ZERO_CHECK.vcproj
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if(MSVC)
    add_compile_options(/wd4251)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
endif()

file(GLOB FESceneGraphUI_SRC
	"FESceneGraphUI.cpp"
	"FESceneGraphUI.h"
)

if(FE_SCENE_GRAPH_UI_BUILD_SHARED_LIBS)
    add_library(FESceneGraphUI SHARED ${FESceneGraphUI_SRC})
    target_compile_definitions(FESceneGraphUI PRIVATE FESCENEGRAPHUI_EXPORTS)
    target_compile_definitions(FESceneGraphUI PUBLIC FESCENEGRAPHUI_SHARED)

    # Sometimes /GL conflicts with WINDOWS_EXPORT_ALL_SYMBOLS
	set_target_properties(FESceneGraphUI PROPERTIES COMPILE_OPTIONS "/GL-")
	set_target_properties(FESceneGraphUI PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)

    # Disable C4251 warning
    target_compile_options(FESceneGraphUI PRIVATE /wd4251)
else()
    add_library(FESceneGraphUI STATIC ${FESceneGraphUI_SRC})
endif()

# FESceneGraphUI require FocalEngine to already exist as a target.
if(NOT TARGET FocalEngine)
    message(FATAL_ERROR 
        "FESceneGraphUI requires the FocalEngine target to be defined before "
        "adding this subdirectory. Please add_subdirectory(FocalEngine) first."
    )
endif()

source_group("Source Files" FILES ${FESceneGraphUI_SRC})

target_link_libraries(FESceneGraphUI PUBLIC FocalEngine FEBasicApplication)
target_link_libraries(FESceneGraphUI PUBLIC FocalEngine)

# Expose our own include directory to consumers
target_include_directories(FESceneGraphUI PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Export for parent scope
set(FE_SCENE_GRAPH_UI_DIR ${CMAKE_CURRENT_SOURCE_DIR} PARENT_SCOPE)