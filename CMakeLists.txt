cmake_minimum_required(VERSION 3.16)
project(main_theProblem_bad C)
project(main_theProblem_noMutex C)
project(main_theProblem_Mutex C)
project(main_theProblem_threadless C)
project(mossaTest C)

set(CMAKE_C_STANDARD 11)

find_package(Threads REQUIRED)
find_package(Curses REQUIRED)

add_executable(dinning_philosofers_c_bad main_bad.c colors.h)
add_executable(main_theProblem_noMutex main_theProblem_noMutex.c colors.h)
add_executable(main_theProblem_threadless main_theProblem_threadless.c colors.h)
add_executable(main_theProblem_Mutex main_theProblem_Mutex.c)
add_executable(mossaTest mossaTest.c)

target_link_libraries(dinning_philosofers_c_bad Threads::Threads)
target_link_libraries(main_theProblem_noMutex Threads::Threads ${CURSES_LIBRARY} m)
target_link_libraries(main_theProblem_Mutex Threads::Threads ${CURSES_LIBRARY} m)
target_link_libraries(mossaTest Threads::Threads ${CURSES_LIBRARY} m)
