cmake_minimum_required(VERSION 3.16)
project(ProteusClone VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Core Gui)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    src/*.cpp
    src/*.h
)

list(APPEND SOURCES resources/resources.qrc)

if (WIN32)
    list(APPEND SOURCES resources/windows_icon.rc)
endif()

add_executable(ProteusClone ${SOURCES})

target_include_directories(ProteusClone PRIVATE src)

target_link_libraries(ProteusClone
    PRIVATE
    Qt6::Widgets
    Qt6::Core
    Qt6::Gui
)


# IMPORTANT:
# Do NOT run windeployqt as a POST_BUILD command. Some Windows/Qt setups return
# a non-zero deployment exit code even when the executable has been built correctly,
# which makes CMake report:
#   CMakeFiles\ProteusClone.dir\build.make:....: ProteusClone.exe Error 1
# Runtime deployment is therefore handled by deploy_windows_mingw.bat after build.

option(PROTEUS_CREATE_DEPLOY_TARGET "Create a manual Qt runtime deployment target" ON)
if (WIN32 AND PROTEUS_CREATE_DEPLOY_TARGET)
    get_target_property(_qt_qmake Qt6::qmake IMPORTED_LOCATION)
    if (_qt_qmake)
        get_filename_component(_qt_bin_dir "${_qt_qmake}" DIRECTORY)
        find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${_qt_bin_dir}")
        if (WINDEPLOYQT_EXECUTABLE)
            add_custom_target(deploy_runtime
                COMMAND "${WINDEPLOYQT_EXECUTABLE}"
                        "$<IF:$<CONFIG:Debug>,--debug,--release>"
                        --compiler-runtime
                        "$<TARGET_FILE:ProteusClone>"
                DEPENDS ProteusClone
                COMMENT "Deploying Qt runtime files next to ProteusClone.exe"
                VERBATIM
            )
        endif()
    endif()
endif()
