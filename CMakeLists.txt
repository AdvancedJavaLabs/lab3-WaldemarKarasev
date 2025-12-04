cmake_minimum_required(VERSION 3.10)
project(text_processor CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(common)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wextra-semi -O2  -Wno-uninitialized")

add_executable(mapper
    src/mapper/main.cpp
)
target_include_directories(mapper PUBLIC ${CMAKE_SOURCE_DIR})
target_link_libraries(mapper common-func)

add_executable(reducer
    src/reducer/main.cpp
)
target_include_directories(reducer PUBLIC ${CMAKE_SOURCE_DIR})
target_link_libraries(reducer common-func)


# file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/resources" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")