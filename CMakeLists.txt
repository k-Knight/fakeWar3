cmake_minimum_required(VERSION 3.16)
project(fakeWar3)

if(CMAKE_BUILD_TYPE MATCHES Release)
    set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -mwindows -static")
endif()

file(GLOB SOURCE_FILES
    "*.cpp"
)

set(APP war3)

add_executable(${APP} ${SOURCE_FILES})

set_target_properties(
    ${APP} PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)
