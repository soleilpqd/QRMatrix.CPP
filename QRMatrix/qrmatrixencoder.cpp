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

#include "qrmatrixencoder.h"
#include "common.h"
#include <string>

#include "Encoder/numericencoder.h"
#include "Encoder/alphanumericencoder.h"
#include "Encoder/kanjiencoder.h"

#include "Polynomial/polynomial.h"

#if LOGABLE
#include "../DevTools/devtools.h"
#endif

using namespace QRMatrix;

// PREPARE DATA --------------------------------------------------------------------------------------------------------------------------------------

/// Calculate encoded data bits count,
/// include Mode Indicator and ECI header bits,
/// exclude Characters Count bits
unsigned int QRMatrixEncoder_calculateEncodedDataBitsCount(
    QRMatrixSegment* segments,
    unsigned int count
) {
    unsigned int totalDataBitsCount = 0;
    for (unsigned int index = 0; index < count; index += 1) {
        QRMatrixSegment segment = segments[index];
        if (segment.length() == 0) {
            continue;
        }
        // Mode indicator
        totalDataBitsCount += 4;
        // ECI
        if (segment.isEciHeaderRequired()) {
            totalDataBitsCount += 4; // ECI Header
            if (segment.eci() <= 128) {
                totalDataBitsCount += 8; // 1 byte ECI Indicator
            } else if (segment.eci() <= 16383) {
                totalDataBitsCount += 16; // 2 bytes ECI Indicator
            } else {
                totalDataBitsCount += 24; // 3 bytes ECI Indicator
            }
        }
        // Data
        switch (segment.mode()) {
        case EncodingMode::numeric: {
            // 3 characters encoded in 10 bits (each character is 1 byte)
            unsigned int numberOfGroups = (segment.length() / 3);
            totalDataBitsCount += numberOfGroups * NUM_TRIPLE_DIGITS_BITS_LEN;
            // Remaining chars
            UnsignedByte remainChars = segment.length() % 3;
            switch (remainChars) {
            case 1:
                totalDataBitsCount += NUM_SINGLE_DIGIT_BITS_LEN;
                break;
            case 2:
                totalDataBitsCount += NUM_DOUBLE_DIGITS_BITS_LEN;
            default:
                break;
            }
        }
        break;
        case EncodingMode::alphaNumeric: {
            // 2 characters encoded in 11 bits (each character is 1 byte)
            // Remaining character encoded in 6 bits.
            unsigned int numberOfGroups = segment.length() / 2;
            unsigned int remaining = segment.length() % 2;
            totalDataBitsCount += ALPHA_NUM_PAIR_CHARS_BITS_LEN * numberOfGroups +
                                  ALPHA_NUM_SINGLE_CHAR_BITS_LEN * remaining;
        }
        break;
        case EncodingMode::kanji:
            // 2 bytes per Kanji character.
            // Each character is encoded in 13 bits.
            totalDataBitsCount += (segment.length() / 2) * 13;
            break;
        case EncodingMode::byte:
            totalDataBitsCount += segment.length() * 8;
            break;
        }
    }
    return totalDataBitsCount;
}

/// Find QR Version & its properties
ErrorCorrectionInfo QRMatrixEncoder_findVersion(
    QRMatrixSegment* segments,
    unsigned int count,
    ErrorCorrectionLevel level,
    UnsignedByte minVersion,
    QRMatrixExtraMode extraMode,
    bool isStructuredAppend
) {
    // This is total estimated bits of data, excluding bits for Characters Count,
    // because each version requires difference number of Characters Count bits.
    unsigned int totalDataBitsCount = QRMatrixEncoder_calculateEncodedDataBitsCount(segments, count);
    if (isStructuredAppend) {
        totalDataBitsCount += 20; // 4 bits header, 4 bits position, 4 bits total number, 1 byte parity
    }
    if (extraMode.mode == EncodingExtraMode::fnc1First) {
        totalDataBitsCount += 4; // 4 bits FNC1 indicator
    } else  if (extraMode.mode == EncodingExtraMode::fnc1Second) {
        totalDataBitsCount += 12; // 4 bits FNC1 indicator, 8 bits Application Indicator
    }
    // Go for each version, get total bits and check
    bool isMicro = (extraMode.mode == EncodingExtraMode::microQr && !isStructuredAppend);
    UnsignedByte maxVer = isMicro ? MICROQR_MAX_VERSION : QR_MAX_VERSION;
    for (UnsignedByte version = (minVersion > 0 && minVersion <= maxVer) ? minVersion : 1; version <= maxVer; version += 1) {
        if (version > 1 && isMicro && level == ErrorCorrectionLevel::high) {
            throw QR_EXCEPTION("Error Correction Level High not available in MicroQR.");
        }
        ErrorCorrectionInfo info = isMicro ?
            ErrorCorrectionInfo::microErrorCorrectionInfo(version, level) :
            ErrorCorrectionInfo::errorCorrectionInfo(version, level);
        unsigned int capacity = info.codewords * 8;
        if (capacity == 0) {
            break;
        }
        if (totalDataBitsCount >= capacity) {
            // Not need to count total bits, just go to next version
            continue;
        }
        // Calculate total number of bits for Characters Count for this version
        unsigned int totalBits = totalDataBitsCount;
        bool hasAlpha = false;
        bool hasKanji = false;
        bool hasByte = false;
        for (unsigned int index = 0; index < count; index += 1) {
            QRMatrixSegment segment = segments[index];
            if (segment.length() == 0) {
                continue;
            }
            switch (segment.mode()) {
            case QRMatrix::alphaNumeric:
                hasAlpha = true;
                break;
            case QRMatrix::byte:
                hasByte = true;
                break;
            case QRMatrix::kanji:
                hasKanji = true;
                break;
            default:
                break;
            }
            totalBits += isMicro ?
                Common::microCharactersCountIndicatorLength(version, segment.mode()) :
                Common::charactersCountIndicatorLength(version, segment.mode());
        }
        // Check if this QR version bits capacity is enough for required total bits
        if (totalBits <= capacity) {
            UnsignedByte finalVer = version;
            if (isMicro) {
                if (finalVer < 2 && hasAlpha) {
                    // AlphaNumeric Mode is not available with M1
                    finalVer = 2;
                }
                if (finalVer < 3 && (hasByte || hasKanji)) {
                    // Byte/Kanji Mode is not available with <= M2
                    finalVer = 3;
                }
            }
            if (finalVer == version) {
                return info;
            }
            return isMicro ?
                ErrorCorrectionInfo::microErrorCorrectionInfo(finalVer, level) :
                ErrorCorrectionInfo::errorCorrectionInfo(finalVer, level);
        }
    }
    return ErrorCorrectionInfo();
}

// ENCODE DATA---------------------------------------------------------------------------------------------------------------------------------------

/// Encode ECI Indicator
UnsignedByte* QRMatrixEncoder_encodeEciIndicator(Unsigned4Bytes indicator, UnsignedByte* resultLength) {
    UnsignedByte* indicatorPtr = (UnsignedByte*)&indicator;
    UnsignedByte* result;
    if (indicator <= 127) {
        *resultLength = 1;
        result = new UnsignedByte [1];
        if (Common::isLittleEndian) {
            result[0] = indicatorPtr[0] & 0x7F;
        } else {
            result[0] = indicatorPtr[3] & 0x7F;
        }
    } else if (indicator <= 16383) {
        *resultLength = 2;
        result = new UnsignedByte [2];
        if (Common::isLittleEndian) {
            result[0] = (indicatorPtr[1] & 0x3F) | 0x80;
            result[1] = indicatorPtr[0];
        } else {
            result[0] = (indicatorPtr[2] & 0x3F) | 0x80;
            result[1] = indicatorPtr[3];
        }
    } else if (indicator <= 999999) {
        *resultLength = 3;
        result = new UnsignedByte [3];
        if (Common::isLittleEndian) {
            result[0] = (indicatorPtr[2] & 0x1F) | 0xC0;
            result[1] = indicatorPtr[1];
            result[2] = indicatorPtr[0];
        } else {
            result[0] = (indicatorPtr[1] & 0x1F) | 0xC0;
            result[1] = indicatorPtr[2];
            result[2] = indicatorPtr[3];
        }
    } else {
        throw QR_EXCEPTION("Invalid ECI Indicator");
    }
    return result;
}

/// Encode segments into buffer
void QRMatrixEncoder_encodeSegment(
    UnsignedByte* buffer,
    QRMatrixSegment segment,
    unsigned int segmentIndex,
    ErrorCorrectionLevel level,
    ErrorCorrectionInfo ecInfo,
    unsigned int* bitIndex,
    QRMatrixExtraMode extraMode
) {
    if (segment.length() == 0) {
        return;
    }
    bool isMicro = extraMode.mode == EncodingExtraMode::microQr;
    unsigned int uintSize = sizeof(unsigned int);
    // ECI Header if enable
    if (!isMicro && segment.isEciHeaderRequired()) {
        UnsignedByte eciLen = 0;
        UnsignedByte* eciHeader = QRMatrixEncoder_encodeEciIndicator(segment.eci(), &eciLen);
        // 4 bits of ECI mode indicator
        UnsignedByte eciModeHeader = 0b0111;
        Common::copyBits(&eciModeHeader, 1, 4, false, buffer, *bitIndex, 4);
        *bitIndex += 4;
        // ECI indicator
        Common::copyBits(
            eciHeader,
            eciLen, // size of eciUtf8Indicator
            0, // bit of eciUtf8Indicator to start copy
            false, // `eciHeader` is always normal order (big endian)
            buffer,
            *bitIndex,
            8 * eciLen // number of bits to be copied
            );
        *bitIndex += 8 * eciLen;
        delete[] eciHeader;
    }
    if (segmentIndex == 0) {
        if (extraMode.mode == EncodingExtraMode::fnc1First) {
            UnsignedByte fnc1Header = 0b0101;
            Common::copyBits(&fnc1Header, 1, 4, false, buffer, *bitIndex, 4);
            *bitIndex += 4;
        } else if (extraMode.mode == EncodingExtraMode::fnc1Second) {
            UnsignedByte fnc1Header = 0b1001;
            Common::copyBits(&fnc1Header, 1, 4, false, buffer, *bitIndex, 4);
            *bitIndex += 4;
            fnc1Header = 0;
            switch (extraMode.appIndicatorLength) {
            case 1:
                fnc1Header = extraMode.appIndicator[0] + 100;
                break;
            case 2: {
                std::string numStr = std::string((char*)extraMode.appIndicator, 2);
                fnc1Header = std::stoi(numStr);
            }
                break;
            default:
                break;
            }
            Common::copyBits(&fnc1Header, 1, 0, false, buffer, *bitIndex, 8);
            *bitIndex += 8;
        }
    }
    // Bits of segment mode indicator
    UnsignedByte numberOfModeBits = isMicro ? Common::microModeIndicatorLength(ecInfo.version, segment.mode()) : 4;
    if (numberOfModeBits > 0) {
        UnsignedByte mode = isMicro ? Common::microQREncodingModeValue(segment.mode()) : segment.mode();
        Common::copyBits(&mode, 1, 8 - numberOfModeBits, false, buffer, *bitIndex, numberOfModeBits);
        *bitIndex += numberOfModeBits;
    }
    // Character counts bits
    unsigned int charCountIndicatorLen = isMicro ?
        Common::microCharactersCountIndicatorLength(ecInfo.version, segment.mode()) :
        Common::charactersCountIndicatorLength(ecInfo.version, segment.mode());
    unsigned int charCount = 0;
    switch (segment.mode()) {
    case EncodingMode::numeric:
    case EncodingMode::alphaNumeric:
    case EncodingMode::byte:
        charCount = segment.length();
        break;
    case EncodingMode::kanji:
        charCount = segment.length() / 2;
        break;
    }
    Common::copyBits(
        (UnsignedByte*)&charCount,
        uintSize,
        uintSize * 8 - charCountIndicatorLen,
        Common::isLittleEndian,
        buffer,
        *bitIndex,
        charCountIndicatorLen
    );
    *bitIndex += charCountIndicatorLen;

#if LOGABLE
    LOG("SEGMENT:", "",
        isMicro ? "MicroQR Version: " : "QR Version: ", std::to_string(ecInfo.version).c_str(),
        "Mode: 0x", DevTools::getHex(segment.mode()).c_str(),
        "EC Level: ", DevTools::getHex(level).c_str(),
        "Char count: ", std::to_string(charCount).c_str(),
        "Payload length: ", std::to_string(ecInfo.codewords).c_str(),
        "EC length: ", std::to_string(ecInfo.ecCodewordsTotalCount()).c_str()
        );
#endif

    switch (segment.mode()) {
    case EncodingMode::numeric:
        *bitIndex += NumericEncoder::encode(segment.data(), segment.length(), buffer, *bitIndex);
        break;
    case EncodingMode::alphaNumeric:
        *bitIndex += AlphaNumericEncoder::encode(segment.data(), segment.length(), buffer, *bitIndex);
        break;
    case EncodingMode::byte: {
        UnsignedByte* bytes = segment.data();
        for (unsigned int idx = 0; idx < segment.length(); idx += 1) {
            Common::copyBits(bytes, 1, 0, false, buffer, *bitIndex, 8);
            *bitIndex += 8;
            bytes += 1;
        }
    }
        break;
    case EncodingMode::kanji:
        *bitIndex += KanjiEncoder::encode(segment.data(), segment.length(), buffer, *bitIndex);
        break;
    }
}

// ERROR CORRECTION ---------------------------------------------------------------------------------------------------------------------------------

/// Generate Error correction bytes
/// @return Binary data (should be deleted on unused)
UnsignedByte* QRMatrixEncoder_generateErrorCorrections(
    /// Bytes from previous (encode data) step
    UnsignedByte* encodedData,
    /// EC Info from previous step
    ErrorCorrectionInfo ecInfo,
    /// Group number: 0, 1
    UnsignedByte group,
    /// Block number: 0, ...
    UnsignedByte block
) {
    UnsignedByte maxGroup = ecInfo.groupCount();
    if (group >= maxGroup) {
        throw QR_EXCEPTION("Invalid group number");
    }
    UnsignedByte maxBlock = 0;
    unsigned int offset = 0;
    unsigned int blockSize = 0;
    switch (group) {
    case 0:
        maxBlock = ecInfo.group1Blocks;
        blockSize = ecInfo.group1BlockCodewords;
        offset = block * ecInfo.group1BlockCodewords;
        break;
    case 1:
        maxBlock = ecInfo.group2Blocks;
        blockSize = ecInfo.group2BlockCodewords;
        offset = ecInfo.group1Blocks * ecInfo.group1BlockCodewords + ecInfo.group2BlockCodewords * block;
        break;
    default:
        break;
    }
    if (block >= maxBlock) {
        throw QR_EXCEPTION("Invalid block number");
    }

    Polynomial mesg(blockSize);
    for (unsigned int index = 0; index < blockSize; index += 1) {
        mesg.terms[index] = encodedData[offset + index];
    }
    Polynomial ecc = mesg.getErrorCorrections(ecInfo.ecCodewordsPerBlock);
    UnsignedByte* result = Common::allocate(ecc.length);
    for (unsigned int index = 0; index < ecc.length; index += 1) {
        result[index] = ecc.terms[index];
    }
#if LOGABLE
    LOG("DATA to ECC", ":",
        "group=", std::to_string(group).c_str(),
        "block=", std::to_string(block).c_str(),
        DevTools::getBin(mesg.terms, mesg.length).c_str());
    LOG("ECC", ":",
        "group=", std::to_string(group).c_str(),
        "block=", std::to_string(block).c_str(),
        DevTools::getBin(result, ecc.length).c_str());
#endif
    return result;
}


/// Generate Error correction bytes
/// @return Binary data (should be deleted on unused). 2D array contains EC bytes for all blocks.
UnsignedByte** QRMatrixEncoder_generateErrorCorrections(
    /// Bytes from previous (encode data) step
    UnsignedByte* encodedData,
    /// EC Info from previous step
    ErrorCorrectionInfo ecInfo
) {
    UnsignedByte maxGroup = ecInfo.groupCount();
    UnsignedByte** result = new UnsignedByte* [ecInfo.ecBlockTotalCount()];
    unsigned int blockIndex = 0;
    for (UnsignedByte group = 0; group < maxGroup; group += 1) {
        UnsignedByte maxBlock = 0;
        switch (group) {
        case 0:
            maxBlock = ecInfo.group1Blocks;
            break;
        case 1:
            maxBlock = ecInfo.group2Blocks;
            break;
        default:
            break;
        }
        for (UnsignedByte block = 0; block < maxBlock; block += 1) {
            result[blockIndex] = QRMatrixEncoder_generateErrorCorrections(encodedData, ecInfo, group, block);
            blockIndex += 1;
        }
    }
    return result;
}

// INTERLEAVE ---------------------------------------------------------------------------------------------------------------------------------------

/// Interleave data codeworks
/// Throw error if QR has only 1 block in total (check ecInfo before call this function)
/// @return Binary data (should be deleted on unused)
UnsignedByte* QRMatrixEncoder_interleave(
    /// Encoded data
    UnsignedByte* encodedData,
    /// EC info
    ErrorCorrectionInfo ecInfo
) {
    if (ecInfo.ecBlockTotalCount() == 1) {
        throw QR_EXCEPTION("Interleave not required");
    }
    UnsignedByte* result = new UnsignedByte [ecInfo.codewords];
    unsigned int blockCount = ecInfo.ecBlockTotalCount();
    UnsignedByte* blockPtr[blockCount];
    UnsignedByte* blockEndPtr[blockCount];

    UnsignedByte groupCount = ecInfo.groupCount();
    unsigned int blockIndex = 0;

    for (UnsignedByte group = 0; group < groupCount; group += 1) {
        UnsignedByte blockCount = ecInfo.group1Blocks;
        unsigned int blockSize = ecInfo.group1BlockCodewords;
        if (group == 1) {
            blockCount = ecInfo.group2Blocks;
            blockSize = ecInfo.group2BlockCodewords;
        }
        for (UnsignedByte block = 0; block < blockCount; block += 1) {
            unsigned int offset = 0;
            if (group == 1) {
                offset += ecInfo.group1Blocks * ecInfo.group1BlockCodewords;
            }
            offset += block * blockSize;
            blockPtr[blockIndex] = &encodedData[offset];
            blockEndPtr[blockIndex] = &encodedData[offset + blockSize - 1];
            blockIndex += 1;
        }
    }

    blockIndex = 0;
    unsigned int resIndex = 0;
    bool found = true;
    while (found) {
        result[resIndex] = *blockPtr[blockIndex];
        resIndex += 1;
        blockPtr[blockIndex] += 1;

        unsigned int loopCount = 0;
        found = false;
        while (loopCount < blockCount) {
            blockIndex += 1;
            if (blockIndex >= blockCount) {
                blockIndex = 0;
            }
            if (blockPtr[blockIndex] <= blockEndPtr[blockIndex]) {
                found = true;
                break;
            }
            loopCount += 1;
        }
    }
    return result;
}

/// Interleave error correction codeworks
/// Throw error if QR has only 1 block in total (check ecInfo before call this function)
/// @return Binary data (should be deleted on unused)
UnsignedByte* QRMatrixEncoder_interleave(
    /// Error correction data
    UnsignedByte** data,
    /// EC info
    ErrorCorrectionInfo ecInfo
) {
    if (ecInfo.ecBlockTotalCount() == 1) {
        throw QR_EXCEPTION("Interleave not required");
    }
    unsigned int blockCount = ecInfo.ecBlockTotalCount();
    UnsignedByte* result = new UnsignedByte[ecInfo.ecCodewordsTotalCount()];
    unsigned int resIndex = 0;
    for (unsigned int index = 0; index < ecInfo.ecCodewordsPerBlock; index += 1) {
        for (unsigned int jndex = 0; jndex < blockCount; jndex += 1) {
            result[resIndex] = data[jndex][index];
            resIndex += 1;
        }
    }
    return result;
}

// FINALIZE -----------------------------------------------------------------------------------------------------------------------------------------

void QRMatrixEncoder_clean(UnsignedByte* buffer, UnsignedByte** ecBuffer, ErrorCorrectionInfo ecInfo) {
    for (unsigned int index = 0; index < (ecInfo.group1Blocks + ecInfo.group2Blocks); index += 1) {
        delete[] ecBuffer[index];
    }
    delete[] ecBuffer;
    delete[] buffer;
}

QRMatrixBoard QRMatrixEncoder_finishEncodingData(
    UnsignedByte* buffer,
    ErrorCorrectionInfo ecInfo,
    unsigned int* bitIndex,
    UnsignedByte maskId,
    QRMatrixExtraMode extraMode
) {
    bool isMicro = (extraMode.mode == EncodingExtraMode::microQr);
    bool isMicroV13 = isMicro && ((ecInfo.version == 1) || ecInfo.version == 3);
    unsigned int bufferLen = ecInfo.codewords;
    unsigned int bufferBitsLen = bufferLen * 8;
    if (isMicroV13) {
        bufferBitsLen -= 4;
    }
    /// Terminator
    unsigned int terminatorLength = isMicro ? Common::microTerminatorLength(ecInfo.version) : 4;
    for (unsigned int index = 0; *bitIndex < bufferBitsLen && index < terminatorLength; index += 1) {
        *bitIndex += 1;
    }

    /// Make data multiple by 8
    if (isMicroV13) {
        while ((*bitIndex < bufferBitsLen - 4) && (*bitIndex % 8 != 0)) {
            *bitIndex += 1;
        }
    } else {
        while (*bitIndex < bufferBitsLen && *bitIndex % 8 != 0) {
            *bitIndex += 1;
        }
    }

    /// Fill up
    UnsignedByte byteFilling1 = 0b11101100; // 0xEC
    UnsignedByte byteFilling2 = 0b00010001; // 0x11
    UnsignedByte curByteFilling = byteFilling1;
    while (*bitIndex < bufferBitsLen - (isMicroV13 ? 4 : 0)) {
        Common::copyBits(
            (UnsignedByte*)&curByteFilling,
            1, // size of curByteFilling
            0, // Bit index of curByteFilling
            false, // copy 1 byte source, so we don't need to care byte order here
            buffer, // destination
            *bitIndex, // destination bit index
            8
        );
        *bitIndex += 8;
        if (curByteFilling == byteFilling1) {
            curByteFilling = byteFilling2;
        } else {
            curByteFilling = byteFilling1;
        }
    }
    *bitIndex = bufferBitsLen;

    // Error corrections
    UnsignedByte** ecBuffer = QRMatrixEncoder_generateErrorCorrections(buffer, ecInfo);

#if LOGABLE
    LOG(
        "Payload:\n", DevTools::getBin(buffer, ecInfo.codewords).c_str()
    );
#endif

    // Interleave or not
    if (ecInfo.ecBlockTotalCount() > 1) {
        UnsignedByte *interleave = QRMatrixEncoder_interleave(buffer, ecInfo);
        UnsignedByte *ecInterleave = QRMatrixEncoder_interleave(ecBuffer, ecInfo);

#if LOGABLE
        LOG(
            "Payload (interleave):\n", DevTools::getBin(interleave, ecInfo.codewords).c_str(),
            "EC (interleave):\n", DevTools::getBin(ecInterleave, ecInfo.ecCodewordsTotalCount()).c_str()
        );
#endif

        QRMatrixBoard board(interleave, ecInterleave, ecInfo, maskId, isMicro);

        delete[] interleave;
        delete[] ecInterleave;
        QRMatrixEncoder_clean(buffer, ecBuffer, ecInfo);
        return board;
    } else {
#if LOGABLE
    LOG(
        "EC:\n", DevTools::getBin(ecBuffer[0], ecInfo.ecCodewordsPerBlock).c_str()
    );
#endif
        QRMatrixBoard board(buffer, ecBuffer[0], ecInfo, maskId, isMicro);
        QRMatrixEncoder_clean(buffer, ecBuffer, ecInfo);
        return board;
    }

}

QRMatrixBoard QRMatrixEncoder_encodeSingle(
    QRMatrixSegment* segments,
    unsigned int count,
    ErrorCorrectionLevel level,
    QRMatrixExtraMode extraMode,
    UnsignedByte minVersion,
    UnsignedByte maskId,
    UnsignedByte sequenceIndex,
    UnsignedByte sequenceTotal,
    UnsignedByte parity
) {
    unsigned int segCount = 0;
    for (unsigned int index = 0; index < count; index += 1) {
        if (segments[index].length() > 0) {
            segCount += 1;
        }
    }
    if (segCount == 0) {
        throw QR_EXCEPTION("No input.");
    }
    if (extraMode.mode == EncodingExtraMode::fnc1Second) {
        bool isValid = false;
        switch (extraMode.appIndicatorLength) {
        case 1: {
            UnsignedByte value = extraMode.appIndicator[0];
            isValid = (value >= 'a' && value <= 'z') || (value >= 'A' && value <= 'Z');
        }
            break;
        case 2:
        {
            UnsignedByte value1 = extraMode.appIndicator[0];
            UnsignedByte value2 = extraMode.appIndicator[1];
            isValid = (value1 >= '0' && value1 <= '9') && (value2 >= '0' && value2 <= '9');
        }
            break;
        default:
            break;
        }
        if (!isValid) {
            throw QR_EXCEPTION("Invalid Application Indicator for FNC1 Second Position mode");
        }
    }
    bool isStructuredAppend = sequenceTotal > 0 && sequenceTotal <= 16;
    ErrorCorrectionInfo ecInfo = QRMatrixEncoder_findVersion(segments, count, level, minVersion, extraMode, isStructuredAppend);
    if (ecInfo.version == 0) {
        throw QR_EXCEPTION("Unable to find suitable QR version.");
    }
    if (isStructuredAppend && extraMode.mode == EncodingExtraMode::microQr) {
        extraMode = QRMatrixExtraMode();
    }
    // Allocate
    UnsignedByte* buffer = Common::allocate(ecInfo.codewords);
    unsigned int bitIndex = 0;
    // Structured append
    if (isStructuredAppend) {
        UnsignedByte bufByte = 0b0011;
        Common::copyBits(&bufByte, 1, 4, false, buffer, bitIndex, 4);
        bitIndex += 4;
        Common::copyBits(&sequenceIndex, 1, 4, false, buffer, bitIndex, 4);
        bitIndex += 4;
        bufByte = sequenceTotal - 1;
        Common::copyBits(&bufByte, 1, 4, false, buffer, bitIndex, 4);
        bitIndex += 4;
        Common::copyBits(&parity, 1, 0, false, buffer, bitIndex, 8);
        bitIndex += 8;
    }
    // Encode data
    for (unsigned int index = 0; index < count; index += 1) {
        QRMatrixEncoder_encodeSegment(buffer, segments[index], index, level, ecInfo, &bitIndex, extraMode);
    }
    // Finish
    return QRMatrixEncoder_finishEncodingData(buffer, ecInfo, &bitIndex, maskId, extraMode);
}

// PUBLIC METHODS -----------------------------------------------------------------------------------------------------------------------------------

QRMatrixBoard QRMatrixEncoder::encode(
    QRMatrixSegment* segments,
    unsigned int count,
    ErrorCorrectionLevel level,
    QRMatrixExtraMode extraMode,
    UnsignedByte minVersion,
    UnsignedByte maskId
) {
    return QRMatrixEncoder_encodeSingle(
        segments, count, level, extraMode, minVersion, maskId, 0, 0, 0
    );
}

UnsignedByte QRMatrixEncoder::getVersion(
    QRMatrixSegment* segments,
    unsigned int count,
    ErrorCorrectionLevel level,
    QRMatrixExtraMode extraMode,
    bool isStructuredAppend
) {
    unsigned int segCount = 0;
    for (unsigned int index = 0; index < count; index += 1) {
        if (segments[index].length() > 0) {
            segCount += 1;
        }
    }
    if (segCount == 0) {
        return 0;
    }
    try {
        ErrorCorrectionInfo ecInfo = QRMatrixEncoder_findVersion(segments, count, level, 0, extraMode, isStructuredAppend);
        if (ecInfo.version == 0) {
            return 0;
        }
        return ecInfo.version;
    } catch (QRMatrixException exception) {
        return 0;
    }
}

QRMatrixBoard* QRMatrixEncoder::encode(
    /// Array of data parts to be encoded
    QRMatrixStructuredAppend* parts,
    /// Number of parts
    unsigned int count
) {
    if (count > 16) {
        throw QR_EXCEPTION("Structured Append only accepts 16 parts maximum");
    }
    if (count == 0) {
        throw QR_EXCEPTION("No input.");
    }
//    if (count == 1) {
        // Should change to encode single QR symbol or throw error?
        // But no rule prevents to make a Structured Append QR symbol single part
//    }
    UnsignedByte parity = 0;
    for (UnsignedByte index = 0; index < count; index += 1) {
        QRMatrixStructuredAppend part = parts[index];
        for (unsigned int segIndex = 0; segIndex < part.count; segIndex += 1) {
            QRMatrixSegment segment = part.segments[segIndex];
            for (unsigned int idx = 0; idx < segment.length(); idx += 1) {
                if (index == 0 && segIndex == 0 && idx == 0) {
                    parity = segment.data()[idx];
                } else {
                    parity = parity ^ segment.data()[idx];
                }
            }
        }
    }
    QRMatrixBoard* result = new QRMatrixBoard [count];
    for (UnsignedByte index = 0; index < count; index += 1) {
        QRMatrixStructuredAppend part = parts[index];
        try {
            result[index] = QRMatrixEncoder_encodeSingle(
                part.segments, part.count,
                part.level, part.extraMode,
                part.minVersion, part.maskId,
                index, count, parity
            );
        } catch (QRMatrixException exception) {
            delete[] result;
            throw exception;
            return nullptr;
        }
    }
    return result;
}
