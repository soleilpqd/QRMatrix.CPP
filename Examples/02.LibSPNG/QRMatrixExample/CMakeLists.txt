cmake_minimum_required(VERSION 3.5)

project(QRMatrixExample LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (APPLE) # libspng from HomeBrew
    # /usr/local/include/spng.h
    include_directories(/usr/local/include)
    # /usr/local/lib/libspng.dylib
    link_directories(/usr/local/lib)
elseif (LINUX) # libspng from Fedora (x64) repository
    # /usr/include/spng.h
    include_directories(/usr/include)
    # /usr/lib64/libspng.so
    link_directories(/usr/lib64)
endif()

# Source
set(PROJECT_SOURCES
    main.cpp
#    ../../../DevTools/devtools.h
#    ../../../DevTools/devtools.cpp
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

add_executable(QRMatrixExample ${PROJECT_SOURCES})

target_link_libraries(QRMatrixExample spng)

install(TARGETS QRMatrixExample
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
