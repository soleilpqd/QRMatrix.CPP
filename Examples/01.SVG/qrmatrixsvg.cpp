/*
    QRMatrix - QR pixels presentation.
    Copyright © 2023 duongpq/soleilpqd.

    Permission is hereby granted, free of charge, to any person obtaining a copy of
    this software and associated documentation files (the “Software”), to deal in
    the Software without restriction, including without limitation the rights to use,
    copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the
    Software, and to permit persons to whom the Software is furnished to do so, subject
    to the following conditions:

    The above copyright notice and this permission notice shall be included in all copies
    or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
    INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
    PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE
    FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
    OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
    DEALINGS IN THE SOFTWARE.
*/

#include "qrmatrixsvg.h"
#include <fstream>
#include <iostream>
#include <string>

using namespace std;
using namespace QRMatrix;

void QRMatrixSvg::draw(QRMatrix::QRMatrixBoard board, const char* path, unsigned int scale, bool isMicro) {
    if (scale < 1) {
        throw invalid_argument("Scale > 0");
    }
    unsigned int quietZone = isMicro ? 2 : 4;
    ofstream outFile;
    outFile.open(path);
    // <svg>
    unsigned int dimension = board.dimension() * scale + 2 * quietZone * scale;
    string dimensionStr = to_string(dimension);
    outFile << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\" width=\"" << dimensionStr.c_str()
            << "\" height=\"" << dimensionStr.c_str()
            << "\">\n";
    outFile << "    <rect fill=\"white\" stroke=\"none\""
            << " width=\"" << dimensionStr.c_str()
            << "\" height=\""<< dimensionStr.c_str()
            << "\" x=\"0\" y=\"0\"/>\n";
    outFile << "    <g fill=\"black\" stroke=\"none\">\n";
    // Content
    UnsignedByte** data = board.buffer();
    string cellSizeStr = to_string(scale);
    for (unsigned int row = 0; row < board.dimension(); row += 1) {
        for (unsigned int column = 0; column < board.dimension(); column += 1) {
            UnsignedByte cell = data[row][column];
            UnsignedByte low = cell & BoardCell::lowMask;
            if (low == BoardCell::set) {
                unsigned int x = (quietZone + column) * scale;
                unsigned int y = (quietZone + row) * scale;

                outFile << "        <rect width=\"" << cellSizeStr.c_str()
                        << "\" height=\""<< cellSizeStr.c_str()
                        << "\" x=\"" << to_string(x).c_str()
                        << "\" y=\"" << to_string(y).c_str()
                        << "\"/>\n";
            }
        }
    }
    outFile << "    </g>\n";
    // </svg>
    outFile << "</svg>\n";
    outFile.close();
}

void QRMatrixSvg::drawDetail(
    QRMatrix::QRMatrixBoard board,
    const char* path,
    unsigned int scale,
    bool isMicro,
    const char* backgroundColor,
    const char* dataColor,
    const char* finderColor,
    const char* timingColor,
    const char* darkColor,
    const char* aligmentColor,
    const char* versionColor,
    const char* formatColor,
    const char* ecColor,
    const char* remainderColor
) {
    if (scale < 1) {
        throw invalid_argument("Scale > 0");
    }
    unsigned int quietZone = isMicro ? 2 : 4;
    ofstream outFile;
    outFile.open(path);
    // <svg>
    unsigned int dimension = board.dimension() * scale + 2 * quietZone * scale;
    string dimensionStr = to_string(dimension);
    outFile << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\" width=\"" << dimensionStr.c_str()
            << "\" height=\"" << dimensionStr.c_str()
            << "\">\n";
    outFile << "    <rect fill=\"" << backgroundColor << "\" stroke=\"none\""
            << " width=\"" << dimensionStr.c_str()
            << "\" height=\""<< dimensionStr.c_str()
            << "\" x=\"0\" y=\"0\"/>\n";
    // Data cells
    outFile << "    <!-- Data cells -->\n";
    outFile << "    <g fill=\"" << dataColor << "\" stroke=\"none\">\n";
    UnsignedByte** data = board.buffer();
    string cellSizeStr = to_string(scale);
    for (unsigned int row = 0; row < board.dimension(); row += 1) {
        for (unsigned int column = 0; column < board.dimension(); column += 1) {
            UnsignedByte cell = data[row][column];
            UnsignedByte high = cell & BoardCell::highMask;
            UnsignedByte low = cell & BoardCell::lowMask;
            if (low == BoardCell::set && high == 0) {
                unsigned int x = (quietZone + column) * scale;
                unsigned int y = (quietZone + row) * scale;

                outFile << "        <rect width=\"" << cellSizeStr.c_str()
                        << "\" height=\""<< cellSizeStr.c_str()
                        << "\" x=\"" << to_string(x).c_str()
                        << "\" y=\"" << to_string(y).c_str()
                        << "\"/>\n";
            }
        }
    }
    outFile << "    </g>\n";
    // Finder cells
    outFile << "    <!-- Finder cells -->\n";
    outFile << "    <g fill=\"" << finderColor << "\" stroke=\"none\">\n";
    for (unsigned int row = 0; row < board.dimension(); row += 1) {
        for (unsigned int column = 0; column < board.dimension(); column += 1) {
            UnsignedByte cell = data[row][column];
            UnsignedByte high = cell & BoardCell::highMask;
            UnsignedByte low = cell & BoardCell::lowMask;
            if (low == BoardCell::set && high == BoardCell::finder) {
                unsigned int x = (quietZone + column) * scale;
                unsigned int y = (quietZone + row) * scale;

                outFile << "        <rect width=\"" << cellSizeStr.c_str()
                        << "\" height=\""<< cellSizeStr.c_str()
                        << "\" x=\"" << to_string(x).c_str()
                        << "\" y=\"" << to_string(y).c_str()
                        << "\"/>\n";
            }
        }
    }
    outFile << "    </g>\n";
    // Timing cells
    outFile << "    <!-- Timing cells -->\n";
    outFile << "    <g fill=\"" << timingColor << "\" stroke=\"none\">\n";
    for (unsigned int row = 0; row < board.dimension(); row += 1) {
        for (unsigned int column = 0; column < board.dimension(); column += 1) {
            UnsignedByte cell = data[row][column];
            UnsignedByte high = cell & BoardCell::highMask;
            UnsignedByte low = cell & BoardCell::lowMask;
            if (low == BoardCell::set && high == BoardCell::timing) {
                unsigned int x = (quietZone + column) * scale;
                unsigned int y = (quietZone + row) * scale;

                outFile << "        <rect width=\"" << cellSizeStr.c_str()
                        << "\" height=\""<< cellSizeStr.c_str()
                        << "\" x=\"" << to_string(x).c_str()
                        << "\" y=\"" << to_string(y).c_str()
                        << "\"/>\n";
            }
        }
    }
    outFile << "    </g>\n";
    // Aligment cells
    outFile << "    <!-- Aligment cells -->\n";
    outFile << "    <g fill=\"" << aligmentColor << "\" stroke=\"none\">\n";
    for (unsigned int row = 0; row < board.dimension(); row += 1) {
        for (unsigned int column = 0; column < board.dimension(); column += 1) {
            UnsignedByte cell = data[row][column];
            UnsignedByte high = cell & BoardCell::highMask;
            UnsignedByte low = cell & BoardCell::lowMask;
            if (low == BoardCell::set && high == BoardCell::alignment) {
                unsigned int x = (quietZone + column) * scale;
                unsigned int y = (quietZone + row) * scale;

                outFile << "        <rect width=\"" << cellSizeStr.c_str()
                        << "\" height=\""<< cellSizeStr.c_str()
                        << "\" x=\"" << to_string(x).c_str()
                        << "\" y=\"" << to_string(y).c_str()
                        << "\"/>\n";
            }
        }
    }
    outFile << "    </g>\n";
    // Version cells
    outFile << "    <!-- Version info cells -->\n";
    outFile << "    <g fill=\"" << versionColor << "\" stroke=\"none\">\n";
    for (unsigned int row = 0; row < board.dimension(); row += 1) {
        for (unsigned int column = 0; column < board.dimension(); column += 1) {
            UnsignedByte cell = data[row][column];
            UnsignedByte high = cell & BoardCell::highMask;
            UnsignedByte low = cell & BoardCell::lowMask;
            if (low == BoardCell::set && high == BoardCell::version) {
                unsigned int x = (quietZone + column) * scale;
                unsigned int y = (quietZone + row) * scale;

                outFile << "        <rect width=\"" << cellSizeStr.c_str()
                        << "\" height=\""<< cellSizeStr.c_str()
                        << "\" x=\"" << to_string(x).c_str()
                        << "\" y=\"" << to_string(y).c_str()
                        << "\"/>\n";
            }
        }
    }
    outFile << "    </g>\n";
    // Format cells
    outFile << "    <!-- Format info cells -->\n";
    outFile << "    <g fill=\"" << formatColor << "\" stroke=\"none\">\n";
    for (unsigned int row = 0; row < board.dimension(); row += 1) {
        for (unsigned int column = 0; column < board.dimension(); column += 1) {
            UnsignedByte cell = data[row][column];
            UnsignedByte high = cell & BoardCell::highMask;
            UnsignedByte low = cell & BoardCell::lowMask;
            if (low == BoardCell::set && high == BoardCell::format) {
                unsigned int x = (quietZone + column) * scale;
                unsigned int y = (quietZone + row) * scale;

                outFile << "        <rect width=\"" << cellSizeStr.c_str()
                        << "\" height=\""<< cellSizeStr.c_str()
                        << "\" x=\"" << to_string(x).c_str()
                        << "\" y=\"" << to_string(y).c_str()
                        << "\"/>\n";
            }
        }
    }
    outFile << "    </g>\n";
    // Dark cell
    outFile << "    <!-- Dark cells -->\n";
    outFile << "    <g fill=\"" << darkColor << "\" stroke=\"none\">\n";
    UnsignedByte cell = data[board.dimension() - 8][8];
    UnsignedByte high = cell & BoardCell::highMask;
    UnsignedByte low = cell & BoardCell::lowMask;
    if (low == BoardCell::set && high == BoardCell::dark) {
        unsigned int x = (quietZone + 8) * scale;
        unsigned int y = (quietZone + board.dimension() - 8) * scale;

        outFile << "        <rect width=\"" << cellSizeStr.c_str()
                << "\" height=\""<< cellSizeStr.c_str()
                << "\" x=\"" << to_string(x).c_str()
                << "\" y=\"" << to_string(y).c_str()
                << "\"/>\n";
    }
    outFile << "    </g>\n";
    // ErrorCorrection cells
    outFile << "    <!-- Error correction cells -->\n";
    outFile << "    <g fill=\"" << ecColor << "\" stroke=\"none\">\n";
    for (unsigned int row = 0; row < board.dimension(); row += 1) {
        for (unsigned int column = 0; column < board.dimension(); column += 1) {
            UnsignedByte cell = data[row][column];
            UnsignedByte high = cell & BoardCell::highMask;
            UnsignedByte low = cell & BoardCell::lowMask;
            if (low == BoardCell::set && high == BoardCell::errorCorrection) {
                unsigned int x = (quietZone + column) * scale;
                unsigned int y = (quietZone + row) * scale;

                outFile << "        <rect width=\"" << cellSizeStr.c_str()
                        << "\" height=\""<< cellSizeStr.c_str()
                        << "\" x=\"" << to_string(x).c_str()
                        << "\" y=\"" << to_string(y).c_str()
                        << "\"/>\n";
            }
        }
    }
    outFile << "    </g>\n";
    // Remainder cells
    outFile << "    <!-- Remainder cells -->\n";
    outFile << "    <g fill=\"" << remainderColor << "\" stroke=\"none\">\n";
    for (unsigned int row = 0; row < board.dimension(); row += 1) {
        for (unsigned int column = 0; column < board.dimension(); column += 1) {
            UnsignedByte cell = data[row][column];
            UnsignedByte high = cell & BoardCell::highMask;
            UnsignedByte low = cell & BoardCell::lowMask;
            if (low == BoardCell::set && high == BoardCell::remainder) {
                unsigned int x = (quietZone + column) * scale;
                unsigned int y = (quietZone + row) * scale;

                outFile << "        <rect width=\"" << cellSizeStr.c_str()
                        << "\" height=\""<< cellSizeStr.c_str()
                        << "\" x=\"" << to_string(x).c_str()
                        << "\" y=\"" << to_string(y).c_str()
                        << "\"/>\n";
            }
        }
    }
    outFile << "    </g>\n";
    // </svg>
    outFile << "</svg>\n";
    outFile.close();
}
