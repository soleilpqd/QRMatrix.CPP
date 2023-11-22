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

#ifndef QRMATRIXSVG_H
#define QRMATRIXSVG_H

#include "../../QRMatrix/qrmatrixboard.h"

class QRMatrixSvg {
public:
    static void draw(
        /// QR Matrix to draw
        QRMatrix::QRMatrixBoard board,
        /// File path to write to
        const char* path,
        /// How many pixels per QR cell
        unsigned int scale = 1,
        /// MicroQR
        bool isMicro = false
    );
    static void drawDetail(
        /// QR Matrix to draw
        QRMatrix::QRMatrixBoard board,
        /// File path to write to
        const char* path,
        /// How many pixels per QR cell
        unsigned int scale = 1,
        /// MicroQR
        bool isMicro = false,
        /// Background color
        const char* backgroundColor = "white",
        /// Data cell color
        const char* dataColor = "black",
        /// Finder cell color
        const char* finderColor = "blue",
        /// Timing cell color
        const char* timingColor = "green",
        /// Dark cell color
        const char* darkColor = "red",
        /// Aligment cell color
        const char* aligmentColor = "purple",
        /// Version cell color
        const char* versionColor = "orange",
        /// Format color
        const char* formatColor = "magenta",
        /// EC color
        const char* ecColor = "darkblue",
        /// Remainder color
        const char* remainderColor = "darkgreen"
    );
};

#endif // QRMATRIXSVG_H
