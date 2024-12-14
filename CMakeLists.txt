cmake_minimum_required(VERSION 3.6)

project(adatr)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(magidoor)
add_executable(adatr Main.cpp sqlite3.c Game.cpp Horse.cpp)

if (WIN32)
	TARGET_LINK_LIBRARIES(adatr mdoor Ws2_32)
Else()
	TARGET_LINK_LIBRARIES(adatr mdoor)
Endif()
