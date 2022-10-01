cmake_minimum_required(VERSION 3.14)

project(UdpServer LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_PREFIX_PATH $ENV{HOME}/Qt/6.3.1/gcc_64)


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Network Concurrent)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Network Concurrent)

add_executable(UdpServer
  main.cpp
  udpserver.h udpserver.cpp
  client.h client.cpp
)
target_link_libraries(UdpServer Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::Network Qt${QT_VERSION_MAJOR}::Concurrent)

install(TARGETS UdpServer
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
