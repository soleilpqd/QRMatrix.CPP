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

#include "qrmatrixsegment.h"
#include "common.h"
#include "Exception/qrmatrixexception.h"
#include <cstring>
#include <string>

using namespace QRMatrix;
using namespace std;

// Data Validation ----------------------------------------------------------------------------------------------------------------------------------

void validateNumeric(const UnsignedByte* data, unsigned int length) {
    static const char* pattern = "0123456789";
    for (unsigned int index = 0; index < length; index += 1) {
        UnsignedByte byte = data[index];
        if (strchr(pattern, (char)byte) == NULL) {
            string mesg = "Invalid data for Numeric mode [";
            mesg.append(std::to_string(index));
            mesg.append("] ");
            mesg.append(std::to_string(byte));
            throw QR_EXCEPTION(mesg.c_str());
        }
    }
}

void validateAlphaNumeric(const UnsignedByte* data, unsigned int length) {
    static const char* pattern = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ $%*+-./:";
    for (unsigned int index = 0; index < length; index += 1) {
        UnsignedByte byte = data[index];
        if (strchr(pattern, (char)byte) == NULL) {
            string mesg = "Invalid data for AlphaNumeric mode [";
            mesg.append(std::to_string(index));
            mesg.append("] ");
            mesg.append(std::to_string(byte));
            throw QR_EXCEPTION(mesg.c_str());
        }
    }
}

void validateKanji(const UnsignedByte* data, unsigned int length) {
    // Only accept 2 bytes ShiftJIS characters
    if ((length % 2) > 0) {
        throw QR_EXCEPTION("Invalid data for Kanji mode");
    }
    for (unsigned int index = 0; index < length; index += 2) {
        Unsigned2Bytes curChar = 0;
        UnsignedByte* curCharPtr = (UnsignedByte*)&curChar;
        if (Common::isLittleEndian) {
            *curCharPtr = data[index + 1];
            *(curCharPtr + 1) = data[index];
        } else {
            *curCharPtr = data[index];
            *(curCharPtr + 1) = data[index + 1];
        }
        bool isValid = (
            (curChar >= 0x8140) && (curChar <= 0x9FFC)) ||
            ((curChar >= 0xE040) && (curChar <= 0xEBBF)
        );
        if (!isValid) {
            string mesg = "Invalid data for Kanji mode [";
            mesg.append(std::to_string(index));
            mesg.append("] ");
            mesg.append(std::to_string(data[index]));
            mesg.append(" ");
            mesg.append(std::to_string(data[index + 1]));
            mesg.append(": ");
            mesg.append(std::to_string(curChar));
            throw QR_EXCEPTION(mesg.c_str());
        }
    }
}

void validateInputBytes(EncodingMode mode, const UnsignedByte* data, unsigned int length) {
    switch (mode) {
    case EncodingMode::numeric:
        validateNumeric(data, length);
        break;
    case EncodingMode::alphaNumeric:
        validateAlphaNumeric(data, length);
        break;
    case EncodingMode::kanji:
        validateKanji(data, length);
        break;
    default:
        break;
    }
}

// PUBLIC -------------------------------------------------------------------------------------------------------------------------------------------

QRMatrixSegment::~QRMatrixSegment() {
    if (data_ != NULL) {
        delete[] data_;
    }
}

QRMatrixSegment::QRMatrixSegment(QRMatrixSegment &other) {
    length_ = other.length_;
    mode_ = other.mode_;
    eci_ = other.eci_;
    if (length_ > 0) {
        data_ = Common::allocate(length_);
        for (unsigned int index = 0; index < length_; index += 1) {
            data_[index] = other.data_[index];
        }
    } else {
        data_ = NULL;
    }
}

QRMatrixSegment::QRMatrixSegment(EncodingMode mode, const UnsignedByte *data, unsigned int length, unsigned int eciIndicator) {
    validateInputBytes(mode, data, length);
    length_ = length;
    mode_ = mode;
    eci_ = eciIndicator;
    if (length_ > 0) {
        data_ = Common::allocate(length_);
        for (unsigned int index = 0; index < length_; index += 1) {
            data_[index] = data[index];
        }
    } else {
        data_ = NULL;
    }
}

QRMatrixSegment::QRMatrixSegment() {
    length_ = 0;
    mode_ = EncodingMode::byte;
    eci_ = defaultEciAssigmentValue;
    data_ = NULL;
}

void QRMatrixSegment::fill(EncodingMode mode, const UnsignedByte *data, unsigned int length, unsigned int eciIndicator) {
    validateInputBytes(mode, data, length);
    if (data_ != NULL) {
        delete[] data_;
    }
    length_ = length;
    mode_ = mode;
    eci_ = eciIndicator;
    if (length_ > 0) {
        data_ = Common::allocate(length_);
        for (unsigned int index = 0; index < length_; index += 1) {
            data_[index] = data[index];
        }
    } else {
        data_ = NULL;
    }
}

void QRMatrixSegment::operator=(QRMatrixSegment other) {
    if (data_ != NULL) {
        delete[] data_;
    }
    length_ = other.length_;
    mode_ = other.mode_;
    eci_ = other.eci();
    if (length_ > 0) {
        data_ = Common::allocate(length_);
        for (unsigned int index = 0; index < length_; index += 1) {
            data_[index] = other.data_[index];
        }
    } else {
        data_ = NULL;
    }
}
