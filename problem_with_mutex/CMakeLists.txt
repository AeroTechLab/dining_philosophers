project(problem_with_mutex C)

find_package(Threads REQUIRED)

add_executable(${PROJECT_NAME} ${PROJECT_NAME}.c)

target_include_directories(${PROJECT_NAME} PRIVATE ../include)
target_link_libraries(${PROJECT_NAME} Threads::Threads)
