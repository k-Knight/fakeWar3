cmake_minimum_required(VERSION 3.16)
project(fakeWar3)

set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -static")
if(CMAKE_BUILD_TYPE MATCHES Release)
    set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -mwindows")
endif()

file(GLOB SOURCE_FILES
    "*.cpp"
)

set(APP war3)
add_executable(${APP} ${SOURCE_FILES})
target_compile_features(${APP} PRIVATE cxx_std_17)

set_target_properties(
    ${APP} PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)
