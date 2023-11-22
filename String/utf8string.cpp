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

#include "utf8string.h"
#include "../QRMatrix/Exception/qrmatrixexception.h"
#include <cstring>
#include "../QRMatrix/common.h"

using namespace QRMatrix;

Utf8String::~Utf8String() {
    delete[] rawString_;
    delete[] charsMap_;
}

Utf8String::Utf8String(Utf8String &other) {
    isValid_ = other.isValid();
    charCount_ = other.charCount_;
    byteCount_ = other.byteCount_;
    maxBytesPerChar_ = other.maxBytesPerChar_;
    charsMap_ = Common::allocate(charCount_);
    rawString_ = Common::allocate(byteCount_ + 1);
    for (unsigned int index = 0; index < charCount_; index += 1) {
        charsMap_[index] = other.charsMap_[index];
    }
    for (unsigned int index = 0; index < byteCount_; index += 1) {
        rawString_[index] = other.rawString_[index];
    }
}

Utf8String::Utf8String(const UnsignedByte* raw, const unsigned int length) {
    UnsignedByte singleMask = singleByteMask();
    UnsignedByte singlePrefix = singleBytePrefix();
    UnsignedByte doubleMask = doubleBytesMask();
    UnsignedByte doublePrefix = doubleBytesPrefix();
    UnsignedByte tripleMask = tripleBytesMask();
    UnsignedByte triplePrefix = tripleBytesPrefix();
    UnsignedByte quadrupleMask = quadrupleBytesMask();
    UnsignedByte quadruplePrefix = quadrupleBytesPrefix();
    UnsignedByte secondaryMask = secondaryByteMask();
    UnsignedByte secondaryPrefix = secondaryBytePrefix();

    isValid_ = false;
    charCount_ = 0;
    maxBytesPerChar_ = 0;
    unsigned int bytesCount = length;
    if (bytesCount == 0) {
        bytesCount = (unsigned int)std::strlen((char*)raw);
    }
    rawString_ = Common::allocate(bytesCount + 1); // +1 for null terminator
    byteCount_ = bytesCount;
    charsMap_ = Common::allocate(bytesCount);

    int byteToCheck = 0;
    for (int index = 0; index < bytesCount; index += 1) {
        UnsignedByte currentByte = raw[index];
        rawString_[index] = currentByte;
        if (index == byteToCheck) {
            UnsignedByte charSize = 0;
            if ((currentByte & singleMask) == singlePrefix) {
                charSize = 1;
            } else if ((currentByte & doubleMask) == doublePrefix) {
                charSize = 2;
            } else if ((currentByte & tripleMask) == triplePrefix) {
                charSize = 3;
            } else if ((currentByte & quadrupleMask) == quadruplePrefix) {
                charSize = 4;
            } else {
                throw QR_EXCEPTION("Input bytes seem be not UTF-8.");
            }
            byteToCheck += charSize;
            if (maxBytesPerChar_ < charSize) {
                maxBytesPerChar_ = charSize;
            }
            charsMap_[charCount_] = charSize;
            charCount_ += 1;
        } else if ((currentByte & secondaryMask) != secondaryPrefix) {
            throw QR_EXCEPTION("Input bytes seem be not UTF-8.");
        }
    }
    isValid_ = true;
}

unsigned char* Utf8String_encode(UnsignedByte* codePtr, UnsignedByte* destPtr, const unsigned char charSize) {
    unsigned char prefix = 0;
    switch (charSize) {
    case 2:
        prefix = Utf8String::doubleBytesPrefix();
        break;
    case 3:
        prefix = Utf8String::tripleBytesPrefix();
        break;
    case 4:
        prefix = Utf8String::quadrupleBytesPrefix();
        break;
    default:
        break;
    }
    UnsignedByte secondaryPrefix = Utf8String::secondaryBytePrefix();
    unsigned int prefixBitLen = charSize + 1;
    const unsigned int sourceSize = 4 * 8;
    // Fist byte
    unsigned int firstByteDataLen = 8 - prefixBitLen;
    unsigned int sourceIndex = sourceSize - firstByteDataLen - ((unsigned int)charSize - 1) * 6;
    *destPtr = prefix;
    Common::copyBits(codePtr, 4, sourceIndex, Common::isLittleEndian, destPtr, prefixBitLen, firstByteDataLen);
    destPtr += 1;
    sourceIndex += firstByteDataLen;
    // Next bytes
    for (int index = 0; index < charSize - 1; index += 1) {
        *destPtr = secondaryPrefix;
        Common::copyBits(codePtr, 4, sourceIndex, Common::isLittleEndian, destPtr, 2, 6);
        destPtr += 1;
        sourceIndex += 6;
    }
    return destPtr;
}

Utf8String::Utf8String(UnicodePoint unicodes) {
    Unsigned4Bytes singleByteLastCode = singleByteLastCodePoint();
    Unsigned4Bytes doubleBytesLastCode = doubleBytesLastCodePoint();
    Unsigned4Bytes tripleBytesLastCode = tripleBytesLastCodePoint();
    const Unsigned4Bytes* codes = unicodes.unicodePoints();

    unsigned int length = unicodes.charactersCount();
    charCount_ = 0;
    maxBytesPerChar_ = 0;
    charsMap_ = Common::allocate(length);
    unsigned int bytesCount = 0;
    for (int index = 0; index < length; index += 1) {
        unsigned char charSize = 0;
        Unsigned4Bytes code = codes[index];
        if (code <= singleByteLastCode) {
            charSize = 1;
        } else if (code <= doubleBytesLastCode) {
            charSize = 2;
        } else if (code <= tripleBytesLastCode) {
            charSize = 3;
        } else {
            charSize = 4;
        }
        if (maxBytesPerChar_ < charSize) {
            maxBytesPerChar_ = charSize;
        }
        charsMap_[charCount_] = charSize;
        bytesCount += charSize;
        charCount_ += 1;
    }

    rawString_ = Common::allocate(bytesCount + 1); // +1 for null terminator
    byteCount_ = bytesCount;
    UnsignedByte* ptr = rawString_;
    for (int index = 0; index < length; index += 1) {
       UnsignedByte charSize = charsMap_[index];
        Unsigned4Bytes code = codes[index];
        UnsignedByte* codePtr = (UnsignedByte*)&code;
        if (charSize == 1) {
            *ptr = (UnsignedByte)code;
            ptr += 1;
        } else {
            ptr = Utf8String_encode(codePtr, ptr, charSize);
        }
    }

    isValid_ = true;
}

Utf8String Utf8String::substring(const unsigned int position, const unsigned int count) {
    if (position >= charCount_ || (position + count) > charCount_) {
        throw QR_EXCEPTION("Position & Position + count must be in range of string charactersCount");
    }
    if (!isValid_) {
        throw QR_EXCEPTION("This object is invalid so can not perform the requested action.");
    }
    unsigned int posBytes = 0;
    unsigned int countBytes = 0;
    unsigned int limit = position + count;
    for (unsigned int index = 0; index < limit; index += 1) {
        UnsignedByte curCharBytes = charsMap_[index];
        if (index < position) {
            posBytes += (unsigned int)curCharBytes;
        } else {
            countBytes += (unsigned int)curCharBytes;
        }
    }
    return Utf8String(rawString_ + posBytes, countBytes);
}

Unsigned4Bytes Utf8String_decode(UnsignedByte* source, UnsignedByte charSize) {
    unsigned int u4BSize = (unsigned int)sizeof(Unsigned4Bytes);
    Unsigned4Bytes result = 0;
    UnsignedByte* resultPtr = (UnsignedByte*)&result;
    unsigned int resultBitIndex = u4BSize * 8;
    if (charSize == 1) {
        result = (Unsigned4Bytes)*source;
    } else {
        UnsignedByte* lastByte = source + charSize - 1;
        for (int index = 0; index < charSize; index += 1) {
            UnsignedByte *curByte = lastByte - index;
            unsigned int prefixBitCount = 2;
            if (curByte == source) {
                prefixBitCount = charSize + 1;
            }
            unsigned int bitCount = 8 - prefixBitCount;
            resultBitIndex -= bitCount;
            Common::copyBits(curByte, 1, prefixBitCount, Common::isLittleEndian, resultPtr, resultBitIndex, bitCount);
        }
        if (Common::isLittleEndian) { // Reverse bytes
            Unsigned4Bytes tmp = result;
            UnsignedByte* tmpPtr = (UnsignedByte*)&tmp;
            for (int index = 0; index < u4BSize; index += 1) {
                resultPtr[index] = tmpPtr[u4BSize - index - 1];
            }
        }
    }
    return result;
}

UnicodePoint Utf8String::unicodes() {
    if (!isValid_) {
        throw QR_EXCEPTION("This object is invalid so can not perform the requested action.");
    }
    UnicodePoint result = UnicodePoint(charCount_);
    UnsignedByte* charPtr = rawString_;

    for (int index = 0; index < charCount_; index += 1) {
        UnsignedByte charSize = charsMap_[index];
        Unsigned4Bytes value = Utf8String_decode(charPtr, charSize);
        result.setCharacter(value, index);
        charPtr += charSize;
    }
    return result;
}
