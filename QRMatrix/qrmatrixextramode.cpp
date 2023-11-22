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

#include "qrmatrixextramode.h"

using namespace QRMatrix;

QRMatrixExtraMode::~QRMatrixExtraMode() {
    if (appIndicator != nullptr) {
        delete[] appIndicator;
    }
}

QRMatrixExtraMode::QRMatrixExtraMode() {
    mode = EncodingExtraMode::none;
    appIndicator = nullptr;
    appIndicatorLength = 0;
}

QRMatrixExtraMode::QRMatrixExtraMode(EncodingExtraMode mode) {
    this->mode = mode;
    appIndicator = nullptr;
    appIndicatorLength = 0;
}

QRMatrixExtraMode::QRMatrixExtraMode(UnsignedByte* appId, UnsignedByte appIdLen) {
    mode = EncodingExtraMode::fnc1Second;
    appIndicatorLength = appIdLen;
    appIndicator = new UnsignedByte [appIdLen];
    for (unsigned int index = 0; index < appIdLen; index += 1) {
        appIndicator[index] = appId[index];
    }
}

QRMatrixExtraMode::QRMatrixExtraMode(QRMatrixExtraMode &other) {
    mode = other.mode;
    appIndicatorLength = other.appIndicatorLength;
    appIndicator = new UnsignedByte [appIndicatorLength];
    for (unsigned int index = 0; index < appIndicatorLength; index += 1) {
        appIndicator[index] = other.appIndicator[index];
    }
}

void QRMatrixExtraMode::operator=(QRMatrixExtraMode other) {
    if (appIndicator != nullptr) {
        delete appIndicator;
    }
    mode = other.mode;
    appIndicatorLength = other.appIndicatorLength;
    appIndicator = new UnsignedByte [appIndicatorLength];
    for (unsigned int index = 0; index < appIndicatorLength; index += 1) {
        appIndicator[index] = other.appIndicator[index];
    }
}

QRMatrixStructuredAppend::QRMatrixStructuredAppend(QRMatrixSegment* segs, unsigned int segCount, ErrorCorrectionLevel ecLevel) {
    count = segCount;
    segments = new QRMatrixSegment [count];
    for (unsigned int index = 0; index < count; index += 1) {
        segments[index] = segs[index];
    }
    level = ecLevel;
    minVersion = 0;
    maskId = 0xFF;
    extraMode = QRMatrixExtraMode();
}

QRMatrixStructuredAppend::~QRMatrixStructuredAppend() {
    delete[] segments;
}

QRMatrixStructuredAppend::QRMatrixStructuredAppend() {
    count = 0;
    segments = segments = new QRMatrixSegment [count];
    level = ErrorCorrectionLevel::low;
    minVersion = 0;
    maskId = 0xFF;
    extraMode = QRMatrixExtraMode();
}

QRMatrixStructuredAppend::QRMatrixStructuredAppend(QRMatrixStructuredAppend &other) {
    count = other.count;
    segments = new QRMatrixSegment [count];
    for (unsigned int index = 0; index < count; index += 1) {
        segments[index] = other.segments[index];
    }
    level = other.level;
    minVersion = other.minVersion;
    maskId = other.maskId;
    extraMode = other.extraMode;
}

void QRMatrixStructuredAppend::operator=(QRMatrixStructuredAppend other) {
    delete[] segments;
    count = other.count;
    segments = new QRMatrixSegment [count];
    for (unsigned int index = 0; index < count; index += 1) {
        segments[index] = other.segments[index];
    }
    level = other.level;
    minVersion = other.minVersion;
    maskId = other.maskId;
    extraMode = other.extraMode;
}
