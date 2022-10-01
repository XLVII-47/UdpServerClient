cmake_minimum_required(VERSION 3.14)

project(UdpClient LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_PREFIX_PATH $ENV{HOME}/Qt/6.3.1/gcc_64)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Network Concurrent)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Network Concurrent)

add_executable(UdpClient
  main.cpp
  udpclient.h udpclient.cpp

)
target_link_libraries(UdpClient Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::Network Qt6::Concurrent)

install(TARGETS UdpClient
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
