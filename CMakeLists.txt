cmake_minimum_required(VERSION 3.16)

project(MessageSocket VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Qt6 REQUIRED COMPONENTS Gui Qml Quick)

qt_standard_project_setup(REQUIRES 6.5)
qt_policy(SET QTP0004 NEW)

set(CMAKE_AUTORCC ON)
set(QT_QML_GENERATE_QMLLS_INI ON)
qt_add_executable(messagesocket
  ressource.qrc
  main.cpp
  socket.h
  socket.cpp
  watcher.h
  watcher.cpp
)

qt_add_qml_module(messagesocket
  URI MessageSocket
  VERSION 1.0
  QML_FILES
    Main.qml
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
set_target_properties(messagesocket PROPERTIES
#    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.PharmaCync_Start
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE_GUI_IDENTIFIER com.messagesocket.app
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(messagesocket
    PRIVATE Qt6::Quick
)

include(GNUInstallDirs)
install(TARGETS messagesocket
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
