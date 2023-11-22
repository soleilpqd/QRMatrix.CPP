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

#include <iostream>
#include <cstring>
#include <iostream>

#include "../../QRMatrix/qrmatrixencoder.h"
#include "../../String/utf8string.h"
#include "qrmatrixsvg.h"

#if __linux__
#define OUTPUT_PREFIX "/home/soleilpqd/Desktop/"
#elif __APPLE__
#define OUTPUT_PREFIX "/Users/soleilpqd/Desktop/"
#endif

using namespace std;
using namespace QRMatrix;

void encodeQR(const char* path, const UnsignedByte* raw, EncodingMode mode, unsigned int eci, bool isMicro = false, const char* detailPath = NULL) {
    try {
        QRMatrixSegment segment(mode, raw, std::strlen((const char*)raw), eci != 0 ? eci : defaultEciAssigmentValue);
        QRMatrixSegment segments[] = {segment};
        ErrorCorrectionLevel level = isMicro ? ErrorCorrectionLevel::low : ErrorCorrectionLevel::high;
        QRMatrixExtraMode extra = isMicro ? QRMatrixExtraMode(EncodingExtraMode::microQr) : QRMatrixExtraMode();
        QRMatrixBoard board = QRMatrixEncoder::encode(segments, 1, level, extra);
        cout << path << endl;
        cout << board.description(detailPath != NULL) << endl;
        QRMatrixSvg::draw(board, path, 10, isMicro);
        if (detailPath != NULL) {
            QRMatrixSvg::drawDetail(board, detailPath, 10);
        }
    } catch (QRMatrixException error) {
        std::cout << error.file << " " << std::to_string(error.line) << ": " << error.func << "\n" << error.description << std::endl;
    }
}

/// @brief Numeric mode
void testNumeric() {
    // This must be QR Version 1 (with EC high, 17 chars maximum)
    // And when we add one or more digits to string, QR Version must increase
    const UnsignedByte* raw = (const UnsignedByte*)"12345678901234567";
    encodeQR(OUTPUT_PREFIX"test_numeric.svg", raw, EncodingMode::numeric, 0);
}

/// @brief Alphanumeric mode
void testAlphaNumeric() {
    // This must be QR Version 1 (with EC high, 10 chars maximum)
    // And when we add one or more digits to string, QR Version must increase
    const UnsignedByte* raw = (const UnsignedByte*)"ABC$ 67890";
    encodeQR(OUTPUT_PREFIX"test_alphanumeric.svg", raw, EncodingMode::alphaNumeric, 0);
}

/// @brief Byte mode with standard encoding (Latin1)
void testLatin() {
    // This must be QR Version 1 (with EC high, 7 chars maximum)
    // And when we add one or more digits to string, QR Version must increase
    const UnsignedByte* raw = (const UnsignedByte*)"L1! \xA9\xC2\xE2";
    encodeQR(OUTPUT_PREFIX"test_latin.svg", raw, EncodingMode::byte, 0);
}

/// @brief Kanji mode (with ShiftJIS encoding)
void testKanji() {
    // This must be QR Version 1 (with EC high, 4 chars maximum)
    // And when we add one or more digits to string, QR Version must increase
    const UnsignedByte* raw = (const UnsignedByte*)"\x82\x4f\x82\x60\x82\xa0\x83\x41"; // "０Ａあア" ShiftJIS // \x88\x9f 亜
    encodeQR(OUTPUT_PREFIX"test_kanji.svg", raw, EncodingMode::kanji, 0);
}

/// @brief Byte mode using UTF-8 encoding directly
void testUtf8() {
    const UnsignedByte* raw = (const UnsignedByte*)"Hello world!\nXin chào thế giới!\nこんにちは世界！\n안녕하세요!\n你好世界！";
    encodeQR(OUTPUT_PREFIX"test_utf8.svg", raw, EncodingMode::byte, 0);
}

/// @brief ECI mode with UTF-8 encoding
void testUtf8ECI() {
    const UnsignedByte* raw = (const UnsignedByte*)"Hello world!\nXin chào thế giới!\nこんにちは世界！\n안녕하세요!\n你好世界！";
    encodeQR(OUTPUT_PREFIX"test_utf8eci.svg", raw, EncodingMode::byte, 26, false, OUTPUT_PREFIX"test_utf8eci_detail.svg");
}

/// @brief ECI mode with custom encoding (EUC-KR)
void testCustomECI() {
    const UnsignedByte* raw = (const UnsignedByte*)"\xbe\xc8\xb3\xe7\xc7\xcf\xbc\xbc\xbf\xe4\x21"; // "안녕하세요!" EUC-KR sequence
    encodeQR(OUTPUT_PREFIX"test_eci.svg", raw, EncodingMode::byte, 30); // 30 is ECI Indicator for KS X 1001 (which includes EUC-KR)
}

/// @brief Mixed modes
void testMixModes() {
    QRMatrixSegment segment1(EncodingMode::alphaNumeric, (const UnsignedByte*)"ABC123 ", 7);
    QRMatrixSegment segment2(EncodingMode::byte, (const UnsignedByte*)"\xbe\xc8\xb3\xe7\xc7\xcf\xbc\xbc\xbf\xe4\x21", 11, 30);
    QRMatrixSegment segments[] = {segment1, segment2};
    QRMatrixBoard board = QRMatrixEncoder::encode(segments, 2, ErrorCorrectionLevel::high);
    cout << "MIXED MODES:" << endl;
    cout << board.description() << endl;
    QRMatrixSvg::draw(board, OUTPUT_PREFIX"test_mixed.svg", 10);
}

/// @brief Mixed modes auto
void testAutoMixModes() {
    Utf8String text((const UnsignedByte*)"123456789 こんにちは世界！A56B 안녕하세요!");
    UnicodePoint unicodes = text.unicodes();
    unsigned int segmentCount = 0;
    ErrorCorrectionLevel ecLevel = ErrorCorrectionLevel::high;
    QRMatrixSegment* segments = unicodes.segments(ecLevel, &segmentCount);
    std::cout << "Mixed auto: " << std::to_string(segmentCount) << std::endl;
    for (unsigned int index = 0; index < segmentCount; index += 1) {
        std::cout << std::to_string(index) << ": " << std::to_string(segments[index].mode()) << std::endl;
    }
    QRMatrixBoard board = QRMatrixEncoder::encode(segments, segmentCount, ecLevel);
    delete[] segments;
    QRMatrixSvg::draw(board, OUTPUT_PREFIX"test_mixed_auto.svg", 10);
}

/// @brief Numeric mode
void testMicroNumeric() {
    const UnsignedByte* raw = (const UnsignedByte*)"12345";
    encodeQR(OUTPUT_PREFIX"test_micro_numeric.svg", raw, EncodingMode::numeric, 0, true);
}

/// @brief Alphanumeric mode
void testMicroAlphaNumeric() {
    const UnsignedByte* raw = (const UnsignedByte*)"A12345";
    encodeQR(OUTPUT_PREFIX"test_micro_alphanumeric.svg", raw, EncodingMode::alphaNumeric, 0, true);
}

/// @brief Kanji mode (with ShiftJIS encoding)
void testMicroKanji() {
    const UnsignedByte* raw = (const UnsignedByte*)"\x82\x4f\x82\x60\x82\xa0\x83\x41"; // "０Ａあア" ShiftJIS
    encodeQR(OUTPUT_PREFIX"test_micro_kanji.svg", raw, EncodingMode::kanji, 0, true);
}

/// @brief Byte mode using UTF-8 encoding directly
void testMicroByte() {
    const UnsignedByte* raw = (const UnsignedByte*)"안녕";
    encodeQR(OUTPUT_PREFIX"test_micro_byte.svg", raw, EncodingMode::byte, 0, true);
}

/// @brief Mixed modes auto
void testMicroMixModes() {
    Utf8String text((const UnsignedByte*)"1234こんにちは");
    UnicodePoint unicodes = text.unicodes();
    unsigned int segmentCount = 0;
    ErrorCorrectionLevel ecLevel = ErrorCorrectionLevel::low;
    QRMatrixSegment* segments = unicodes.segments(ecLevel, &segmentCount, true);
    std::cout << "Mixed auto: " << std::to_string(segmentCount) << std::endl;
    for (unsigned int index = 0; index < segmentCount; index += 1) {
        std::cout << std::to_string(index) << ": " << std::to_string(segments[index].mode()) << std::endl;
    }

    QRMatrixExtraMode extra = QRMatrixExtraMode(EncodingExtraMode::microQr);
    QRMatrixBoard board = QRMatrixEncoder::encode(segments, segmentCount, ecLevel, extra);
    delete[] segments;
    QRMatrixSvg::draw(board, OUTPUT_PREFIX"test_micro_mixed.svg", 10, true);
}

void testFNC1First() {
    QRMatrixSegment segment1(EncodingMode::numeric, (const UnsignedByte*)"01049123451234591597033130128", 29);
    QRMatrixSegment segment2(EncodingMode::alphaNumeric, (const UnsignedByte*)"%10ABC123", 9);
    QRMatrixSegment segments[] = {segment1, segment2};
    QRMatrixExtraMode extra(EncodingExtraMode::fnc1First);
    QRMatrixBoard board = QRMatrixEncoder::encode(segments, 2, ErrorCorrectionLevel::high, extra);
    QRMatrixSvg::draw(board, OUTPUT_PREFIX"test_fnc1_1st.svg", 10, false);
}

void testFNC1Second() {
    QRMatrixSegment segment1(EncodingMode::alphaNumeric, (const UnsignedByte*)"AA1234BBB112", 12);
    QRMatrixSegment segment2(EncodingMode::byte, (const UnsignedByte*)"text text text text\n", 20);
    QRMatrixSegment segments[] = {segment1, segment2};
    QRMatrixExtraMode extra((UnsignedByte*)"37", 2);
    QRMatrixBoard board = QRMatrixEncoder::encode(segments, 2, ErrorCorrectionLevel::high, extra);
    QRMatrixSvg::draw(board, OUTPUT_PREFIX"test_fnc1_2nd.svg", 10, false);
}

void testStructuredAppend() {
    ErrorCorrectionLevel ecLevel = ErrorCorrectionLevel::low;

    QRMatrixSegment segment11(EncodingMode::numeric, (const UnsignedByte*)"123", 3);
    QRMatrixSegment segment12(EncodingMode::alphaNumeric, (const UnsignedByte*)"ABC", 3);
    QRMatrixSegment segments1[] = {segment11, segment12};
    QRMatrixStructuredAppend part1(segments1, 2, ecLevel);

    QRMatrixSegment segment21(EncodingMode::numeric, (const UnsignedByte*)"345", 3);
    QRMatrixSegment segment22(EncodingMode::alphaNumeric, (const UnsignedByte*)"DEF", 3);
    QRMatrixSegment segments2[] = {segment21, segment22};
    QRMatrixStructuredAppend part2(segments2, 2, ecLevel);

    QRMatrixStructuredAppend allParts[] = {part1, part2};
    QRMatrixBoard* boards = QRMatrixEncoder::encode(allParts, 2);
    QRMatrixSvg::draw(boards[0], OUTPUT_PREFIX"test_structured_append_1.svg", 10, false);
    QRMatrixSvg::draw(boards[1], OUTPUT_PREFIX"test_structured_append_2.svg", 10, false);
    delete[] boards;
}

int main() {
    try {
        testNumeric();
        testAlphaNumeric();
        testLatin();
        testKanji();
        testUtf8();
        testUtf8ECI();
        testCustomECI();
        testMixModes();
        testAutoMixModes();

        testMicroNumeric();
        testMicroAlphaNumeric();
        testMicroByte();
        testMicroKanji();
        testMicroMixModes();

        testFNC1First();
        testFNC1Second();
        testStructuredAppend();
    } catch (QRMatrixException error) {
        std::cout << error.file << " " << std::to_string(error.line) << ": " << error.func << "\n" << error.description << std::endl;
        return 1;
    }

    return 0;
}
