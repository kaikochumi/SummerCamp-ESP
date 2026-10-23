cmake_minimum_required(VERSION 3.20)
project(SummerCampEsp VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT WIN32)
  message(FATAL_ERROR "SummerCampEsp: Windows only (Win32 + D3D11).")
endif()

# Dear ImGui, vendored in third_party/imgui
set(IMGUI_DIR "${CMAKE_SOURCE_DIR}/third_party/imgui" CACHE PATH "Dear ImGui")
if(NOT EXISTS "${IMGUI_DIR}/imgui.h")
  message(FATAL_ERROR "third_party/imgui is missing.")
endif()

set(IMGUI_SRC
  ${IMGUI_DIR}/imgui.cpp
  ${IMGUI_DIR}/imgui_draw.cpp
  ${IMGUI_DIR}/imgui_tables.cpp
  ${IMGUI_DIR}/imgui_widgets.cpp
  ${IMGUI_DIR}/backends/imgui_impl_win32.cpp
  ${IMGUI_DIR}/backends/imgui_impl_dx11.cpp)

add_executable(summercamp-esp
  src/main.cpp
  src/memory.cpp
  src/game.cpp
  src/config.cpp
  src/i18n.cpp
  src/overlay.cpp
  src/menu.cpp
  ${IMGUI_SRC})

target_include_directories(summercamp-esp PRIVATE
  ${IMGUI_DIR} ${IMGUI_DIR}/backends ${CMAKE_SOURCE_DIR}/src)

target_link_libraries(summercamp-esp PRIVATE d3d11 d3dcompiler dwmapi psapi shell32)
target_compile_definitions(summercamp-esp PRIVATE
  UNICODE _UNICODE NOMINMAX WIN32_LEAN_AND_MEAN)

if(MINGW)
  target_link_options(summercamp-esp PRIVATE -static-libgcc -static-libstdc++)
endif()
