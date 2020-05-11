cmake_minimum_required(VERSION 3.16)

project(dining_philosofer)

add_subdirectory("problem_with_semaphore_bad")
add_subdirectory("problem_threadless")
add_subdirectory("problem_without_mutex")
add_subdirectory("problem_with_mutex")


find_package(Threads)

project(programando_onlinely)

add_executable(${PROJECT_NAME} ${PROJECT_NAME}.c)

target_link_libraries(${PROJECT_NAME} Threads::Threads ${CURSES_LIBRARIES})

