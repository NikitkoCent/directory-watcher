cmake_minimum_required(VERSION 3.1.0)
project(directory-watcher CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_REQUIRED ON)

set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_INSTALL_PREFIX
    "${CMAKE_CURRENT_SOURCE_DIR}/bin/${CMAKE_CXX_COMPILER_ID}-${CMAKE_CXX_COMPILER_VERSION}")

if (CMAKE_VS_PLATFORM_NAME)
    set(CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}/${CMAKE_VS_PLATFORM_NAME}")
endif ()

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if (CMAKE_VERSION VERSION_LESS "3.7.0")
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()

find_package(Qt5 REQUIRED COMPONENTS Core Gui Widgets)

add_executable(directory-watcher WIN32
               src/main.cpp
               src/controller/mainwindow_controller.cpp
               src/model/directory_watcher.h
               src/model/directory_watcher_worker.h
               src/model/file_operations.h
               src/model/ordered_set.h
               src/model/path.h
               src/model/qt_directory_watcher_worker.h
               src/model/change_entry.cpp
               src/model/directory_watcher_worker.cpp
               src/model/windows/win_extend_path_limit.h
               src/model/windows/ci_char_traits.cpp
               src/model/windows/directory_watcher.cpp
               src/model/windows/file_operations.cpp
               src/model/windows/hashes.cpp
               src/model/windows/path.cpp
               src/view/mainwindow.ui
               src/view/mainwindow.h
               src/view/mainwindow.cpp)

if (WIN32)
    target_sources(directory-watcher PRIVATE resources/win_resources.rc)
endif()

target_link_libraries(directory-watcher
                      Qt5::Core
                      Qt5::Gui
                      Qt5::Widgets)

# ========================================== windeployqt ==========================================
include(src/build/DeployQt.cmake)

# Will be installed into ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_CONFIG_NAME}
windeployqt(directory-watcher "\${CMAKE_INSTALL_CONFIG_NAME}")
# ======================================== end windeployqt ========================================

include(src/build/SetupCPack.cmake)
