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

#ifndef UTF16STRING_H
#define UTF16STRING_H

#include "../QRMatrix/constants.h"
#include "unicodepoint.h"

namespace QRMatrix {

    class Utf16String {
    public:
        static inline Unsigned2Bytes highSurrogateLowerBound()   { return 0xD800; }
        static inline Unsigned2Bytes highSurrogateHigherBound()  { return 0xDBFF; }
        static inline Unsigned2Bytes lowSurrogateLowerBound()    { return 0xDC00; }
        static inline Unsigned2Bytes lowSurrogateHigherBound()   { return 0xDFFF; }

        static inline Unsigned2Bytes singleWord1LowerBound()    { return 0x0000; }
        static inline Unsigned2Bytes singleWord1HigherBound()   { return 0xD7FF; }
        static inline Unsigned2Bytes singleWord2LowerBound()    { return 0xE000; }
        static inline Unsigned2Bytes singleWord2HigherBound()   { return 0xFFFF; }

        ~Utf16String();
        Utf16String(Utf16String &other);
        /// Init from UTF-16 words (2 bytes) data
        Utf16String(
            /// Data
            const Unsigned2Bytes* raw,
            /// Number of words of `raw`
            const unsigned int length,
            /// Bytes order
            const bool isLittleEndian
        );

        /// Is valid data
        inline bool isValid() { return isValid_; }

        /// Unicode characters count
        inline unsigned int charactersCount() { return charCount_; }

        /// Get Unicode characters code points (decoded data).
        UnicodePoint unicodes();

        /// Bytes order
        inline bool isLittleEndian() { return isLE_; }

    private:
        bool isLE_;
        bool isValid_;
        Unsigned2Bytes* rawString_;
        unsigned int charCount_;
        unsigned int byteCount_;
        UnsignedByte* charsMap_;
    };

}

#endif // UTF16STRING_H
