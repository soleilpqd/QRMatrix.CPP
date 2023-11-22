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

#ifndef QRMATRIXBOARD_H
#define QRMATRIXBOARD_H

#include "constants.h"
#include <string>

namespace QRMatrix {

    /// Internal data model
    struct ErrorCorrectionInfo;

    /// Value of QR board cell
    enum BoardCell {
        /// Cell is not filled yet
        neutral         = 0x00,
        /// Cell is white (lower 4 bits)
        unset           = 0x05,
        /// Cell is black (lower 4 bits)
        set             = 0x0A,
        /// Cell is finder (higher 4 bits)
        finder          = 0x10,
        /// Cell is separator (higher 4 bits)
        separator        = 0x20,
        /// Cell is alignment (higher 4 bits)
        alignment       = 0x30,
        /// Cell is timing (higher 4 bits)
        timing          = 0x40,
        /// Cell is dark module (higher 4 bits)
        dark            = 0x50,
        /// Cell is format (higher 4 bits)
        format  = 0x60,
        /// Cell is version (higher 4 bits) (version ≥ 7)
        version = 0x70,
        /// Cell is ErrorCorrection
        errorCorrection = 0x80,
        /// Cell is Remainder
        remainder = 0x90,

        /// cell & lowMask = lower 4 bits value
        lowMask         = 0x0F,
        /// cell & highMask = higher 4 bits value
        highMask        = 0xF0,
        /// cell & funcMask > 0 => cell is function module
        funcMask        = 0x70
    };

    /// QR cells (modules) (not inclues quiet zone)
    class QRMatrixBoard {
    public:
        ~QRMatrixBoard();
        QRMatrixBoard(QRMatrixBoard &other);
        void operator=(QRMatrixBoard other);
        /// Place holder. Internal purpose. Do not use.
        QRMatrixBoard();
        /// To create QR board, refer `QRMatrixEncoder`.
        /// This constructor is for internal purpose.
        QRMatrixBoard(UnsignedByte* data, UnsignedByte* errorCorrection, ErrorCorrectionInfo ecInfo, UnsignedByte maskId, bool isMicro);

        /// Size (dimension - number of cells on each side)
        inline UnsignedByte dimension() { return dimension_; }
        /// UnsignedByte[row][column]: 1 byte per cell.
        /// High word (left 4 bits) is cell type.
        /// Low word (right 4 bits) is black or white.
        /// See `BoardCell` for values.
        inline UnsignedByte** buffer() { return buffer_; }

        /// To print to Console
        std::string description(bool isTypeVisible = false);
    private:
        UnsignedByte dimension_;
        UnsignedByte** buffer_;
    };

}


#endif // QRMATRIXBOARD_H
