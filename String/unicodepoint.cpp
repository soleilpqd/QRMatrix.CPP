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

#include "unicodepoint.h"
#include "utf8string.h"
#include "shiftjisstring.h"
#include "../QRMatrix/Exception/qrmatrixexception.h"
#include <cstring>

using namespace QRMatrix;

UnicodePoint::~UnicodePoint() {
    delete[] rawString_;
}

UnicodePoint::UnicodePoint(UnicodePoint &other) {
    charCount_ = other.charactersCount();
    rawString_ = new Unsigned4Bytes [charCount_];
    for (int index = 0; index < charCount_; index += 1) {
        rawString_[index] = other.rawString_[index];
    }
}

UnicodePoint::UnicodePoint(const unsigned int charCount) {
    charCount_ = charCount;
    rawString_ = new Unsigned4Bytes [charCount];
    for (int index = 0; index < charCount; index += 1) {
        rawString_[index] = 0;
    }
}

UnicodePoint::UnicodePoint(const Unsigned4Bytes* codes, const unsigned int charCount) {
    charCount_ = charCount;
    rawString_ = new Unsigned4Bytes [charCount];
    for (int index = 0; index < charCount; index += 1) {
        rawString_[index] = codes[index];
    }
}

UnicodePoint::UnicodePoint(const UnsignedByte* codes, const unsigned int charCount) {
    charCount_ = charCount;
    rawString_ = new Unsigned4Bytes [charCount];
    for (int index = 0; index < charCount; index += 1) {
        rawString_[index] = (Unsigned4Bytes)codes[index];
    }
}

void UnicodePoint::setCharacter(Unsigned4Bytes character, unsigned int index) {
    if (index >= charCount_) {
        throw QR_EXCEPTION("Index out of string length");
    }
    rawString_[index] = character;
}

void UnicodePoint::setCharacter(UnsignedByte character, unsigned int index) {
    Unsigned4Bytes code = (Unsigned4Bytes)character;
    UnicodePoint::setCharacter(code, index);
}

bool UnicodePoint::isEqual(UnicodePoint other) {
    if (charCount_ == other.charactersCount()) {
        for (int index = 0; index < charCount_; index += 1) {
            if (rawString_[index] != other.rawString_[index]) {
                return false;
            }
        }
        return true;
    }
    return false;
}

UnicodePoint UnicodePoint::substring(unsigned int startIndex, unsigned int length) {
    UnicodePoint result = UnicodePoint(length);
    for (unsigned int index = 0; index < length; index += 1) {
        result.rawString_[index] = rawString_[index + startIndex];
    }
    return result;
}

bool UnicodePoint_testKanji(Unsigned4Bytes point) {
    UnicodePoint unicodes(&point, 1);
    try {
        ShiftJisString shiftjis(unicodes);
        return shiftjis.minBytesPerChar() > 1 && shiftjis.maxBytesPerChar() > 1;
    } catch (QRMatrixException exception) {
        return false;
    }
}

bool UnicodePoint_testByte(Unsigned4Bytes point) {
    return point <= 0xFF;
}

bool UnicodePoint_testNumeric(Unsigned4Bytes point) {
    return point >= '0' && point <= '9';
}

bool UnicodePoint_testAlphaNumeric(Unsigned4Bytes point) {
    if (UnicodePoint_testByte(point)) {
        static const char *table = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ $%*+-./:";
        char character = (char)point;
        const char* targetPtr = std::strchr(table, character);
        return targetPtr != NULL;
    }
    return false;
}

EncodingMode UnicodePoint_testMode(
    Unsigned4Bytes* points,
    unsigned int length,
    unsigned int curIndex,
    bool isMicro,
    unsigned int* outLength
) {
    *outLength = 1;
    if (UnicodePoint_testKanji(points[curIndex])) {
        unsigned int limit = isMicro ? 5 : 7;
        unsigned int kanjiCount = 1;
        for (unsigned int index = curIndex + 1; index < length; index += 1) {
            if (UnicodePoint_testKanji(points[index])) {
                kanjiCount += 1;
            } else {
                break;
            }
        }
        if (kanjiCount >= limit) {
            *outLength = kanjiCount;
            return EncodingMode::kanji;
        }
        return EncodingMode::byte;
    }
    if (UnicodePoint_testNumeric(points[curIndex])) {
        unsigned int limit = isMicro ? 4 : 6;
        unsigned int numericCount = 1;
        for (unsigned int index = curIndex + 1; index < length; index += 1) {
            if (UnicodePoint_testNumeric(points[index])) {
                numericCount += 1;
            } else {
                break;
            }
        }
        if (numericCount >= limit) {
            *outLength = numericCount;
            return EncodingMode::numeric;
        }
    }
    if (UnicodePoint_testAlphaNumeric(points[curIndex])) {
        unsigned int limit = isMicro ? 6 : 8;
        unsigned int alphaNumCount = 1;
        for (unsigned int index = curIndex + 1; index < length; index += 1) {
            if (UnicodePoint_testAlphaNumeric(points[index])) {
                alphaNumCount += 1;
            } else {
                break;
            }
        }
        if (alphaNumCount >= limit) {
            *outLength = alphaNumCount;
            return EncodingMode::alphaNumeric;
        }
    }
    return EncodingMode::byte;
}

QRMatrixSegment* UnicodePoint::segments(ErrorCorrectionLevel level, unsigned int* length, bool isMicro) {
    if (charactersCount() == 0) {
        return NULL;
    }
    if (isMicro && level == ErrorCorrectionLevel::high) {
        throw QR_EXCEPTION("Invalid Error Correction Level for MicroQR");
    }

    EncodingMode* segmentModes = new EncodingMode [charCount_];
    unsigned int* segmentLengths = new unsigned int [charCount_];
    unsigned int segmentIndex = 0;
    unsigned int byteModeCount = 0;

    unsigned int segmentLen = 0;
    EncodingMode mode = UnicodePoint_testMode(rawString_, charCount_, 0, isMicro, &segmentLen);
    if (mode != EncodingMode::byte) {
        segmentModes[segmentIndex] = mode;
        segmentLengths[segmentIndex] = segmentLen;
        segmentIndex += 1;
    } else {
        byteModeCount = 1;
    }
    for (unsigned int index = segmentLen; index < charCount_; index += 1) {
        mode = UnicodePoint_testMode(rawString_, charCount_, index, isMicro, &segmentLen);
        if (mode != EncodingMode::byte) {
            if (byteModeCount > 0) {
                segmentModes[segmentIndex] = EncodingMode::byte;
                segmentLengths[segmentIndex] = byteModeCount;
                segmentIndex += 1;
                byteModeCount = 0;
            }
            segmentModes[segmentIndex] = mode;
            segmentLengths[segmentIndex] = segmentLen;
            segmentIndex += 1;
            index = index + segmentLen - 1;
        } else {
            byteModeCount += 1;
        }
    }
    if (byteModeCount > 0) {
        segmentModes[segmentIndex] = EncodingMode::byte;
        segmentLengths[segmentIndex] = byteModeCount;
        segmentIndex += 1;
    }

    QRMatrixSegment* result = new QRMatrixSegment [segmentIndex];
    unsigned int offset = 0;
    for (unsigned int index = 0; index < segmentIndex; index += 1) {
        EncodingMode segMode = segmentModes[index];
        unsigned int segLen = segmentLengths[index];
        UnicodePoint segCodes = substring(offset, segLen);
        switch (segMode) {
        case EncodingMode::numeric:
        case EncodingMode::alphaNumeric:
        case EncodingMode::byte: {
            Utf8String utf8(segCodes);
            result[index].fill(segMode, utf8.utf8Data(), utf8.bytesCount());
        }
            break;
        case EncodingMode::kanji: {
            ShiftJisString shiftjis(segCodes);
            result[index].fill(segMode, shiftjis.bytes(), shiftjis.charactersCount() * 2);
        }
            break;
        }
        offset += segLen;
    }
    *length = segmentIndex;
    delete[] segmentLengths;
    delete[] segmentModes;
    return result;
}
