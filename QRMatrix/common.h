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

#ifndef COMMON_H
#define COMMON_H

#include "constants.h"
#include <iostream>

namespace QRMatrix {

    struct ErrorCorrectionInfo {
        UnsignedByte version;
        ErrorCorrectionLevel level;
        Unsigned2Bytes codewords;
        Unsigned2Bytes ecCodewordsPerBlock;
        Unsigned2Bytes group1Blocks;
        Unsigned2Bytes group1BlockCodewords;
        Unsigned2Bytes group2Blocks;
        Unsigned2Bytes group2BlockCodewords;

        inline ErrorCorrectionInfo() {
            version = 0;
            level = ErrorCorrectionLevel::low;
            codewords = 0;
            ecCodewordsPerBlock = 0;
            group1Blocks = 0;
            group1BlockCodewords = 0;
            group2Blocks = 0;
            group2BlockCodewords = 0;
        }
        inline ErrorCorrectionInfo(UnsignedByte ver, ErrorCorrectionLevel lev, const Unsigned2Bytes data[6]) {
            version = ver;
            level = lev;
            codewords = data[0];
            ecCodewordsPerBlock = data[1];
            group1Blocks = data[2];
            group1BlockCodewords = data[3];
            group2Blocks = data[4];
            group2BlockCodewords = data[5];
        }

        /// Number of bytes for QR data.
        /// @ref: https://www.thonky.com/qr-code-tutorial/error-correction-table.
        static ErrorCorrectionInfo errorCorrectionInfo(
            UnsignedByte version,
            ErrorCorrectionLevel level
        );
        static ErrorCorrectionInfo microErrorCorrectionInfo(
            UnsignedByte version,
            ErrorCorrectionLevel level
        );

        inline UnsignedByte groupCount() { return (group2Blocks == 0 ? 1 : 2); }
        inline UnsignedByte ecBlockTotalCount() { return group1Blocks + group2Blocks; }
        inline unsigned int ecCodewordsTotalCount() { return ecBlockTotalCount() * ecCodewordsPerBlock; }
    };

    /// Common functions
    class Common {
    public:
        /// Current environment
        static bool isLittleEndian;

        /// Get QR dimension by its version (version in 1...40 ~ dimension 21...177).
        static inline UnsignedByte dimensionByVersion(UnsignedByte version) { return (version - 1) * versionsOffset + minimumDimension; }
        /// Get MicroQR dimension by its version (version in 1...4 ~ dimension 11...17).
        static inline UnsignedByte microDimensionByVersion(UnsignedByte version) { return (version - 1) * versionsMicroOffset + minimumMicroDimension; }

        /// Number of bits for character counts indicator.
        static unsigned int charactersCountIndicatorLength(
            UnsignedByte version,
            EncodingMode mode
        );
        /// Number of bits of character counts indicator for MicroQR.
        static unsigned int microCharactersCountIndicatorLength(
            UnsignedByte version,
            EncodingMode mode
        );
        /// Number of bits of mode indicator for MicroQR.
        static unsigned int microModeIndicatorLength(
            UnsignedByte version,
            EncodingMode mode
        );
        /// Number of bits of terminator for MicroQR.
        static unsigned int microTerminatorLength(
            UnsignedByte version
        );
        /// Map Encoding mode value to MicroQR
        static UnsignedByte microQREncodingModeValue(
            EncodingMode mode
        );
        /// Map ErrorCorrectionLevel value to MicroQR
        static UnsignedByte microQRErrorCorrectionLevelValue(
            ErrorCorrectionLevel level,
            UnsignedByte version
        );

        /// Allocate `count` of bytes (and reset to 0)
        static UnsignedByte* allocate(unsigned int count);

        /// Copy `count` bits of source BYTE (from bit `sourceStartIndex`) into `destination` BYTE stating from bit at `destStartIndex`.
        /// Bits index is from left to right.
        /// Bit index must be 0...7 (because this copies 1 byte only).
        /// `count` must be 1...8 (bits - size of `source`), also `count` must be < 8 - destStartIndex && 8 - sourceStartIndex.
        static void copyBits(
            /// Source Byte.
            UnsignedByte sourceByte,
            /// Source starting bit.
            unsigned int sourceStartIndex,
            /// Destination memory allocation.
            UnsignedByte* destination,
            /// Destination starting bit.
            unsigned int destStartIndex,
            /// Number of bits to be copied.
            unsigned int count
        );

        /// Copy `count` bits of source (from bit 0th) into `destination` stating from bit at `startIndex`.
        static void copyBits(
            /// Source.
            UnsignedByte* source,
            /// Source length in byte.
            unsigned int sourceLength,
            /// Source starting bit index
            unsigned int sourceStartIndex,
            /// Source order (little/big endian).
            bool isSourceOrderReversed,
            /// Destination (bytes array - not care about order here).
            UnsignedByte* destination,
            /// Destintation starting bit index.
            unsigned int destStartIndex,
            /// Number of bits to be copied.
            unsigned int count
        );

        /// Return array of 6 items contains QR aligment locations for version 2...40
        static const UnsignedByte* alignmentLocations(UnsignedByte version);

    };

}

#endif // COMMON_H
