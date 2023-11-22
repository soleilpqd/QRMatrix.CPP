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

#ifndef QRMATRIXSEGMENT_H
#define QRMATRIXSEGMENT_H

#include "constants.h"

namespace QRMatrix {

    const unsigned int defaultEciAssigmentValue = 3;

    /// Hold information of input data for 1 QR segment.
    struct QRMatrixSegment {
    public:
        ~QRMatrixSegment();
        QRMatrixSegment(QRMatrixSegment &other);
        /// Create QR segment
        QRMatrixSegment(
            /// Encoding mode
            EncodingMode mode,
            /// Bytes sequence to encode.
            /// If mode is `Numeric`, `data` must contain only ASCII bytes of characters `[0...9]`
            /// (or other text encoding if compatiple eg. UTF-8).
            /// If mode is `AlphaNumeric, `data` must contain only ASCII bytes of characters `[0...9][A...Z] $%*+-./:`
            /// (or other text encoding if compatiple eg. UTF -8).
            /// If mode is `Kanji`, `data` must contain only 2-bytes ShiftJIS characters
            /// (each 2-bytes must be in range [0x8140...0x9FFC] & [0xE040...0xEBBF]).
            const UnsignedByte* data,
            /// Number of `data` bytes
            unsigned int length,
            /// Enable ECI mode with given ECI Indicator (ECI Assigment value)
            unsigned int eciIndicator = defaultEciAssigmentValue
        );
        /// Create empty and set later
        QRMatrixSegment();
        /// Fill segment with given data
        void fill(EncodingMode mode, const UnsignedByte* data, unsigned int length, unsigned int eciIndicator = defaultEciAssigmentValue);
        void operator=(QRMatrixSegment other);

        inline EncodingMode mode() { return mode_; }
        inline unsigned int length() { return length_; }
        inline unsigned int eci() { return eci_; }
        inline UnsignedByte* data() { return data_; }
        /// 0 to ignore ECI Indicator.
        /// Default QR ECI indicator is 3, so we ignore too.
        /// MicroQR does not have ECI mode, so we ignore this in MicroQR.
        inline bool isEciHeaderRequired() { return eci_ != defaultEciAssigmentValue; }
    private:
        EncodingMode mode_;
        unsigned int length_;
        UnsignedByte* data_;
        unsigned int eci_;
    };

}

#endif // QRMATRIXSEGMENT_H
