cmake_minimum_required(VERSION 3.5)

project(QRMatrix LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Source
set(PROJECT_SOURCES
    main.cpp
#    ../../DevTools/devtools.cpp
#    ../../DevTools/devtools.h
    ../../QRMatrix/common.cpp
    ../../QRMatrix/common.h
    ../../QRMatrix/constants.h
    ../../QRMatrix/qrmatrixboard.cpp
    ../../QRMatrix/qrmatrixboard.h
    ../../QRMatrix/qrmatrixencoder.cpp
    ../../QRMatrix/qrmatrixencoder.h
    ../../QRMatrix/qrmatrixsegment.cpp
    ../../QRMatrix/qrmatrixsegment.h
    ../../QRMatrix/Encoder/alphanumericencoder.cpp
    ../../QRMatrix/Encoder/alphanumericencoder.h
    ../../QRMatrix/Encoder/kanjiencoder.cpp
    ../../QRMatrix/Encoder/kanjiencoder.h
    ../../QRMatrix/Encoder/numericencoder.cpp
    ../../QRMatrix/Encoder/numericencoder.h
    ../../QRMatrix/Exception/qrmatrixexception.cpp
    ../../QRMatrix/Exception/qrmatrixexception.h
    ../../QRMatrix/Polynomial/polynomial.cpp
    ../../QRMatrix/Polynomial/polynomial.h
    ../../QRMatrix/qrmatrixextramode.h
    ../../QRMatrix/qrmatrixextramode.cpp
    ../../String/latinstring.cpp
    ../../String/latinstring.h
    ../../String/shiftjisstring.cpp
    ../../String/shiftjisstring.h
    ../../String/shiftjisstringmap.cpp
    ../../String/shiftjisstringmap.h
    ../../String/unicodepoint.cpp
    ../../String/unicodepoint.h
    ../../String/utf8string.cpp
    ../../String/utf8string.h
    qrmatrixsvg.h
    qrmatrixsvg.cpp
)

add_executable(QRMatrix ${PROJECT_SOURCES})

install(TARGETS QRMatrix
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
