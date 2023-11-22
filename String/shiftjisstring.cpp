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

#include "shiftjisstring.h"
#include "../QRMatrix/Exception/qrmatrixexception.h"
#include "../QRMatrix/common.h"
#include "shiftjisstringmap.h"

using namespace QRMatrix;

ShiftJisString::~ShiftJisString() {
    delete[] rawString_;
    delete[] charsMap_;
}

ShiftJisString::ShiftJisString(ShiftJisString &other) {
    isValid_ = other.isValid_;
    charCount_ = other.charCount_;
    byteCount_ = other.byteCount_;
    minBytesPerChar_ = other.minBytesPerChar_;
    maxBytesPerChar_ = other.maxBytesPerChar_;
    rawString_ = Common::allocate(byteCount_);
    for (unsigned int index = 0; index < byteCount_; index += 1) {
        rawString_[index] = other.rawString_[index];
    }
    charsMap_ = Common::allocate(charCount_);
    for (unsigned int index = 0; index < charCount_; index += 1) {
        charsMap_[index] = other.charsMap_[index];
    }
}

ShiftJisString::ShiftJisString(const UnsignedByte* raw, unsigned int length) {
    isValid_ = false;
    charCount_ = 0;
    byteCount_ = length;
    minBytesPerChar_ = 2;
    maxBytesPerChar_ = 0;

    charsMap_ = Common::allocate(length);
    rawString_ = Common::allocate(length);
    unsigned int indexToCheck = 0;
    for (unsigned int index = 0; index < byteCount_; index += 1) {
        UnsignedByte byte = raw[index];
        if (index == indexToCheck) {
            UnsignedByte charSize = 0;
            if ((byte >= 0x20 && byte <= 0x7E) || (byte >= 0xA1 && byte <= 0xDF) || byte == '\r' || byte == '\n' || byte == '\t') {
                // Single byte character
                charSize = 1;
            } else if ((byte >= 0x81 && byte <= 0x9F) || (byte >= 0xE0 && byte <= 0xFC)) {
                // Double bytes character
                charSize = 2;
            } else {
                throw QR_EXCEPTION("Given data contains invalid byte.");
            }
            charsMap_[charCount_] = charSize;
            charCount_ += 1;
            if (maxBytesPerChar_ < charSize) {
                maxBytesPerChar_ = charSize;
            }
            if (minBytesPerChar_ > charSize) {
                minBytesPerChar_ = charSize;
            }
            indexToCheck += charSize;
        } else {
            // Second byte of double bytes characters
            if ((byte < 0x40) || (byte == 0x7F) || (byte > 0xFC)) {
                throw QR_EXCEPTION("Given data contains invalid byte.");
            }
        }
        rawString_[index] = byte;
    }
    isValid_ = true;
}

ShiftJisString::ShiftJisString(UnicodePoint unicodes) {
    isValid_ = false;
    charCount_ = unicodes.charactersCount();
    byteCount_ = 0;
    minBytesPerChar_ = 2;
    maxBytesPerChar_ = 0;

    Unsigned2Bytes (*map1)[188] = ShiftJisString_KanjiUnicode1Map(); // 31
    Unsigned2Bytes (*map2)[188] = ShiftJisString_KanjiUnicode2Map(); // 29

    charsMap_ = Common::allocate(charCount_);

    UnsignedByte* buffer = Common::allocate(charCount_ * 2);
    UnsignedByte* bufferPtr = buffer;

    const Unsigned4Bytes *points = unicodes.unicodePoints();
    for (unsigned int index = 0; index < charCount_; index += 1) {
        Unsigned4Bytes point = points[index];
        UnsignedByte charSize = 0;
        // Special case
        if (point == 0xA5) { // '¥'
            *bufferPtr = 0x5C; // '\'
            charSize = 1;
        } else if (point == 0x203E) { // '‾'
            *bufferPtr = 0x7E; // '~'
            charSize = 1;
        } else if (point == 0x7E) { // '~'
            *bufferPtr = 0x81;
            *(bufferPtr + 1) = 0x60; // '～'
            charSize = 2;
        } else  if (point == 0x5C) { // '\'
            *bufferPtr = 0x81;
            *(bufferPtr + 1) = 0x5F; // '＼'
            charSize = 2;
        } else if ((point >= 0x20 && point < 0x7E) || (point >= 0xFF61 && point <= 0xFF9F) || point == '\r' || point == '\n' || point == '\t') {
            // Single byte character
            charSize = 1;
            if (point < 0x7E) {
                *bufferPtr = (UnsignedByte)point;
            } else {
                Unsigned4Bytes tmp = point - 0xFF61 + 0xA1;
                *bufferPtr = (UnsignedByte)tmp;
            }
        } else {
            UnsignedByte firstByte = 0;
            UnsignedByte secondByte = 0;
            for (UnsignedByte idx = 0; idx < 31; idx += 1) {
                for (UnsignedByte jdx = 0; jdx < 188; jdx += 1) {
                    Unsigned2Bytes code = map1[idx][jdx];
                    if (point == code) {
                        firstByte = idx + 0x81;
                        secondByte = jdx + 0x40;
                        if (secondByte >= 0x7F) {
                            secondByte += 1;
                        }
                    }
                }
            }
            if (firstByte == 0) {
                for (UnsignedByte idx = 0; idx < 29; idx += 1) {
                    for (UnsignedByte jdx = 0; jdx < 188; jdx += 1) {
                        Unsigned2Bytes code = map2[idx][jdx];
                        if (point == code) {
                            firstByte = idx + 0xE0;
                            secondByte = jdx + 0x40;
                            if (secondByte >= 0x7F) {
                                secondByte += 1;
                            }
                        }
                    }
                }
            }
            if (firstByte == 0) {
                throw QR_EXCEPTION("Given data contains invalid character.");
            }
            // Double bytes character
            charSize = 2;
            *bufferPtr = firstByte;
            *(bufferPtr + 1) = secondByte;
        }
        charsMap_[index] = charSize;
        if (maxBytesPerChar_ < charSize) {
            maxBytesPerChar_ = charSize;
        }
        if (minBytesPerChar_ > charSize) {
            minBytesPerChar_ = charSize;
        }
        byteCount_ += charSize;
        bufferPtr += 2;
    }

    rawString_ = Common::allocate(byteCount_);
    UnsignedByte *rawPtr = rawString_;
    bufferPtr = buffer;

    for (unsigned int index = 0; index < charCount_; index += 1) {
        UnsignedByte charSize = charsMap_[index];
        *rawPtr = *bufferPtr;
        if (charSize == 2) {
            *(rawPtr + 1) = *(bufferPtr + 1);
        }
        rawPtr += charSize;
        bufferPtr += 2;
    }

    delete[] buffer;
    isValid_ = true;
}

unsigned int ShiftJisString::getCharacterByte(unsigned int index, UnsignedByte* charSize) {
    if (index >= charCount_) {
        throw QR_EXCEPTION("Position must be in range of string charactersCount");
    }
    if (!isValid_) {
        throw QR_EXCEPTION("This object is invalid so can not perform the requested action.");
    }
    unsigned int posBytes = 0;
    for (unsigned int idx = 0; idx < index; idx += 1) {
        UnsignedByte curCharBytes = charsMap_[idx];
        posBytes += curCharBytes;
    }
    *charSize = charsMap_[index];
    return posBytes;
}

UnicodePoint ShiftJisString::unicodes() {
    if (!isValid_) {
        throw QR_EXCEPTION("This object is invalid so can not perform the requested action.");
    }
    UnicodePoint result = UnicodePoint(charCount_);
    UnsignedByte* charPtr = rawString_;
    Unsigned2Bytes (*map1)[188] = ShiftJisString_KanjiUnicode1Map();
    Unsigned2Bytes (*map2)[188] = ShiftJisString_KanjiUnicode2Map();

    for (unsigned int index = 0; index < charCount_; index += 1) {
        UnsignedByte charSize = charsMap_[index];
        UnsignedByte curByte = *charPtr;
        Unsigned4Bytes point = '?';
        if (charSize == 1) {
            if (curByte == 0x5C) { // '\'
                point = 0xA5; // '¥'
            } else if (curByte == 0x7E) { // '~'
                point = 0x203E; // '‾'
            } else if (curByte < 0x7E) {
                point = curByte;
            } else if (curByte >= 0xA1 && curByte <= 0xDF) {
                point = curByte - 0xA1 + 0xFF61;
            }
        } else {
            UnsignedByte nextByte = *(charPtr + 1);
            UnsignedByte secondIndex = nextByte - 0x40;
            if (nextByte > 0x7F) {
                secondIndex -= 1;
            }
            if (curByte >= 0xE0) {
                UnsignedByte firstIndex = curByte - 0xE0;
                Unsigned2Bytes value = map2[firstIndex][secondIndex];
                if (value > 0) {
                    point = (Unsigned4Bytes)value;
                }
            } else if (curByte >= 0x81) {
                UnsignedByte firstIndex = curByte - 0x81;
                Unsigned2Bytes value = map1[firstIndex][secondIndex];
                if (value > 0) {
                    point = (Unsigned4Bytes)value;
                }
            }
        }
        result.setCharacter(point, index);
        charPtr += charSize;
    }
    return result;
}
