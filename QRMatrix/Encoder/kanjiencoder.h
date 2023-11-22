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

#ifndef KANJIENCODER_H
#define KANJIENCODER_H

#include "../constants.h"

namespace QRMatrix {

    class KanjiEncoder {
    public:
        /// Encode text.
        /// @return Number of written bits.
        static unsigned int encode(
            /// Text to be encoded
            const UnsignedByte* text,
            /// Number of bytes of text
            unsigned int length,
            /// Buffer to write result into
            UnsignedByte* buffer,
            /// The position of buffer bit to start writting
            unsigned int startIndex
        );
    };

}

#endif // KANJIENCODER_H
