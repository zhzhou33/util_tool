cmake_minimum_required(VERSION 3.1.0)

if(POLICY CMP0074)
    # find_package() uses <PackageName>_ROOT variables.
    # This policy was introduced in CMake version 3.12.
    cmake_policy(SET CMP0074 NEW)
endif()

project(TimeWheel)

# C++ standard requirements.
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(LIB_TARGET_NAME util)
option(BUILD_UTIL_EXAMPLE "Build example executable" ON)

# CMake 3.1.0+ required.
# See: https://stackoverflow.com/a/29871891
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

include_directories(${PROJECT_SOURCE_DIR} ./spdlog/include)

file(GLOB LIB_SRCS *.cpp)
list(REMOVE_ITEM LIB_SRCS
    "${PROJECT_SOURCE_DIR}/main.cpp"
    "${PROJECT_SOURCE_DIR}/test.cpp")


set(LIBS
${CMAKE_THREAD_LIBS_INIT}
)

add_library(${LIB_TARGET_NAME} ${LIB_SRCS})
target_include_directories(${LIB_TARGET_NAME}
    PUBLIC
        ${PROJECT_SOURCE_DIR}
        ${PROJECT_SOURCE_DIR}/include
)
target_link_libraries(${LIB_TARGET_NAME} PUBLIC ${LIBS})
if(BUILD_SHARED_LIBS)
    target_compile_definitions(${LIB_TARGET_NAME}
        PRIVATE UTIL_BUILD_DLL
        PUBLIC UTIL_SHARED
    )
endif()

if(BUILD_UTIL_EXAMPLE)
    add_executable(time-wheel main.cpp test.cpp)
    target_include_directories(time-wheel PRIVATE ./include)
    target_link_libraries(time-wheel PRIVATE ${LIB_TARGET_NAME})
endif()

