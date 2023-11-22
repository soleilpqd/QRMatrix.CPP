cmake_minimum_required(VERSION 3.5)

project(QRMatrixExample VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

set(PROJECT_SOURCES
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
#    ../../../DevTools/devtools.cpp
#    ../../../DevTools/devtools.h
    ../../../QRMatrix/common.cpp
    ../../../QRMatrix/common.h
    ../../../QRMatrix/constants.h
    ../../../QRMatrix/qrmatrixboard.cpp
    ../../../QRMatrix/qrmatrixboard.h
    ../../../QRMatrix/qrmatrixencoder.cpp
    ../../../QRMatrix/qrmatrixencoder.h
    ../../../QRMatrix/qrmatrixsegment.cpp
    ../../../QRMatrix/qrmatrixsegment.h
    ../../../QRMatrix/Encoder/alphanumericencoder.cpp
    ../../../QRMatrix/Encoder/alphanumericencoder.h
    ../../../QRMatrix/Encoder/kanjiencoder.cpp
    ../../../QRMatrix/Encoder/kanjiencoder.h
    ../../../QRMatrix/Encoder/numericencoder.cpp
    ../../../QRMatrix/Encoder/numericencoder.h
    ../../../QRMatrix/Exception/qrmatrixexception.cpp
    ../../../QRMatrix/Exception/qrmatrixexception.h
    ../../../QRMatrix/Polynomial/polynomial.cpp
    ../../../QRMatrix/Polynomial/polynomial.h
    ../../../QRMatrix/qrmatrixextramode.h
    ../../../QRMatrix/qrmatrixextramode.cpp
    ../../../String/latinstring.cpp
    ../../../String/latinstring.h
    ../../../String/shiftjisstring.cpp
    ../../../String/shiftjisstring.h
    ../../../String/shiftjisstringmap.cpp
    ../../../String/shiftjisstringmap.h
    ../../../String/unicodepoint.cpp
    ../../../String/unicodepoint.h
    ../../../String/utf8string.cpp
    ../../../String/utf8string.h
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(QRMatrixExample
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
else()
    add_executable(QRMatrixExample
        ${PROJECT_SOURCES}
    )
endif()

target_link_libraries(QRMatrixExample PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

set_target_properties(QRMatrixExample PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

install(TARGETS QRMatrixExample
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(QRMatrixExample)
endif()
