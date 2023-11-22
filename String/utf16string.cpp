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

#include "utf16string.h"
#include "../QRMatrix/Exception/qrmatrixexception.h"
#include "../QRMatrix/common.h"

using namespace QRMatrix;

QRMatrix::Utf16String::~Utf16String() {
    delete[] rawString_;
    delete[] charsMap_;
}

QRMatrix::Utf16String::Utf16String(Utf16String &other) {
    isValid_ = other.isValid_;
    charCount_ = other.charCount_;
    byteCount_ = other.byteCount_;
    isLE_ = other.isLE_;
    charsMap_ = Common::allocate(charCount_);
    for (unsigned int index = 0; index < charCount_; index += 1) {
        charsMap_[index] = other.charsMap_[index];
    }
    rawString_ = new Unsigned2Bytes [byteCount_];
    for (unsigned int index = 0; index < byteCount_; index += 1) {
        rawString_[index] = other.rawString_[index];
    }
}

Unsigned2Bytes Utf16String_correctWord(Unsigned2Bytes input, bool isLE) {
    if (Common::isLittleEndian == isLE) {
        return input;
    }
    Unsigned2Bytes result = 0;
    UnsignedByte* resPtr = (UnsignedByte*)&result;
    UnsignedByte* inputPtr = (UnsignedByte*)&input;
    resPtr[0] = inputPtr[1];
    resPtr[1] = inputPtr[0];
    return result;
}

QRMatrix::Utf16String::Utf16String(const Unsigned2Bytes* raw, const unsigned int length, const bool isLittleEndian) {
    Unsigned2Bytes highSurrogateLower   = highSurrogateLowerBound();
    Unsigned2Bytes highSurrogateHigher  = highSurrogateHigherBound();
    Unsigned2Bytes lowSurrogateLower    = lowSurrogateLowerBound();
    Unsigned2Bytes lowSurrogateHigher   = lowSurrogateHigherBound();
    Unsigned2Bytes singleWordLower1     = singleWord1LowerBound();
    Unsigned2Bytes singleWordHigher1    = singleWord1HigherBound();
    Unsigned2Bytes singleWordLower2     = singleWord2LowerBound();
    Unsigned2Bytes singleWordHigher2    = singleWord2HigherBound();

    byteCount_ = length;
    isValid_ = false;
    charCount_ = 0;
    rawString_ = new Unsigned2Bytes [length];
    charsMap_ = Common::allocate(length);
    isLE_ = isLittleEndian;

    int wordToCheck = 0;
    for (int index = 0; index < length; index += 1) {
        Unsigned2Bytes currentWord = raw[index];
        rawString_[index] = currentWord;
        Unsigned2Bytes value = Utf16String_correctWord(currentWord, isLittleEndian);
        if (index == wordToCheck) {
            unsigned char charSize = 0;
            if ((value >= singleWordLower1 && value <= singleWordHigher1) || ( value >= singleWordLower2 && value <= singleWordHigher2)) {
                charSize = 1;
            } else if (value >= highSurrogateLower && value <= highSurrogateHigher) {
                charSize = 2;
            }
            if (charSize == 0) {
                throw QR_EXCEPTION("Input bytes seem be not UTF-16.");
            }
            wordToCheck += charSize;
            charsMap_[charCount_] = charSize;
            charCount_ += 1;
        } else if (value < lowSurrogateLower || value > lowSurrogateHigher) {
            throw QR_EXCEPTION("Input bytes seem be not UTF-16.");
        }
    }
    isValid_ = true;
}

UnicodePoint QRMatrix::Utf16String::unicodes() {
    if (!isValid_) {
        throw QR_EXCEPTION("This object is invalid so can not perform the requested action.");
    }

    Unsigned2Bytes highSurrogateLower = highSurrogateLowerBound();
    Unsigned2Bytes lowSurrogateLower = lowSurrogateLowerBound();

    unsigned int charCount = charactersCount();
    UnicodePoint result = UnicodePoint(charCount);
    unsigned int posWord = 0;

    for (int index = 0; index < charCount; index += 1) {
        UnsignedByte charSize = charsMap_[index];
        if (charSize == 1) {
            Unsigned4Bytes value = Utf16String_correctWord(rawString_[posWord], isLE_);
            result.setCharacter(value, index);
        } else if (charSize == 2) {
            Unsigned4Bytes highWord = Utf16String_correctWord(rawString_[posWord], isLE_);
            Unsigned4Bytes lowWord = Utf16String_correctWord(rawString_[posWord + 1], isLE_);
            highWord = (highWord - highSurrogateLower) * 0x0400;
            lowWord -= lowSurrogateLower;
            Unsigned4Bytes value = (Unsigned4Bytes)highWord + (Unsigned4Bytes)lowWord + 0x10000;
            result.setCharacter(value, index);
        }
        posWord += charSize;
    }
    return result;
}
