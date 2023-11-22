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

#ifndef LATINSTRING_H
#define LATINSTRING_H

#include "../QRMatrix/constants.h"
#include "unicodepoint.h"

namespace QRMatrix {

    /// ISO/IEC 8859-1; equivalent of Unicode Latin-1 block
    class LatinString {
    public:
        ~LatinString();
        LatinString(LatinString &other);
        /// Init from ISO 8859-1 bytes
        LatinString(
            /// ISO/IEC 8859-1 Bytes.
            const UnsignedByte* raw,
            /// Number of bytes
            const unsigned int length
        );
        /// Init from Unicode code points
        LatinString(const UnicodePoint unicodes);

        /// Is valid data
        inline bool isValid() { return isValid_; }

        /// Unicode characters count
        inline unsigned int charactersCount() { return charCount_; }

        /// Character raw bytes.
        /// Note: not NULL ternminated.
        inline const UnsignedByte* bytes() { return rawString_; }

        /// Get Unicode characters code points (decoded data).
        UnicodePoint unicodes();

    private:
        bool isValid_;
        UnsignedByte* rawString_;
        unsigned int charCount_;
    };

}

#endif // LATINSTRING_H
