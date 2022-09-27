cmake_minimum_required(VERSION 3.0)

project(autobiblatex)

set(SRC_FILES main.cpp log.cpp tokenize.cpp)

find_package(PkgConfig REQUIRED)
pkg_check_modules(POPPLER REQUIRED poppler-cpp)
pkg_check_modules(CURL REQUIRED libcurl)

link_directories(${CMAKE_CURRENT_BINARY_DIR})

add_executable(${PROJECT_NAME} ${SRC_FILES})
target_link_libraries( ${PROJECT_NAME} ${POPPLER_LINK_LIBRARIES} ${CURL_LINK_LIBRARIES})
target_include_directories(${PROJECT_NAME} PRIVATE  ${POPPLER_INCLUDE_DIRS} ${CURL_INCLUDE_DIRS})
add_dependencies(${PROJECT_NAME} ${PROJECT_NAME})
target_compile_options(${PROJECT_NAME} PRIVATE "-std=c++2a" "-Wall" "-O2" "-g" "-fno-strict-aliasing" "-Wfatal-errors" "-Wno-reorder")

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -s")

set(CMAKE_INSTALL_PREFIX "/usr")
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)
