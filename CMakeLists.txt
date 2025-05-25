cmake_minimum_required(VERSION 3.10.0)
project(glassyclock)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

add_definitions(-Wall)

if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(CMAKE_INSTALL_PREFIX "/usr")
endif()

set(QT6_MINIMUM_VERSION "6.3.0")
set(KF6_MINIMUM_VERSION "6.0.0")
set(SHELLQT_MINIMUM_VERSION "6.0.0")

find_package(Qt6 "${QT6_MINIMUM_VERSION}" REQUIRED COMPONENTS Core Gui Widgets Svg)
find_package(KF6WindowSystem "${KF6_MINIMUM_VERSION}" REQUIRED)
find_package(LayerShellQt "${SHELLQT_MINIMUM_VERSION}" REQUIRED)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}
                    ${Qt6Core_INCLUDE_DIRS}
                    ${Qt6Gui_INCLUDE_DIRS}
                    ${Qt6Widgets_INCLUDE_DIRS}
                    ${Qt6Svg_INCLUDE_DIRS})

set(glassyclock_SRCS
    main.cpp
    glassyclock.cpp)

set(glassyclock_RESOURCES glassyclock.qrc)

add_executable(glassyclock ${glassyclock_SRCS} ${glassyclock_RESOURCES})

target_link_libraries(glassyclock ${Qt6Core_LIBRARIES}
                                  ${Qt6Gui_LIBRARIES}
                                  ${Qt6Widgets_LIBRARIES}
                                  ${Qt6Svg_LIBRARIES}
                                  ${Qt6DBus_LIBRARIES}
                                  KF6::WindowSystem
                                  LayerShellQtInterface)

install(TARGETS glassyclock RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
