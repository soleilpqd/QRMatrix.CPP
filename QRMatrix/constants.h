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

// REFERENCE: https://www.thonky.com/qr-code-tutorial/

#ifndef CONSTANTS_H
#define CONSTANTS_H

#define LOGABLE 0

#define VERSION "1.1.0"

using UnsignedByte      = unsigned char;
using Unsigned2Bytes    = unsigned short;
using Unsigned4Bytes    = unsigned int;

namespace QRMatrix {

    /// QR Encoding Mode
    enum EncodingMode {
        /// 0-9
        numeric = 0b0001,
        /// 0-9, A-Z, $, %, *, +, -, ., /, :, ' '
        alphaNumeric = 0b0010,
        /// ISO-8859-1 (Unicode Latin-1)
        byte = 0b0100,
        /// Shift JIS double-byte characters
        kanji = 0b1000
    };

    /// QR Error correction level
    enum ErrorCorrectionLevel {
        /// L 7%
        low = 0b01,
        /// M 15%
        medium = 0b00,
        /// Q 25%
        quarter = 0b11,
        /// H 30%
        high = 0b10
    };

    /// Maximum QR version
    const UnsignedByte maximumVersion = 40;
    /// Different dimension amount between 2 sequential versions
    const UnsignedByte versionsOffset = 4;
    /// QR minimum dimension
    const UnsignedByte minimumDimension = 21;

    /// Maximum MicroQR version
    const UnsignedByte maximumMicroVersion = 4;
    /// Different dimension amount between 2 sequential micro versions
    const UnsignedByte versionsMicroOffset = 2;
    /// Micro QR minimum dimension
    const UnsignedByte minimumMicroDimension = 11;

}

#endif // CONSTANTS_H
