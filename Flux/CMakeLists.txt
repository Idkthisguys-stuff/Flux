cmake_minimum_required(VERSION 3.10)

if (WIN32)
    if (DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
        set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
    endif ()
    set(VCPKG_TARGET_TRIPLET "x64-windows" CACHE STRING "")
endif ()

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(VCPKG_TARGET_TRIPLET "x64-windows-llvm" CACHE STRING "")
endif()
project(FluxEngine LANGUAGES C CXX)

set(RESOURCES "")
if (WIN32)
    set(RESOURCES "resource.rc")
endif()

add_executable(Flux
        "main.cpp"
        "core/Window.cpp"
        "core/Window.h"
        "editor/viewport.cpp"
        "editor/viewport.h"
        "editor/ribbon.cpp"
        "editor/explorer.cpp"
        "editor/explorer.h"
        "editor/output.cpp"
        "editor/properties.h"
        "editor/properties.cpp"
        "render/3D/OpenGL/Model.h"
        "render/3D/OpenGL/OpenGLRenderer.h"
        "render/3D/OpenGL/OpenGLManager.cpp"
        "render/3D/OpenGL/OpenGLRenderer.cpp"
        "render/3D/OpenGL/Model.cpp"
        "logic/data/camera.h"
        editor/heiarchy.cpp
        editor/heiarchy.h
        logic/data/Scenenode.h
        logic/Textureloader.cpp
        logic/Textureloader.h
        stb_implementation.cpp
        core/pathHelper.h
        core/pathHelper.cpp
        editor/texteditor.h
        editor/texteditor.cpp
        scripting/luaEngine.h
        scripting/luaEngine.cpp
        logic/runtime.h
        logic/runtime.cpp
        logic/SplashScreen.h
        logic/data/cameraGuizmoModel.h
        logic/PhysicsLayer.h
        resource.rc
        logic/data/material.h
        editor/themes/themes.h
        editor/themes/themes.cpp
        core/Memory/allocators/linearAllocator.h
        core/Memory/allocators/poolAllocator.h
        core/Memory/allocators/stackAllocator.h
        core/Memory/MemoryManager.h
        logic/data/components.h
)

get_filename_component(PROJECT_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/.." ABSOLUTE)

if (NOT WIN32)
    target_sources(Flux PRIVATE
            "${CMAKE_CURRENT_SOURCE_DIR}/lib/imgui/imgui.cpp"
            "${CMAKE_CURRENT_SOURCE_DIR}/lib/imgui/imgui_demo.cpp"
            "${CMAKE_CURRENT_SOURCE_DIR}/lib/imgui/imgui_draw.cpp"
            "${CMAKE_CURRENT_SOURCE_DIR}/lib/imgui/imgui_widgets.cpp"
            "${CMAKE_CURRENT_SOURCE_DIR}/lib/imgui/imgui_tables.cpp"
            "${CMAKE_CURRENT_SOURCE_DIR}/lib/imgui/backends/imgui_impl_sdl3.cpp"
            "${CMAKE_CURRENT_SOURCE_DIR}/lib/imgui/backends/imgui_impl_opengl3.cpp"
            "${CMAKE_CURRENT_SOURCE_DIR}/lib/imguizmo/ImGuizmo.cpp"
            "${CMAKE_CURRENT_SOURCE_DIR}/lib/glad/include"
            "${CMAKE_CURRENT_SOURCE_DIR}/lib/stb"
            "${CMAKE_CURRENT_SOURCE_DIR}/lib/glad/src/glad.c"
    )
endif ()

if (WIN32)
    target_link_options(Flux PRIVATE 
        "$<$<CONFIG:Debug>:/SUBSYSTEM:CONSOLE>"
        "$<$<CONFIG:Release>:/SUBSYSTEM:WINDOWS>"
        "/ENTRY:mainCRTStartup"
    )
endif()

find_package(OpenGL REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(assimp REQUIRED)
find_package(sol2 CONFIG REQUIRED)
find_package(SDL3 CONFIG REQUIRED)

find_package(Jolt CONFIG REQUIRED)

if (WIN32)
    find_package(imgui CONFIG REQUIRED)
    find_package(glad CONFIG REQUIRED)
    find_package(imguizmo CONFIG REQUIRED)
    find_package(Stb REQUIRED)
else ()
endif ()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

file(COPY "${PROJECT_ROOT}/assets" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")
file(COPY "${PROJECT_ROOT}/assets/models" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")
file(COPY "${PROJECT_ROOT}/assets/splash" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")
file(COPY "${PROJECT_ROOT}/assets/fonts" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")
file(COPY "${PROJECT_ROOT}/assets/icons" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/templates" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/templates/Project_Templates" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/templates/File_Templates" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/templates/File_Templates/scripts" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/templates/File_Templates/models" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/core" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/editor" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/logic" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/render" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/scripting" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/shaders" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/lib" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")

if (CMAKE_VERSION VERSION_GREATER 3.12)
    set_property(TARGET Flux PROPERTY CXX_STANDARD 20)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_EXTENSIONS OFF)
endif ()

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

target_link_directories(Flux PRIVATE
    ${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/$<$<CONFIG:Debug>:debug/>lib
)

target_link_libraries(Flux PRIVATE
        glm::glm
        assimp::assimp
        sol2::sol2
        SDL3::SDL3
        Jolt::Jolt
        ${OPENGL_LIBRARIES}
)

if (WIN32)
    target_link_libraries(Flux PRIVATE
            imgui::imgui
            glad::glad
            imguizmo::imguizmo
            opengl32
            dwmapi
            lua51
    )
    target_link_options(Flux PRIVATE 
        "$<$<CONFIG:Debug>:/SUBSYSTEM:CONSOLE>"
        "$<$<CONFIG:Release>:/SUBSYSTEM:WINDOWS>"
        "/ENTRY:mainCRTStartup"
    )
else ()
    target_link_libraries(Flux PRIVATE
            dl
            pthread
    )
endif ()

target_include_directories(Flux PRIVATE
        "engine"
        "engine"
        "assets"
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/include
        ${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/include/luajit
)

if (NOT WIN32)
    target_include_directories(Flux PRIVATE
            "${CMAKE_CURRENT_SOURCE_DIR}/lib/imgui"
            "${CMAKE_CURRENT_SOURCE_DIR}/lib/imgui/backends"
            "${CMAKE_CURRENT_SOURCE_DIR}/lib/glad/include"
            "${CMAKE_CURRENT_SOURCE_DIR}/lib/imguizmo"
            "${CMAKE_CURRENT_SOURCE_DIR}/lib/stb"
    )
endif ()



if (MSVC)
    target_compile_options(Flux PRIVATE $<$<CONFIG:Release>:/O2 /GL>)
    target_link_options(Flux PRIVATE $<$<CONFIG:Release>:/LTCG>)
else ()
    target_compile_options(Flux PRIVATE $<$<CONFIG:Release>:-O3 -flto>)
    target_link_options(Flux PRIVATE $<$<CONFIG:Release>:-flto>)
endif ()

if (NOT MSVC)
    set_target_properties(Flux PROPERTIES INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
endif ()

if (NOT WIN32)
    set(APPIMAGE_OUTPUT_DIR "${CMAKE_SOURCE_DIR}/out/build/x64-wsl")

    set(ICON_SRC "${CMAKE_SOURCE_DIR}/Flux/assets/flux.png")
    set(ICON_RGBA "${CMAKE_CURRENT_BINARY_DIR}/flux.png")

    add_custom_target(AppImage
            COMMAND ${CMAKE_COMMAND} -E rm -rf "AppDir"

            COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/assets/flux.png" "${ICON_RGBA}"

            COMMAND ${CMAKE_COMMAND} -E make_directory "AppDir/usr/share/flux/shaders"
            COMMAND ${CMAKE_COMMAND} -E make_directory "AppDir/usr/share/flux/assets"

            COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/shaders" "AppDir/usr/share/flux/shaders"
            COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/assets" "AppDir/usr/share/flux/assets"

            COMMAND convert "${ICON_SRC}" -depth 8 RGBA:"${ICON_RGBA}_raw.rgba" || true
            COMMAND convert "${ICON_SRC}" -type TrueColorAlpha "${ICON_RGBA}"

            COMMAND ${CMAKE_COMMAND} -E make_directory "AppDir/usr/share/icons/hicolor/256x256/apps"
            COMMAND ${CMAKE_COMMAND} -E make_directory "AppDir/usr/share/pixmaps"

            COMMAND ${CMAKE_COMMAND} -E copy "${ICON_RGBA}" "AppDir/flux.png"
            COMMAND ${CMAKE_COMMAND} -E copy "${ICON_RGBA}" "AppDir/usr/share/icons/hicolor/256x256/apps/flux.png"
            COMMAND ${CMAKE_COMMAND} -E copy "${ICON_RGBA}" "AppDir/usr/share/pixmaps/flux.png"

            COMMAND "${CMAKE_SOURCE_DIR}/linuxdeploy-x86_64.AppImage"
            --appdir "AppDir"
            -e "Flux"
            -d "${CMAKE_SOURCE_DIR}/flux.desktop"
            -i "${ICON_RGBA}"
            --output appimage

            COMMAND ${CMAKE_COMMAND} -E make_directory "${APPIMAGE_OUTPUT_DIR}"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "Flux_Engine-x86_64.AppImage"
            "${APPIMAGE_OUTPUT_DIR}/Flux_Engine.AppImage"

            WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
            COMMENT "Building AppImage (converting icon to RGBA, then packing)..."
            DEPENDS Flux
    )

    add_custom_target(FullDistribution
            COMMAND ${CMAKE_COMMAND} --build . --target Flux
            COMMAND ${CMAKE_COMMAND} --build . --target AppImage
            COMMAND ${CMAKE_CPACK_COMMAND} --config "../CPackConfig.cmake" -G DEB
            WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
            COMMENT "Executing Full Distribution: Engine -> AppImage -> .deb"
            DEPENDS Flux AppImage
    )

    install(TARGETS Flux DESTINATION bin)
    install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/assets/"  DESTINATION share/flux/assets)
    install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/shaders/" DESTINATION share/flux/shaders)

    set(CPACK_PACKAGE_NAME    "FluxEngine")
    set(CPACK_PACKAGE_VENDOR  "ZeroPointStudio")
    set(CPACK_PACKAGE_VERSION "1.0.0")
    set(CPACK_PACKAGE_CONTACT "zeropointstudio445@gmail.com")
    set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "A game engine")
    set(CPACK_GENERATOR       "DEB;TGZ")

    include(CPack)
endif ()

install(TARGETS Flux DESTINATION bin)
install(DIRECTORY "${PROJECT_ROOT}/assets/" DESTINATION bin/assets)
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/shaders/" DESTINATION bin/shaders)
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/templates/" DESTINATION bin/templates)

if (WIN32)
    set(VCPKG_CONFIG_SUBDIR "")
    if (CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_CFG_INTDIR STREQUAL "Debug")
        set(VCPKG_CONFIG_SUBDIR "debug/")
    endif()

    install(DIRECTORY "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/${VCPKG_CONFIG_SUBDIR}bin/"
            DESTINATION bin
            FILES_MATCHING PATTERN "*.dll"
    )
endif()