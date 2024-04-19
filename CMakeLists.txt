cmake_minimum_required(VERSION 3.16)

project(skip_list)

add_library(${PROJECT_NAME} INTERFACE)
target_include_directories(${PROJECT_NAME} 
  INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include/)
target_compile_features(${PROJECT_NAME} INTERFACE cxx_std_20)
target_compile_options(${PROJECT_NAME} INTERFACE -g)
install(TARGETS ${PROJECT_NAME})

add_executable(${PROJECT_NAME}_main test/main.cpp)
target_link_libraries(${PROJECT_NAME}_main ${PROJECT_NAME})

add_executable(${PROJECT_NAME}_test test/stress_test.cpp)
target_link_libraries(${PROJECT_NAME}_test ${PROJECT_NAME} pthread)


