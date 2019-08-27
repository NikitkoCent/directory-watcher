get_filename_component(COMPILER_PATH "${CMAKE_CXX_COMPILER}" DIRECTORY)
file(TO_CMAKE_PATH "${COMPILER_PATH}" COMPILER_PATH)

if (${CONFIG} MATCHES Debug)
    set(CONFIG_PARAM "--debug")
elseif ((${CONFIG} MATCHES Release) OR (${CONFIG} MATCHES RelWithDebInfo) OR (${CONFIG} MATCHES MinSizeRel))
    set(CONFIG_PARAM "--release")
else ()
    set(CONFIG_PARAM "")
endif ()

set(WINDEPLOYQT_PARAMS --no-angle --no-opengl-sw ${CONFIG_PARAM})

execute_process(COMMAND "${CMAKE_COMMAND}" -E
                        env "PATH=${COMPILER_PATH};${QT_BIN_DIR}" "${DEPLOYQT_EXECUTABLE}"
                        --dry-run
                        ${WINDEPLOYQT_PARAMS}
                        --list mapping
                        "${TARGET_EXECUTABLE}"
                OUTPUT_FILE "${CONFIG}/${TARGET}_dependencies"
                OUTPUT_STRIP_TRAILING_WHITESPACE)

set(CMAKE_INSTALL_UCRT_LIBRARIES TRUE)
set(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_SKIP TRUE)

include(InstallRequiredSystemLibraries)

foreach(lib ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS})
    get_filename_component(libname "${lib}" NAME)
    file(APPEND "${TARGET}-dependencies" "\"${lib}\" \"${libname}\"")
endforeach()
