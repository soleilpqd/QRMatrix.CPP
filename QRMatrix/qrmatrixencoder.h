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

#ifndef QRMATRIXENCODER_H
#define QRMATRIXENCODER_H

#include "constants.h"
#include "qrmatrixboard.h"
#include "Exception/qrmatrixexception.h"
#include "qrmatrixsegment.h"
#include "qrmatrixextramode.h"

namespace QRMatrix {

    class QRMatrixEncoder {
    public:

        /// Get QR Version (dimension) to encode given data.
        /// @return 0 if no suiversion
        static UnsignedByte getVersion(
            /// Array of segments to be encoded
            QRMatrixSegment* segments,
            /// Number of segments
            unsigned int count,
            /// Error correction info
            ErrorCorrectionLevel level,
            /// Extra mode
            QRMatrixExtraMode extraMode = QRMatrixExtraMode(),
            /// Is this symbol a part of Structured Append
            bool isStructuredAppend = false
        );

        /// Encode single QR symbol
        static QRMatrixBoard encode(
            /// Array of segments to be encoded
            QRMatrixSegment* segments,
            /// Number of segments
            unsigned int count,
            /// Error correction info
            ErrorCorrectionLevel level,
            /// Extra mode
            QRMatrixExtraMode extraMode = QRMatrixExtraMode(),
            /// Optional. Limit minimum version
            /// (result version = max(minimum version, required version to fit data).
            UnsignedByte minVersion = 0,
            /// Optional. Force to use given mask (0-7).
            /// Almost for test, you can ignore this.
            UnsignedByte maskId = 0xFF
        );

        /// Encode Structured Append QR symbols
        /// @return Array of QRMatrixBoard (should be deleted when done).
        static QRMatrixBoard* encode(
            /// Array of data parts to be encoded
            QRMatrixStructuredAppend* parts,
            /// Number of parts
            unsigned int count
        );

    };

}

#endif // QRMATRIXENCODER_H
