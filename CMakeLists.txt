cmake_minimum_required(VERSION 3.16)

project(MessageSocket VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Qt6 REQUIRED COMPONENTS Gui Qml Quick)

qt_standard_project_setup(REQUIRES 6.5)
qt_policy(SET QTP0004 NEW)

set(CMAKE_AUTORCC ON)
set(QT_QML_GENERATE_QMLLS_INI ON)

add_library(MessageSocketLib STATIC
    socket.cpp
    socket.h
    watcher.cpp
    watcher.h
)

target_include_directories(MessageSocketLib PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(MessageSocketLib
  PUBLIC Qt6::Gui Qt6::Quick Qt6::Qml
)

qt_add_executable(messagesocket main.cpp ressource.qrc)
target_link_libraries(messagesocket PRIVATE MessageSocketLib)

qt_add_qml_module(messagesocket
  URI MessageSocket
  VERSION 1.0
  QML_FILES
    Main.qml
)

