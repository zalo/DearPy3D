cmake_minimum_required (VERSION 3.13)

project ("DearPy3D")

set(MV_NO_SHADERS ${MV_NO_SHADERS})

# various settings for third party libs
add_definitions(-DSEMPER_INCLUDE_IMGUI)
add_definitions(-D_USE_MATH_DEFINES)

add_subdirectory ("DearPy3D")
add_subdirectory ("DearSandbox")