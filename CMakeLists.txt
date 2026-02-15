cmake_minimum_required(VERSION 3.16)

project(HamVibe VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Test Sql Network)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Test Sql Network)

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
        frequencylabel.cpp
        frequencylabel.h
        udpreceiver.cpp
        udpreceiver.h
        tcpreceiver.cpp
        tcpreceiver.h
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(HamVibe
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        rig.h rig.cpp
        frequencylabel.h
        frequencylabel.cpp
        delegate.h
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET HamVibe APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(HamVibe SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(HamVibe
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(HamVibe PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Sql Qt${QT_VERSION_MAJOR}::Network)
target_include_directories(HamVibe PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_directories(HamVibe PRIVATE "C:/Program Files/hamlib-w64-4.6.5/lib/gcc")
target_link_libraries(HamVibe PRIVATE hamlib)

set(HAMLIB_BIN_DIR "C:/Program Files/hamlib-w64-4.6.5/bin")
function(copy_hamlib_runtime target)
    foreach(_dll libhamlib-4.dll libusb-1.0.dll)
        add_custom_command(TARGET ${target} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${HAMLIB_BIN_DIR}/${_dll}"
                "$<TARGET_FILE_DIR:${target}>/${_dll}"
        )
    endforeach()
endfunction()

copy_hamlib_runtime(HamVibe)
target_include_directories(HamVibe PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../hamlib/include")

enable_testing()
add_executable(HamVibeTests
    tests/main_test.cpp
    tests/frequencylabel_test.cpp
    tests/rig_test.cpp
    frequencylabel.h
    frequencylabel.cpp
    rig.h
    rig.cpp
)
target_include_directories(HamVibeTests PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/../hamlib/include"
)
target_link_directories(HamVibeTests PRIVATE "C:/Program Files/hamlib-w64-4.6.5/lib/gcc")
target_link_libraries(HamVibeTests PRIVATE Qt${QT_VERSION_MAJOR}::Test Qt${QT_VERSION_MAJOR}::Widgets hamlib)
copy_hamlib_runtime(HamVibeTests)
target_include_directories(HamVibeTests PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}"
)
target_link_libraries(HamVibeTests PRIVATE Qt${QT_VERSION_MAJOR}::Test Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Sql Qt${QT_VERSION_MAJOR}::Network)
add_test(NAME HamVibeTests COMMAND HamVibeTests)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.HamVibe)
endif()
set_target_properties(HamVibe PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS HamVibe
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(HamVibe)
endif()
