project(problem_without_mutex C)

find_package(Threads REQUIRED)
#find_package(Curses REQUIRED)

add_executable(${PROJECT_NAME} ${PROJECT_NAME}.c)

target_link_libraries(${PROJECT_NAME} Threads::Threads)
