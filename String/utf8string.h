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

#ifndef UTF8STRING_H
#define UTF8STRING_H

#include "../QRMatrix/constants.h"
#include "unicodepoint.h"

#define UTF8_SINGLE_BYTE_MASK       0b10000000
#define UTF8_SINGLE_BYTE_PREFIX     0b00000000
#define UTF8_DOUBLE_BYTES_MASK      0b11100000
#define UTF8_DOUBLE_BYTES_PREFIX    0b11000000
#define UTF8_TRIPLE_BYTES_MASK      0b11110000
#define UTF8_TRIPLE_BYTES_PREFIX    0b11100000
#define UTF8_QUADRUPLE_BYTES_MASK   0b11111000
#define UTF8_QUADRUPLE_BYTES_PREFIX 0b11110000

#define UTF8_SECONDARY_BYTE_MASK    0b11000000
#define UTF8_SECONDARY_BYTE_PREFIX  0b10000000

#define UTF8_SINGLE_BYTE_LAST_CODE_POINT        0x0000007F
#define UTF8_DOUBLE_BYTES_LAST_CODE_POINT       0x000007FF
#define UTF8_TRIPLE_BYTES_LAST_CODE_POINT       0x0000FFFF
#define UTF8_QUADRUPLE_BYTES_LAST_CODE_POINT    0x0010FFFF

namespace QRMatrix {

    /// Handle unicode (UTF-8) from std::string
    class Utf8String{
    public:
        ~Utf8String();
        Utf8String(Utf8String &other);
        /// Init from bytes of UTF-8 encoded string.
        Utf8String(
            /// Bytes.
            const UnsignedByte* raw,
            /// Number of bytes (leave 0 if `raw` is C-String (null-terminated).
            const unsigned int length = 0
        );
        /// Init from Unicode codes (Encode given Unicode characters into UTF-8)
        Utf8String(
            /// Array of codes
            const UnicodePoint unicodes
        );

        /// Is valid data
        inline bool isValid() { return isValid_; }

        /// Unicode characters count
        inline unsigned int charactersCount() { return charCount_; }

        /// Bytes count
        inline unsigned int bytesCount() { return byteCount_; }

        inline unsigned int maxBytesPerChar() { return maxBytesPerChar_; }

        /// Substring by Unicode characters. Throw error on invalid arguments.
        Utf8String substring(const unsigned int position, const unsigned int count);

        /// UTF-8 bytes.
        /// NULL terminated.
        inline const UnsignedByte* utf8Data() { return rawString_; }

        /// Get Unicode characters code points (decoded data).
        UnicodePoint unicodes();

    private:
        bool isValid_;
        UnsignedByte* rawString_;
        unsigned int charCount_;
        unsigned int byteCount_;
        unsigned int maxBytesPerChar_;
        UnsignedByte* charsMap_;
    };

}

#endif // UTF8STRING_H
