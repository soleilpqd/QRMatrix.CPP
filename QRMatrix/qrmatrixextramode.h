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

#ifndef QRMATRIXEXTRAMODE_H
#define QRMATRIXEXTRAMODE_H

#include "constants.h"
#include "qrmatrixsegment.h"

namespace QRMatrix {

    enum EncodingExtraMode {
        /// none
        none,
        /// MicroQR
        microQr,
        /// FNC1 First Position
        fnc1First,
        /// FNC2 Second Position
        fnc1Second
    };

    struct QRMatrixExtraMode {
        /// Extra mode
        EncodingExtraMode mode;
        /// A C-String represents Application Indicator for FNC1 Second Position.
        /// 2 valid forms:
        ///  - Single ASCII character [a-z][A-Z] eg. `a`.
        ///  - Two ditis number eg. `01`
        UnsignedByte* appIndicator;
        UnsignedByte appIndicatorLength;

        ~QRMatrixExtraMode();
        QRMatrixExtraMode(QRMatrixExtraMode &other);
        void operator=(QRMatrixExtraMode other);
        /// Default init, none
        QRMatrixExtraMode();
        /// Init for MicroQR or FCN1 First Position mode
        QRMatrixExtraMode(EncodingExtraMode mode);
        /// Init for FCN2 Second Positon mode
        QRMatrixExtraMode(UnsignedByte* appId, UnsignedByte appIdLen);
    };

    struct QRMatrixStructuredAppend {
        /// Data segments
        QRMatrixSegment* segments;
        /// Count of segments
        unsigned int count;
        /// Error correction info
        ErrorCorrectionLevel level;
        /// Optional. Limit minimum version
        /// (result version = max(minimum version, required version to fit data).
        UnsignedByte minVersion;
        /// Optional. Force to use given mask (0-7).
        /// Almost for test, you can ignore this.
        UnsignedByte maskId;
        /// Extra mode (MicroQR will be ignored. Not sure about FNC1.)
        QRMatrixExtraMode extraMode;

        QRMatrixStructuredAppend(QRMatrixSegment* segs, unsigned int segCount, ErrorCorrectionLevel ecLevel);

        QRMatrixStructuredAppend();
        QRMatrixStructuredAppend(QRMatrixStructuredAppend &other);
        ~QRMatrixStructuredAppend();
        void operator=(QRMatrixStructuredAppend other);
    };

}

#endif // QRMATRIXEXTRAMODE_H
