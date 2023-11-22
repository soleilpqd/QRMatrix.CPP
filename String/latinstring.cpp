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

#include "latinstring.h"
#include "../QRMatrix/Exception/qrmatrixexception.h"
#include "../QRMatrix/common.h"

using namespace QRMatrix;

LatinString::~LatinString() {
    delete[] rawString_;
}

LatinString::LatinString(LatinString &other) {
    isValid_ = other.isValid_;
    charCount_ = other.charCount_;
    rawString_ = Common::allocate(charCount_);
    for (unsigned int index = 0; index < charCount_; index += 1) {
        rawString_[index] = other.rawString_[index];
    }
}

LatinString::LatinString(const UnsignedByte* raw, const unsigned int length) {
    charCount_ = length;
    rawString_ = Common::allocate(length);
    isValid_ = true;
    for (unsigned int index = 0; index < charCount_; index += 1) {
        UnsignedByte value = raw[index];
        rawString_[index] = value;
        if ((value < 0x20 || (0x7E < value && value < 0xA0)) && value != '\n' && value != '\r' && value != '\t') {
            isValid_ = false;
            throw QR_EXCEPTION("Given bytes contains byte out of range of ISO/IEC 8859-1 charset.");
        }
    }
}

LatinString::LatinString(UnicodePoint unicodes) {
    charCount_ = unicodes.charactersCount();
    rawString_ = Common::allocate(charCount_);
    isValid_ = true;
    const Unsigned4Bytes* points = unicodes.unicodePoints();
    for (unsigned int index = 0; index < charCount_; index += 1) {
        Unsigned4Bytes value = points[index];
        rawString_[index] = (UnsignedByte)value;
        if ((value < 0x20 || (0x7E < value && value < 0xA0) || value > 0xFF) && value != '\n' && value != '\r' && value != '\t') {
            isValid_ = false;
            throw QR_EXCEPTION("Given unicodes contain character out of range of ISO/IEC 8859-1 charset.");
        }
    }
}

UnicodePoint LatinString::unicodes() {
    if (!isValid_) {
        throw QR_EXCEPTION("This object is invalid so can not perform the requested action.");
    }
    UnicodePoint result = UnicodePoint(charCount_);
    for (unsigned int index = 0; index < charCount_; index += 1) {
        Unsigned4Bytes value = (Unsigned4Bytes)rawString_[index];
        result.setCharacter(value, index);
    }
    return result;
}
