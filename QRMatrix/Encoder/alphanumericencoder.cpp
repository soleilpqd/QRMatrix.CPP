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

#include "alphanumericencoder.h"
#include "../Exception/qrmatrixexception.h"
#include "../common.h"
#include <cstring>

using namespace QRMatrix;

unsigned int AlphaNumericEncoder::indexOfCharacter(UnsignedByte character) {
    static const char *table = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ $%*+-./:";
    const char* targetPtr = std::strchr(table, character);
    if (targetPtr != NULL) {
        return (unsigned int)(targetPtr - table);
    }
    return -1;
}

unsigned int AlphaNumericEncoder::encodedValueOfPair(const UnsignedByte* pair, unsigned int length) {
    if ((length < 1) || (length > 2)) {
        throw QR_EXCEPTION("Pair must have 1 or 2 characters.");
    }
    unsigned int value1 = AlphaNumericEncoder::indexOfCharacter(pair[0]);
    if (length == 1) {
        return value1;
    }
    unsigned int value2 = AlphaNumericEncoder::indexOfCharacter(pair[1]);
    return value1 * AlphaNumericEncoder::alphaNumericMultiplication() + value2;
}

unsigned int AlphaNumericEncoder::encode(const UnsignedByte* text, unsigned int length, UnsignedByte* buffer, unsigned int startIndex) {
    unsigned int index = 0;
    unsigned int bitIndex = startIndex;
    unsigned int charCount = 2;
    UnsignedByte pair[2];
    while (index < length) {
        pair[0] = text[index];
        unsigned int offset  = length - index;
        unsigned int encodedLen = 0;
        if (offset > 1) {
            charCount = 2;
            pair[1] = text[index + 1];
            encodedLen = AlphaNumericEncoder::pairCharactersBitsLength();
        } else {
            charCount = 1;
            encodedLen = AlphaNumericEncoder::singleCharacterBitsLenth();
        }
        index += charCount;
        unsigned int encodedData = AlphaNumericEncoder::encodedValueOfPair(pair, charCount);
        unsigned int encodedSize = sizeof(unsigned int);
        unsigned int encodedOffset = encodedSize * 8 - encodedLen;
        Common::copyBits(
            (unsigned char*)&encodedData,
            encodedSize,
            encodedOffset,
            Common::isLittleEndian,
            buffer,
            bitIndex,
            encodedLen
        );
        bitIndex += encodedLen;
    }
    return bitIndex - startIndex;
}
