cmake_minimum_required(VERSION 3.1.0)
project(directory-watcher CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if (CMAKE_VERSION VERSION_LESS "3.7.0")
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()

# Make this a GUI application on Windows
if (WIN32)
    set(CMAKE_WIN32_EXECUTABLE ON)
endif()

find_package(Qt5 REQUIRED COMPONENTS Core Gui Widgets)

add_executable(directory-watcher
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
get_target_property(QMAKE_PATH Qt5::qmake IMPORTED_LOCATION)
get_filename_component(QT_BIN_DIR "${QMAKE_PATH}" DIRECTORY)
find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${QT_BIN_DIR}")

set(PATH_DELIM :)
if (WIN32)
    set(PATH_DELIM ;)
endif()

add_custom_command(TARGET directory-watcher POST_BUILD
                   COMMAND "${CMAKE_COMMAND}" -E echo "Path${PATH_DELIM} $ENV{PATH}"
                   COMMAND "${CMAKE_COMMAND}" -E env "PATH=${QT_BIN_DIR}${PATH_DELIM}$ENV{PATH}"
                           "${WINDEPLOYQT_EXECUTABLE}" "$<TARGET_FILE:directory-watcher>"
                   COMMENT "Running windeployqt at ${WINDEPLOYQT_EXECUTABLE}...")
# ======================================== end windeployqt ========================================
