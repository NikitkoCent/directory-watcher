# The MIT License (MIT)
#
# Copyright (c) 2017-2018 Nathan Osman
# Copyright (c) 2019 Nikita Provotorov
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

cmake_minimum_required(VERSION 3.8)

find_package(Qt5Core REQUIRED)

# Retrieve the absolute path to qmake and then use that path to find
# the windeployqt and macdeployqt binaries
get_target_property(QMAKE_EXECUTABLE Qt5::qmake IMPORTED_LOCATION)
get_filename_component(QT_BIN_DIR "${QMAKE_EXECUTABLE}" DIRECTORY)

find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${QT_BIN_DIR}")
if(WIN32 AND NOT WINDEPLOYQT_EXECUTABLE)
    message(FATAL_ERROR "windeployqt not found")
endif()

find_program(MACDEPLOYQT_EXECUTABLE macdeployqt HINTS "${QT_BIN_DIR}")
if(APPLE AND NOT MACDEPLOYQT_EXECUTABLE)
    message(FATAL_ERROR "macdeployqt not found")
endif()

# Add commands that copy the Qt runtime to the target's output directory after
# build and install the Qt runtime to the specified directory
function(windeployqt target directory)

    # Run windeployqt immediately after build
    add_custom_command(TARGET ${target} POST_BUILD
        COMMAND "${CMAKE_COMMAND}"
                -D TARGET="${target}"
                -D TARGET_EXECUTABLE="$<TARGET_FILE:${target}>"
                -D QT_BIN_DIR="${QT_BIN_DIR}"
                -D DEPLOYQT_EXECUTABLE="${WINDEPLOYQT_EXECUTABLE}"
                -D CONFIG="$<CONFIG>"
                -D CMAKE_CXX_COMPILER="${CMAKE_CXX_COMPILER}"
                -D CMAKE_C_COMPILER="${CMAKE_C_COMPILER}"
                -P "${CMAKE_CURRENT_SOURCE_DIR}/src/build/GatherQtDependencies.cmake"
        COMMAND "${CMAKE_COMMAND}"
                -D TARGET="${target}"
                -D CONFIG="$<CONFIG>"
                -D TARGET_INSTALL_DIRECTORY="$<TARGET_FILE_DIR:${target}>"
                -P "${CMAKE_CURRENT_SOURCE_DIR}/src/build/InstallDependencies.cmake"
    )

    # install(CODE ...) doesn't support generator expressions, but
    # file(GENERATE ...) does - store the path in a file
    file(GENERATE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/${target}_path"
         CONTENT "$<TARGET_FILE:${target}>"
    )

    file(GENERATE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/${target}_path_name"
         CONTENT "$<TARGET_FILE_NAME:${target}>"
    )

    # Before installation, run a series of commands that copy each of the Qt
    # runtime files to the appropriate directory for installation
    install(CODE
        "
            file(READ \"${CMAKE_CURRENT_BINARY_DIR}/\${CMAKE_INSTALL_CONFIG_NAME}/${target}_path\" target_path)
            file(READ \"${CMAKE_CURRENT_BINARY_DIR}/\${CMAKE_INSTALL_CONFIG_NAME}/${target}_path_name\" target_path_name)

            execute_process(COMMAND \"${CMAKE_COMMAND}\"
                            -D TARGET=${target}
                            -D TARGET_INSTALL_DIRECTORY=${CMAKE_INSTALL_PREFIX}/${directory}
                            -D CONFIG=\${CMAKE_INSTALL_CONFIG_NAME}
                            -P \"${CMAKE_CURRENT_SOURCE_DIR}/src/build/InstallDependencies.cmake\"
                            COMMAND \"${CMAKE_COMMAND}\"
                            -E copy \${target_path} ${CMAKE_INSTALL_PREFIX}/${directory}/\${target_path_name}
            )
        "
    )

endfunction()

mark_as_advanced(WINDEPLOYQT_EXECUTABLE)
