cmake_minimum_required(VERSION 3.20)
project(OblivionLANProbe LANGUAGES CXX)

if(NOT WIN32)
    message(FATAL_ERROR "The first LAN probe currently targets Windows only.")
endif()

add_executable(OblivionLANProbe lan_probe.cpp)
target_compile_features(OblivionLANProbe PRIVATE cxx_std_17)
target_compile_definitions(OblivionLANProbe PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX)
target_link_libraries(OblivionLANProbe PRIVATE ws2_32)

if(MSVC)
    target_compile_options(OblivionLANProbe PRIVATE /W4 /permissive- /EHsc)
endif()
