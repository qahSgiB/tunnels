cmake_minimum_required(VERSION 3.15) # TODO: version

project(tunnels CXX)



function(enable_all_warnings target)
    if (MSVC)
        target_compile_options(${target} PRIVATE /W4 /WX)
    else()
        target_compile_options(${target} PRIVATE -Wall -Werror -Wextra)
        target_compile_options(${target} PRIVATE -Wno-missing-field-initializers)
    endif()
endfunction()



# paths
set(libraries_path "${CMAKE_CURRENT_LIST_DIR}/libraries")
set(glue_path "${CMAKE_CURRENT_LIST_DIR}/glue")

## global emscripten settings
if (EMSCRIPTEN)
    set(CMAKE_EXECUTABLE_SUFFIX ".html")

    add_link_options(
        "--shell-file ${glue_path}/web/shell.html"
        "-s WASM=1"
        "-s USE_WEBGPU=1"
        "-s USE_GLFW=3"
        "-s ALLOW_MEMORY_GROWTH=1"
        # "--use-port=emdawnwebgpu"
        # "--closure=1"
    )

    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --shell-file ${glue_path}/web/shell.html -s WASM=1 -s USE_WEBGPU=1 -s USE_GLFW=3 -s ALLOW_MEMORY_GROWTH=1")

    add_compile_options("-Wno-nontrivial-memcall")

    # add_compile_options("--use-port=emdawnwebgpu")
endif()



## GLFW
# using GLFW own's cmake
if(NOT EMSCRIPTEN)
    add_subdirectory("${libraries_path}/glfw")
endif()

## GLM
# GLM is header only
add_library(glm INTERFACE)

target_include_directories(glm INTERFACE "${libraries_path}/glm")

## HEALPix
add_library(healpix STATIC)

file(GLOB_RECURSE healpix_headers ${libraries_path}/healpix/include/*.h)
file(GLOB_RECURSE healpix_sources ${libraries_path}/healpix/src/*.cc)

target_sources(healpix PRIVATE ${healpix_headers} ${healpix_sources})

set_target_properties(healpix PROPERTIES 
    CXX_STANDARD 20
    CXX_EXTENSIONS OFF
)

enable_all_warnings(healpix)

if (MSVC)
    target_compile_options(healpix PRIVATE /wd4244 /wd4267 /wd4701)
else()
    # TODO: same when not using MSVC
endif()

target_include_directories(healpix PUBLIC "${libraries_path}/healpix/include")

## Dawn
add_library(dawn INTERFACE)

if(NOT EMSCRIPTEN)
    # TODO: is this necessary ?
    find_package(Vulkan)
    if(Vulkan_FOUND)
        target_include_directories(dawn INTERFACE "${Vulkan_INCLUDE_DIRS}")
        target_link_libraries(dawn INTERFACE ${Vulkan_LIBRARY})
    endif()

    target_include_directories(dawn INTERFACE "${libraries_path}/dawn/inc")
    # target_link_directories(dawn INTERFACE "${CMAKE_CURRENT_LIST_DIR}/dawn/lib")

    if(WIN32)
        set(DAWN_LIB_PATH "win/x64")
    else()
        set(DAWN_LIB_PATH "linux/x64")
    endif()

    # TODO: improve, directly use CMAKE_BUILD_TYPE, generators
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        set(BUILD_TYPE_DIR "Release")
    else()
        set(BUILD_TYPE_DIR "Debug")
    endif()

    if(WIN32)
        find_library(DAWN_NATIVE_LIB NAMES dawn_native.dll PATHS "${libraries_path}/dawn/lib/${DAWN_LIB_PATH}/${BUILD_TYPE_DIR}" REQUIRED)
        find_library(DAWN_PLATFORM_LIB NAMES dawn_platform.dll PATHS "${libraries_path}/dawn/lib/${DAWN_LIB_PATH}/${BUILD_TYPE_DIR}" REQUIRED)
        find_library(DAWN_DAWN_PROC_LIB NAMES dawn_proc.dll PATHS "${libraries_path}/dawn/lib/${DAWN_LIB_PATH}/${BUILD_TYPE_DIR}" REQUIRED)
    else()
        find_library(DAWN_NATIVE_LIB NAMES libdawn_native.so PATHS "${libraries_path}/dawn/lib/${DAWN_LIB_PATH}/${BUILD_TYPE_DIR}" REQUIRED)
        find_library(DAWN_PLATFORM_LIB NAMES libdawn_platform.so PATHS "${libraries_path}/dawn/lib/${DAWN_LIB_PATH}/${BUILD_TYPE_DIR}" REQUIRED)
        find_library(DAWN_DAWN_PROC_LIB NAMES libdawn_proc.so PATHS "${libraries_path}/dawn/lib/${DAWN_LIB_PATH}/${BUILD_TYPE_DIR}" REQUIRED)
    endif()

    target_link_libraries(dawn INTERFACE ${DAWN_NATIVE_LIB} ${DAWN_PLATFORM_LIB} ${DAWN_DAWN_PROC_LIB})
endif()

## ImGui library
add_library(imgui STATIC)

target_sources(imgui
	PRIVATE 
		"${libraries_path}/imgui/imgui.cpp"
		"${libraries_path}/imgui/imgui_draw.cpp"
		"${libraries_path}/imgui/imgui_tables.cpp"
		"${libraries_path}/imgui/imgui_widgets.cpp"
		"${libraries_path}/imgui/misc/cpp/imgui_stdlib.cpp"
		"${libraries_path}/imgui/backends/imgui_impl_glfw.cpp"
		"${libraries_path}/imgui/backends/imgui_impl_wgpu.cpp"
)

set_target_properties(imgui PROPERTIES 
    CXX_STANDARD 20
    CXX_EXTENSIONS OFF
)

enable_all_warnings(imgui)

if (MSVC)
    # TODO: same when not using MSVC
else()
    # target_compile_options(imgui PRIVATE -Wno-nontrivial-memcall)
    # target_compile_options(imgui PRIVATE -Wno-uninitialized-const-pointer)
endif()

target_include_directories(imgui PUBLIC "${libraries_path}/imgui")

target_link_libraries(imgui PUBLIC glfw dawn)

## tunnels executable

if (EMSCRIPTEN)
    set(glue_file ${glue_path}/web/main.cpp)
else()
    set(glue_file ${glue_path}/native/main.cpp)
endif()

file(GLOB_RECURSE headers ${CMAKE_CURRENT_LIST_DIR}/include/*.h)
file(GLOB_RECURSE sources ${CMAKE_CURRENT_LIST_DIR}/src/*.cpp)

add_executable(tunnels ${headers} ${sources} ${glue_file})

set_target_properties(tunnels PROPERTIES 
    CXX_STANDARD 20
    CXX_EXTENSIONS OFF
)

if (MSVC)
    target_compile_options(tunnels PRIVATE /W4 /WX)
else()
    target_compile_options(tunnels PRIVATE -Wall -Werror -Wextra)
    target_compile_options(tunnels PRIVATE -Wno-missing-field-initializers)
endif()

target_compile_definitions(tunnels PRIVATE GLM_FORCE_SILENT_WARNINGS=1)

target_include_directories(tunnels PRIVATE "${CMAKE_CURRENT_LIST_DIR}/include")

target_link_libraries(tunnels PRIVATE imgui glm healpix)

if(NOT EMSCRIPTEN)
    target_link_libraries(tunnels PRIVATE glfw)
endif()

### link assets for emscripten
if (EMSCRIPTEN)
    set(assets_folder "${CMAKE_CURRENT_LIST_DIR}/assets")
    target_link_options(tunnels PUBLIC --preload-file ${assets_folder}@assets)
    
    file(GLOB_RECURSE assets_files ${assets_folder}/*)
    set_target_properties(tunnels PROPERTIES LINK_DEPENDS "${assets_files}")
endif()

### WebGPU enabled backends
if (NOT EMSCRIPTEN)
    set(ENABLE_D3D12 FALSE)
    set(ENABLE_METAL FALSE)
    set(ENABLE_VULKAN FALSE)
    set(ENABLE_OPENGL FALSE)
    set(ENABLE_NULL TRUE)
    if (WIN32)
        set(ENABLE_D3D12 TRUE)
        set(ENABLE_VULKAN TRUE)
        set(ENABLE_OPENGL TRUE)
    elseif(APPLE)
        set(ENABLE_METAL TRUE)
    elseif(UNIX)
        set(ENABLE_VULKAN TRUE)
        set(ENABLE_OPENGL TRUE)
    endif()

    if (ENABLE_D3D12)
        target_compile_definitions(tunnels PRIVATE DAWN_ENABLE_BACKEND_D3D12)
    endif()
    if (ENABLE_METAL)
        target_compile_definitions(tunnels PRIVATE DAWN_ENABLE_BACKEND_METAL)
    endif()
    if (ENABLE_VULKAN)
        target_compile_definitions(tunnels PRIVATE DAWN_ENABLE_BACKEND_VULKAN)
    endif()
    if (ENABLE_OPENGL)
        target_compile_definitions(tunnels PRIVATE DAWN_ENABLE_BACKEND_OPENGL)
    endif()
    if (ENABLE_NULL)
        target_compile_definitions(tunnels PRIVATE DAWN_ENABLE_BACKEND_NULL)
    endif()
endif()

### copy Dawn library files
# TODO: improve
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(BUILD_TYPE_DIR "Release")
else()
    set(BUILD_TYPE_DIR "Debug")
endif()

if(WIN32)
    add_custom_command(TARGET tunnels POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${libraries_path}/dawn/lib/win/x64/${BUILD_TYPE_DIR}/dawn_native.dll" 
        "${libraries_path}/dawn/lib/win/x64/${BUILD_TYPE_DIR}/dawn_platform.dll"
        "${libraries_path}/dawn/lib/win/x64/${BUILD_TYPE_DIR}/dawn_proc.dll"
    $<TARGET_FILE_DIR:tunnels>)
else()
    add_custom_command(TARGET tunnels POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${libraries_path}/dawn/lib/linux/x64/${BUILD_TYPE_DIR}/libdawn_native.so" 
        "${libraries_path}/dawn/lib/linux/x64/${BUILD_TYPE_DIR}/libdawn_platform.so"
        "${libraries_path}/dawn/lib/linux/x64/${BUILD_TYPE_DIR}/libdawn_proc.so"
    $<TARGET_FILE_DIR:tunnels>)
endif()
