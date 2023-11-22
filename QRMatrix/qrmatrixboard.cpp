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

#include "qrmatrixboard.h"
#include "common.h"
#include "Exception/qrmatrixexception.h"
#include <math.h>

#if LOGABLE
#include "../DevTools/devtools.h"
#endif

using namespace QRMatrix;
using namespace std;

QRMatrixBoard::~QRMatrixBoard() {
    if (dimension_ == 0) {
        return;
    }
    for (UnsignedByte index = 0; index < dimension_; index += 1) {
        delete[] buffer_[index];
    }
    delete[] buffer_;
}

QRMatrixBoard::QRMatrixBoard(QRMatrixBoard &other) {
    dimension_ = other.dimension_;
    buffer_ = new UnsignedByte* [dimension_];
    for (UnsignedByte index = 0; index < dimension_; index += 1) {
        buffer_[index] = new UnsignedByte [dimension_];
        for (UnsignedByte jndex = 0; jndex < dimension_; jndex += 1) {
            buffer_[index][jndex] = other.buffer_[index][jndex];
        }
    }
}

void QRMatrixBoard::operator=(QRMatrixBoard other) {
    if (dimension_ > 0) {
        for (UnsignedByte index = 0; index < dimension_; index += 1) {
            delete[] buffer_[index];
        }
        delete[] buffer_;
        buffer_ = nullptr;
    }
    dimension_ = other.dimension_;
    if (dimension_ > 0) {
        buffer_ = new UnsignedByte* [dimension_];
        for (UnsignedByte index = 0; index < dimension_; index += 1) {
            buffer_[index] = new UnsignedByte [dimension_];
            for (UnsignedByte jndex = 0; jndex < dimension_; jndex += 1) {
                buffer_[index][jndex] = other.buffer_[index][jndex];
            }
        }
    }
}

QRMatrixBoard::QRMatrixBoard() {
    dimension_ = 0;
    buffer_ = nullptr;
}

// Internal ========================================================================================

void QRMatrixBoard_setSquare(
    QRMatrixBoard* board,
    UnsignedByte row,
    UnsignedByte column,
    UnsignedByte size,
    bool isFill,
    bool isSet,
    UnsignedByte prefix
) {
    UnsignedByte value = (isSet ? BoardCell::set : BoardCell::unset) | prefix;
    UnsignedByte** buffer = board->buffer();
    if (isFill) {
        for (UnsignedByte rIndex = row; rIndex < row + size; rIndex += 1) {
            for (UnsignedByte cIndex = column; cIndex < column + size; cIndex += 1) {
                buffer[rIndex][cIndex] = value;
            }
        }
    } else {
        for (UnsignedByte index = 0; index < size; index += 1) {
            buffer[row + index][column] = value;
            buffer[row][column + index] = value;
            buffer[row + index][column + size - 1] = value;
            buffer[row + size - 1][column + index] = value;
        }
    }
}

/// Required remainder bits length
UnsignedByte QRMatrixBoard_remainderBitsLength(UnsignedByte version) {
    if (version >= 2 && version <= 6) {
        return 7;
    }
    if ((version >= 14 && version <= 20) || (version >= 28 && version <= 34)) {
        return 3;
    }
    if (version >= 21 && version <= 27) {
        return 4;
    }
    return 0;
}

// Initinalize QR =============================================================================================

void QRMatrixBoard_addFinderPattern(QRMatrixBoard* board, UnsignedByte row, UnsignedByte column) {
    QRMatrixBoard_setSquare(board, row, column, 7, false, true, BoardCell::finder);
    QRMatrixBoard_setSquare(board, row + 1, column + 1, 5, false, false, BoardCell::finder);
    QRMatrixBoard_setSquare(board, row + 2, column + 2, 3, true, true, BoardCell::finder);
}

void QRMatrixBoard_addFinderPatterns(QRMatrixBoard* board, bool isMicro) {
    QRMatrixBoard_addFinderPattern(board, 0, 0);
    if (isMicro) { return; }
    QRMatrixBoard_addFinderPattern(board, 0, board->dimension() - 7);
    QRMatrixBoard_addFinderPattern(board, board->dimension() - 7, 0);
}

void QRMatrixBoard_addSeparators(QRMatrixBoard* board, bool isMicro) {
    UnsignedByte** buffer = board->buffer();
    UnsignedByte value = BoardCell::unset | BoardCell::separator;
    if (isMicro) {
        for (UnsignedByte index = 0; index < 8; index += 1) {
            buffer[7][index] = value;
            buffer[index][7] = value;
        }
    } else {
        for (UnsignedByte index = 0; index < 8; index += 1) {
            buffer[7][index] = value;
            buffer[index][7] = value;

            buffer[7][board->dimension() - index - 1] = value;
            buffer[index][board->dimension() - 8] = value;

            buffer[board->dimension() - 8][index] = value;
            buffer[board->dimension() - index - 1][7] = value;
        }
    }
}

void QRMatrixBoard_addAlignmentPattern(QRMatrixBoard* board, UnsignedByte row, UnsignedByte column) {
    UnsignedByte tlRow = row - 2;
    UnsignedByte tlCol = column - 2;
    UnsignedByte** buffer = board->buffer();
    for (UnsignedByte rIndex = tlRow; rIndex < tlRow + 5; rIndex += 1) {
        for (UnsignedByte cIndex = tlCol; cIndex < tlCol + 5; cIndex += 1) {
            if (buffer[rIndex][cIndex] != BoardCell::neutral) {
                return;
            }
        }
    }
    QRMatrixBoard_setSquare(board, tlRow, tlCol, 5, false, true, BoardCell::alignment);
    QRMatrixBoard_setSquare(board, tlRow + 1, tlCol + 1, 3, false, false, BoardCell::alignment);
    buffer[row][column] = BoardCell::set | BoardCell::alignment;
}

void QRMatrixBoard_addAlignmentPatterns(QRMatrixBoard* board, ErrorCorrectionInfo ecInfo) {
    if (ecInfo.version < 2) return;
    const UnsignedByte* array = Common::alignmentLocations(ecInfo.version);
    QRMatrixBoard_addAlignmentPattern(board, 6, 6);
    for (UnsignedByte index = 0; index < 6; index += 1) {
        UnsignedByte value = array[index];
        if (value > 0) {
            QRMatrixBoard_addAlignmentPattern(board, 6, value);
            QRMatrixBoard_addAlignmentPattern(board, value, 6);
            QRMatrixBoard_addAlignmentPattern(board, value, value);
            for (UnsignedByte jndex = 0; jndex < 6; jndex += 1) {
                UnsignedByte value2 = array[jndex];
                if (value2 > 0 && value != value2) {
                    QRMatrixBoard_addAlignmentPattern(board, value, value2);
                    QRMatrixBoard_addAlignmentPattern(board, value2, value);
                }
            }
        }
    }
}

void QRMatrixBoard_addTimingPatterns(QRMatrixBoard* board, bool isMicro) {
    UnsignedByte** buffer = board->buffer();
    UnsignedByte valueSet = BoardCell::timing | BoardCell::set;
    UnsignedByte valueUnset = BoardCell::timing | BoardCell::unset;
    UnsignedByte offset = isMicro ? 0 : 6;
    for (UnsignedByte index = 6; index < board->dimension() - offset; index += 1) {
        if ((index % 2) == 0) {
            buffer[offset][index] = valueSet;
            buffer[index][offset] = valueSet;
        } else {
            buffer[offset][index] = valueUnset;
            buffer[index][offset] = valueUnset;
        }
    }
}

void QRMatrixBoard_addDarkAndReservedAreas(QRMatrixBoard* board, ErrorCorrectionInfo ecInfo) {
    UnsignedByte** buffer = board->buffer();
    // Dark cell
    buffer[board->dimension() - 8][8] = BoardCell::dark | BoardCell::set;
    // Reseved cells for format
    for (UnsignedByte index = 0; index < 8; index += 1) {
        if (buffer[8][index] == BoardCell::neutral) {
            buffer[8][index] = BoardCell::format | BoardCell::unset;
        }
        if (buffer[index][8] == BoardCell::neutral) {
            buffer[index][8] = BoardCell::format | BoardCell::unset;
        }
        if (buffer[board->dimension() - index - 1][8] == BoardCell::neutral) {
            buffer[board->dimension() - index - 1][8] = BoardCell::format | BoardCell::unset;
        }
        buffer[8][board->dimension() - index - 1] = BoardCell::format | BoardCell::unset;
    }
    buffer[8][8] = BoardCell::format | BoardCell::unset;
    if (ecInfo.version < 7) return;
    for (UnsignedByte index = 0; index < 3; index += 1) {
        for (UnsignedByte jndex = 0; jndex < 6; jndex += 1) {
            buffer[jndex][board->dimension() - 9 - index] = BoardCell::version | BoardCell::unset;
            buffer[board->dimension() - 9 - index][jndex] = BoardCell::version | BoardCell::unset;
        }
    }
}

void QRMatrixBoard_addMicroReservedAreas(QRMatrixBoard* board, ErrorCorrectionInfo ecInfo) {
    UnsignedByte** buffer = board->buffer();
    for (UnsignedByte index = 0; index < 8; index += 1) {
        if (buffer[8][index] == BoardCell::neutral) {
            buffer[8][index] = BoardCell::format | BoardCell::unset;
        }
        if (buffer[index][8] == BoardCell::neutral) {
            buffer[index][8] = BoardCell::format | BoardCell::unset;
        }
    }
    buffer[8][8] = BoardCell::format | BoardCell::unset;
}

// Fill data & EC bits =============================================================================================

/// Fill data bit into cell; increase data byte & bit index.
void QRMatrixBoard_fillDataBit(
    QRMatrixBoard* board,
    UnsignedByte* data,
    UnsignedByte* errorCorrection,
    UnsignedByte phase,
    unsigned int* byteIndex,
    UnsignedByte* bitIndex,
    int row,
    int column
) {
    UnsignedByte** buffer = board->buffer();
    UnsignedByte value = 0;
    UnsignedByte mask = 0b10000000 >> *bitIndex;
    UnsignedByte prefix = 0x00;

//    std::cout << "DEBUG: " << std::to_string(row) << ":" << std::to_string(column) << " ";
    switch (phase) {
    case 0:
        value = data[*byteIndex];
//        std::cout << "data";
        break;
    case 1:
        value = errorCorrection[*byteIndex];
        prefix = BoardCell::errorCorrection;
//        std::cout << "ec";
        break;
    case 2:
        value = 0;
        mask = 0;
        prefix = BoardCell::remainder;
        break;
    default:
        break;
    }
//    std::cout << std::endl;
    if ((value & mask) > 0) {
        buffer[row][column] = BoardCell::set | prefix;
    } else {
        buffer[row][column] = BoardCell::unset | prefix;
    }
    *bitIndex += 1;
    if (*bitIndex > 7) {
        *bitIndex = 0;
        *byteIndex += 1;
    }
}

/// Check to switch data source: content data, EC data, reminder bits.
bool QRMatrixBoard_checkFilledBit(
    unsigned int* bitCount,
    unsigned int dataBitTotal,
    unsigned int ecBitTotal,
    unsigned int* byteIndex,
    UnsignedByte* bitIndex,
    UnsignedByte* phase,
    UnsignedByte remainderCount
) {
    *bitCount += 1;
    switch (*phase) {
    case 0:
        if (*bitCount >= dataBitTotal) {
            *bitCount = 0;
            *phase += 1;
            *byteIndex = 0;
            *bitIndex = 0;
        }
        break;
    case 1:
        if (*bitCount >= ecBitTotal) {
            *bitCount = 0;
            *phase += 1;
            *byteIndex = 0;
            *bitIndex = 0;
            if (remainderCount == 0) {
                return true;
            }
        }
        break;
    case 2:
        return *bitCount >= remainderCount;
    default:
        break;
    }
    return false;
}

/// Fill content, EC data into QR board
void QRMatrixBoard_placeData(
    QRMatrixBoard* board,
    UnsignedByte* data,
    UnsignedByte* errorCorrection,
    ErrorCorrectionInfo ecInfo,
    UnsignedByte remainderCount,
    bool isMicro
) {
    bool isMicroV13 = isMicro && (ecInfo.version == 1 || ecInfo.version == 3);
    bool isUpward = true;
    UnsignedByte** buffer = board->buffer();
    int column = board->dimension() - 1;
    unsigned int byteIndex = 0;
    UnsignedByte bitIndex = 0;
    unsigned int bitCount = 0;
    unsigned int dataBitTotal = ecInfo.codewords * 8;
    if (isMicroV13) {
        dataBitTotal -= 4;
    }
    unsigned int ecBitTotal = ecInfo.ecCodewordsTotalCount() * 8;
    UnsignedByte phase = 0;
    bool isCompleted = false;

    while (column >= 0 && !isCompleted) {
        if (isUpward) {
            for (int row = board->dimension() - 1; row >= 0 && !isCompleted; row -= 1) {
                if (buffer[row][column] == BoardCell::neutral) {
                    QRMatrixBoard_fillDataBit(
                        board, data, errorCorrection, phase,
                        &byteIndex, &bitIndex, row, column
                    );
                    isCompleted = QRMatrixBoard_checkFilledBit(
                        &bitCount, dataBitTotal, ecBitTotal, &byteIndex,
                        &bitIndex, &phase, remainderCount
                    );
                }
                if (column > 0 && buffer[row][column - 1] == BoardCell::neutral) {
                    QRMatrixBoard_fillDataBit(
                        board, data, errorCorrection, phase,
                        &byteIndex, &bitIndex, row, column - 1
                    );
                    isCompleted = QRMatrixBoard_checkFilledBit(
                        &bitCount, dataBitTotal, ecBitTotal, &byteIndex,
                        &bitIndex, &phase, remainderCount
                    );
                }
            }
        } else {
            for (int row = 0; row < board->dimension() && !isCompleted; row += 1) {
                if (buffer[row][column] == BoardCell::neutral) {
                    QRMatrixBoard_fillDataBit(
                        board, data, errorCorrection, phase,
                        &byteIndex, &bitIndex, row, column
                    );
                    isCompleted = QRMatrixBoard_checkFilledBit(
                        &bitCount, dataBitTotal, ecBitTotal, &byteIndex,
                        &bitIndex, &phase, remainderCount
                    );
                }
                if (column > 0 && buffer[row][column - 1] == BoardCell::neutral) {
                    QRMatrixBoard_fillDataBit(
                        board, data, errorCorrection, phase,
                        &byteIndex, &bitIndex, row, column - 1
                    );
                    isCompleted = QRMatrixBoard_checkFilledBit(
                        &bitCount, dataBitTotal, ecBitTotal, &byteIndex,
                        &bitIndex, &phase, remainderCount
                    );
                }
            }
        }
        column -= 2;
        if (!isMicro && column == 6) {
            column -= 1;
        }
        isUpward = !isUpward;
    }
}

// Masking QR board =============================================================================================

UnsignedByte** QRMatrixBoard_mask(QRMatrixBoard* board, UnsignedByte maskNum) {
    UnsignedByte** result = new UnsignedByte* [board->dimension()];
    UnsignedByte** buffer = board->buffer();
    for (UnsignedByte row = 0; row < board->dimension(); row += 1) {
        result[row] = new UnsignedByte [board->dimension()];
        for (UnsignedByte column = 0; column < board->dimension(); column += 1) {
            UnsignedByte byte = buffer[row][column];
            UnsignedByte low = byte & BoardCell::lowMask;
            bool isFunc = (byte & BoardCell::funcMask) > 0;
            if (isFunc) {
                result[row][column] = byte;
                continue;
            }
            if (low == BoardCell::set) {
                low = BoardCell::unset;
            } else if (low == BoardCell::unset) {
                low = BoardCell::set;
            }
            UnsignedByte maskedByte = low;
            switch (maskNum) {
            case 0:
                if (((row + column) % 2) == 0) {
                    result[row][column] = maskedByte;
                } else {
                    result[row][column] = byte;
                }
                break;
            case 1:
                if ((row % 2) == 0) {
                    result[row][column] = maskedByte;
                } else {
                    result[row][column] = byte;
                }
                break;
            case 2:
                if ((column % 3) == 0) {
                    result[row][column] = maskedByte;
                } else {
                    result[row][column] = byte;
                }
                break;
            case 3:
                if (((row + column) % 3) == 0) {
                    result[row][column] = maskedByte;
                } else {
                    result[row][column] = byte;
                }
                break;
            case 4:
                if (((UnsignedByte)(floor((double)row / 2) + floor((double)column / 3)) % 2) == 0) {
                    result[row][column] = maskedByte;
                } else {
                    result[row][column] = byte;
                }
                break;
            case 5:
                if (((row * column) % 2 + (row * column) % 3) == 0) {
                    result[row][column] = maskedByte;
                } else {
                    result[row][column] = byte;
                }
                break;
            case 6:
                if ((((row * column) % 2 + (row * column) % 3) % 2) == 0) {
                    result[row][column] = maskedByte;
                } else {
                    result[row][column] = byte;
                }
                break;
            case 7:
                if ((((row + column) % 2 + (row * column) % 3) % 2) == 0) {
                    result[row][column] = maskedByte;
                } else {
                    result[row][column] = byte;
                }
                break;
            }
        }
    }
    return result;
}

// Evaluate masked boards to choose the best =============================================================================================

unsigned int QRMatrixBoard_evaluateCondition1(UnsignedByte dimension, UnsignedByte** maskedBoard) {
    unsigned int result = 0;
    UnsignedByte sameColorCount = 0;
    UnsignedByte curColor = BoardCell::dark;
    for (UnsignedByte row = 0; row < dimension; row += 1) {
        for (UnsignedByte column = 0; column < dimension; column += 1) {
            UnsignedByte cell = maskedBoard[row][column] & BoardCell::lowMask;
            if (cell == curColor) {
                sameColorCount += 1;
            } else {
                curColor = cell;
                if (sameColorCount >= 5) {
                    result += 3 + (sameColorCount - 5);
                }
                sameColorCount = 0;
            }
        }
    }
    if (sameColorCount >= 5) {
        result += 3 + (sameColorCount - 5);
    }
    sameColorCount = 0;
    for (UnsignedByte column = 0; column < dimension; column += 1) {
        for (UnsignedByte row = 0; row < dimension; row += 1) {
            UnsignedByte cell = maskedBoard[row][column] & BoardCell::lowMask;
            if (cell == curColor) {
                sameColorCount += 1;
            } else {
                curColor = cell;
                if (sameColorCount >= 5) {
                    result += 3 + (sameColorCount - 5);
                }
                sameColorCount = 0;
            }
        }
    }
    if (sameColorCount >= 5) {
        result += 3 + (sameColorCount - 5);
    }
    return result;
}

unsigned int QRMatrixBoard_evaluateCondition2(UnsignedByte dimension, UnsignedByte** maskedBoard) {
    unsigned int result = 0;
    for (UnsignedByte row = 0; row < dimension - 1; row += 1) {
        for (UnsignedByte column = 0; column < dimension - 1; column += 1) {
            UnsignedByte cell = maskedBoard[row][column] & BoardCell::lowMask;
            UnsignedByte cell1 = maskedBoard[row][column + 1] & BoardCell::lowMask;
            UnsignedByte cell2 = maskedBoard[row + 1][column] & BoardCell::lowMask;
            UnsignedByte cell3 = maskedBoard[row + 1][column + 1] & BoardCell::lowMask;
            if (cell == cell1 && cell == cell2 && cell == cell3) {
                result += 3;
            }
        }
    }
    return result;
}

unsigned int QRMatrixBoard_evaluateCondition3(UnsignedByte dimension, UnsignedByte** maskedBoard) {
    static const UnsignedByte pattern1[11] = {
        BoardCell::set,
        BoardCell::unset,
        BoardCell::set,
        BoardCell::set,
        BoardCell::set,
        BoardCell::unset,
        BoardCell::set,
        BoardCell::unset,
        BoardCell::unset,
        BoardCell::unset,
        BoardCell::unset
    };
    static const UnsignedByte pattern2[11] = {
        BoardCell::unset,
        BoardCell::unset,
        BoardCell::unset,
        BoardCell::unset,
        BoardCell::set,
        BoardCell::unset,
        BoardCell::set,
        BoardCell::set,
        BoardCell::set,
        BoardCell::unset,
        BoardCell::set
    };
    unsigned int result = 0;
    for (UnsignedByte row = 0; row < dimension; row += 1) {
        for (UnsignedByte column = 0; column < dimension - 11; column += 1) {
            bool isMatched1 = true;
            bool isMatched2 = true;
            for (UnsignedByte index = 0; index < 11; index += 1) {
                UnsignedByte cell = maskedBoard[row][column + index] & BoardCell::lowMask;
                if (cell != pattern1[index]) {
                    isMatched1 = false;
                }
                if (cell != pattern2[index]) {
                    isMatched2 = false;
                }
                if (!isMatched1 && !isMatched2) {
                    break;
                }
            }
            if (isMatched1 || isMatched2) {
                result += 40;
            }
        }
    }
    for (UnsignedByte column = 0; column < dimension; column += 1) {
        for (UnsignedByte row = 0; row < dimension - 11; row += 1) {
            bool isMatched1 = true;
            bool isMatched2 = true;
            for (UnsignedByte index = 0; index < 11; index += 1) {
                UnsignedByte cell = maskedBoard[row + index][column] & BoardCell::lowMask;
                if (cell != pattern1[index]) {
                    isMatched1 = false;
                }
                if (cell != pattern2[index]) {
                    isMatched2 = false;
                }
                if (!isMatched1 && !isMatched2) {
                    break;
                }
            }
            if (isMatched1 || isMatched2) {
                result += 40;
            }
        }
    }
    return result;
}

unsigned int QRMatrixBoard_evaluateCondition4(UnsignedByte dimension, UnsignedByte** maskedBoard) {
    unsigned int total = dimension * dimension;
    unsigned int darkCount = 0;
    for (UnsignedByte row = 0; row < dimension; row += 1) {
        for (UnsignedByte column = 0; column < dimension; column += 1) {
            UnsignedByte cell = maskedBoard[row][column] & BoardCell::lowMask;
            if (cell == BoardCell::set) {
                darkCount += 1;
            }
        }
    }
    double percent = ((double)darkCount / (double)total) * 100.0f;
    unsigned int pre5 = percent / 5;
    pre5 *= 5;
    unsigned int next5 = pre5 + 5;
    pre5 = abs((int)pre5 - 50);
    next5 = abs((int)next5 - 50);
    pre5 = pre5 / 5;
    next5 = next5 / 5;

    unsigned int result = min(pre5, next5) * 10;
    return result;
}

unsigned int QRMatrixBoard_evaluateMicro(UnsignedByte dimension, UnsignedByte** maskedBoard) {
    UnsignedByte sum1 = 0;
    UnsignedByte sum2 = 0;
    for (unsigned int index = 0; index < dimension; index += 1) {
        UnsignedByte cell = maskedBoard[index][dimension - 1] & BoardCell::lowMask;
        if (cell == BoardCell::set) {
            sum1 += 1;
        }
        cell = maskedBoard[dimension - 1][index] & BoardCell::lowMask;
        if (cell == BoardCell::set) {
            sum2 += 1;
        }
    }
    if (sum1 <= sum2) {
        return sum1 * 16 + sum2;
    }
    return sum2 * 16 + sum1;
}

UnsignedByte QRMatrixBoard_evaluate(QRMatrixBoard* board, UnsignedByte maskId, bool isMicro) {
    static UnsignedByte microMaskIdMap[4] = {1, 4, 6, 7};
    bool isCustomMask = isMicro ? (maskId < 4) : (maskId < 8);
    if (isCustomMask) {
#if LOGABLE
    LOG(
        "MASKED ID: ", to_string(maskId).c_str()
    );
#endif
        UnsignedByte mId = isMicro ? microMaskIdMap[maskId] : maskId;
        UnsignedByte** mBoard = QRMatrixBoard_mask(board, mId);
        UnsignedByte** buffer = board->buffer();
        for (UnsignedByte row = 0; row < board->dimension(); row += 1) {
            for (UnsignedByte column = 0; column < board->dimension(); column += 1) {
                buffer[row][column] = mBoard[row][column];
            }
        }
        for (UnsignedByte row = 0; row < board->dimension(); row += 1) {
            delete[] mBoard[row];
        }
        delete[] mBoard;
        return maskId;
    }

    UnsignedByte numMasks = isMicro ? 4 : 8;
    UnsignedByte** maskedBoard[numMasks];
    unsigned int minScore = 0;
    UnsignedByte minId = 0;
    unsigned int maxScore = 0;
    UnsignedByte maxId = 0;
#if LOGABLE
    LOG("");
#endif
    for (UnsignedByte index = 0; index < numMasks; index += 1) {
        UnsignedByte mId = isMicro ? microMaskIdMap[index] : index;
        maskedBoard[index] = QRMatrixBoard_mask(board, mId);
        unsigned int score = isMicro ?
            QRMatrixBoard_evaluateMicro(board->dimension(), maskedBoard[index]) :
            QRMatrixBoard_evaluateCondition1(board->dimension(), maskedBoard[index]) +
            QRMatrixBoard_evaluateCondition2(board->dimension(), maskedBoard[index]) +
            QRMatrixBoard_evaluateCondition3(board->dimension(), maskedBoard[index]) +
            QRMatrixBoard_evaluateCondition4(board->dimension(), maskedBoard[index]);
#if LOGABLE
    cout << "MASK [" << to_string(index).c_str() << "]: " << to_string(score).c_str() << endl;
#endif
        if (minScore == 0 || minScore > score) {
            minScore = score;
            minId = index;
        }
        if (maxScore < score) {
            maxScore = score;
            maxId = index;
        }
    }

    UnsignedByte lasId = isMicro ? maxId : minId;

#if LOGABLE
    LOG(
        "MASKED ID: ", to_string(lasId).c_str()
    );
#endif

    UnsignedByte** buffer = board->buffer();
    for (UnsignedByte row = 0; row < board->dimension(); row += 1) {
        for (UnsignedByte column = 0; column < board->dimension(); column += 1) {
            buffer[row][column] = maskedBoard[lasId][row][column];
        }
    }

    for (UnsignedByte index = 0; index < numMasks; index += 1) {
        for (UnsignedByte row = 0; row < board->dimension(); row += 1) {
            delete[] maskedBoard[index][row];
        }
        delete[] maskedBoard[index];
    }

    return lasId;
}

// Version & format =============================================================================================

void QRMatrixBoard_getFormatBits(ErrorCorrectionLevel level, UnsignedByte maskId, UnsignedByte* buffer) {
    static Unsigned2Bytes typeFormats[] = {
        0b1010100000100100, 0b1010001001001010, 0b1011110011111000, 0b1011011010010110, 0b1000101111110010, 0b1000000110011100, 0b1001111100101110, 0b1001010101000000,
        0b1110111110001000, 0b1110010111100110, 0b1111101101010100, 0b1111000100111010, 0b1100110001011110, 0b1100011000110000, 0b1101100010000010, 0b1101001011101100,
        0b0010110100010010, 0b0010011101111100, 0b0011100111001110, 0b0011001110100000, 0b0000111011000100, 0b0000010010101010, 0b0001101000011000, 0b0001000001110110,
        0b0110101010111110, 0b0110000011010000, 0b0111111001100010, 0b0111010000001100, 0b0100100101101000, 0b0100001100000110, 0b0101110110110100, 0b0101011111011010
    };

    UnsignedByte index = level << 3;
    index |= maskId;

    Unsigned2Bytes value = typeFormats[index];
    UnsignedByte* valuePtr = (UnsignedByte*)&value;
    if (Common::isLittleEndian) {
        buffer[0] = valuePtr[1];
        buffer[1] = valuePtr[0];
    } else {
        buffer[0] = valuePtr[0];
        buffer[1] = valuePtr[1];
    }
#if LOGABLE
    LOG(
        "Format payload:\n",
        DevTools::getBin(buffer, 2).c_str()
    );
#endif
}

void QRMatrixBoard_getMicroFormatBits(ErrorCorrectionLevel level, UnsignedByte version, UnsignedByte maskId, UnsignedByte* buffer) {
    static Unsigned2Bytes typeFormats[] {
        0b1000100010001010, 0b1000001011100100, 0b1001110001010110, 0b1001011000111000, 0b1010101101011100, 0b1010000100110010, 0b1011111110000000, 0b1011010111101110,
        0b1100111100100110, 0b1100010101001000, 0b1101101111111010, 0b1101000110010100, 0b1110110011110000, 0b1110011010011110, 0b1111100000101100, 0b1111001001000010,
        0b0000110110111100, 0b0000011111010010, 0b0001100101100000, 0b0001001100001110, 0b0010111001101010, 0b0010010000000100, 0b0011101010110110, 0b0011000011011000,
        0b0100101000010000, 0b0100000001111110, 0b0101111011001100, 0b0101010010100010, 0b0110100111000110, 0b0110001110101000, 0b0111110100011010, 0b0111011101110100
    };
    UnsignedByte index = Common::microQRErrorCorrectionLevelValue(level, version);
    index = index << 2;
    index |= maskId;
    Unsigned2Bytes value = typeFormats[index];
    UnsignedByte* valuePtr = (UnsignedByte*)&value;
    if (Common::isLittleEndian) {
        buffer[0] = valuePtr[1];
        buffer[1] = valuePtr[0];
    } else {
        buffer[0] = valuePtr[0];
        buffer[1] = valuePtr[1];
    }
#if LOGABLE
    LOG(
        "Micro Format payload:\n",
        DevTools::getBin(buffer, 2).c_str()
        );
#endif
}

void QRMatrixBoard_getVersionBits(UnsignedByte version, UnsignedByte* buffer) {
    static Unsigned4Bytes versions[34] = {
        0b00011111001001010000000000000000,
        0b00100001011011110000000000000000,
        0b00100110101001100100000000000000,
        0b00101001001101001100000000000000,
        0b00101110111111011000000000000000,
        0b00110001110110001000000000000000,
        0b00110110000100011100000000000000,
        0b00111001100000110100000000000000,
        0b00111110010010100000000000000000,
        0b01000010110111100000000000000000,
        0b01000101000101110100000000000000,
        0b01001010100001011100000000000000,
        0b01001101010011001000000000000000,
        0b01010010011010011000000000000000,
        0b01010101101000001100000000000000,
        0b01011010001100100100000000000000,
        0b01011101111110110000000000000000,
        0b01100011101100010000000000000000,
        0b01100100011110000100000000000000,
        0b01101011111010101100000000000000,
        0b01101100001000111000000000000000,
        0b01110011000001101000000000000000,
        0b01110100110011111100000000000000,
        0b01111011010111010100000000000000,
        0b01111100100101000000000000000000,
        0b10000010011101010100000000000000,
        0b10000101101111000000000000000000,
        0b10001010001011101000000000000000,
        0b10001101111001111100000000000000,
        0b10010010110000101100000000000000,
        0b10010101000010111000000000000000,
        0b10011010100110010000000000000000,
        0b10011101010100000100000000000000,
        0b10100011000110100100000000000000,
    };

    Unsigned4Bytes value = versions[version - 7];
    UnsignedByte* valuePtr = (UnsignedByte*)&value;
    if (Common::isLittleEndian) {
        buffer[0] = valuePtr[3];
        buffer[1] = valuePtr[2];
        buffer[2] = valuePtr[1];
    } else {
        buffer[0] = valuePtr[0];
        buffer[1] = valuePtr[1];
        buffer[2] = valuePtr[2];
    }

#if LOGABLE
    LOG(
        "Version payload:\n",
        DevTools::getBin(buffer, 3).c_str()
    );
#endif
}

void QRMatrixBoard_setReservedCell(UnsignedByte row, UnsignedByte column, UnsignedByte** buffer, UnsignedByte value) {
    UnsignedByte origin = buffer[row][column];
    UnsignedByte high = origin & 0xF0;
    if (high != BoardCell::format && high != BoardCell::version) {
        throw QR_EXCEPTION("Invalid reserved cell");
    }
    buffer[row][column] = high | (value & 0x0F);
}

void QRMatrixBoard_placeMicroFormat(QRMatrixBoard* board, UnsignedByte maskId, ErrorCorrectionInfo ecInfo) {
    UnsignedByte formatBits[2]; // Use first 15bits only
    QRMatrixBoard_getMicroFormatBits(ecInfo.level, ecInfo.version, maskId, formatBits);
    UnsignedByte** buffer = board->buffer();

    for (UnsignedByte index = 0; index < 15; index += 1) {
        UnsignedByte byteIndex = index < 8 ? 0 : 1;
        UnsignedByte bitIndex = byteIndex == 0 ? index : index - 8;
        UnsignedByte mask = 0b10000000 >> bitIndex;
        UnsignedByte curByte = formatBits[byteIndex] & mask;
        UnsignedByte cell = curByte > 0 ? BoardCell::set : BoardCell::unset;
        if (index < 8) {
            QRMatrixBoard_setReservedCell(8, index + 1, buffer, cell);
        } else {
            QRMatrixBoard_setReservedCell(15 - index, 8, buffer, cell);
        }
    }
}

void QRMatrixBoard_placeFormatAndVersion(QRMatrixBoard* board, UnsignedByte maskId, ErrorCorrectionInfo ecInfo) {
    UnsignedByte formatBits[2]; // Use first 15bits only
    QRMatrixBoard_getFormatBits(ecInfo.level, maskId, formatBits);
    UnsignedByte** buffer = board->buffer();

    for (UnsignedByte index = 0; index < 15; index += 1) {
        UnsignedByte byteIndex = index < 8 ? 0 : 1;
        UnsignedByte bitIndex = byteIndex == 0 ? index : index - 8;
        UnsignedByte mask = 0b10000000 >> bitIndex;
        UnsignedByte curByte = formatBits[byteIndex] & mask;
        UnsignedByte cell = curByte > 0 ? BoardCell::set : BoardCell::unset;
        if (index < 6) {
            QRMatrixBoard_setReservedCell(8, index, buffer, cell);
            QRMatrixBoard_setReservedCell(board->dimension() - index - 1, 8, buffer, cell);
        } else if (index > 8) {
            QRMatrixBoard_setReservedCell(8, board->dimension() - (15 - index), buffer, cell);
            QRMatrixBoard_setReservedCell(14 - index, 8, buffer, cell);
        } else {
            switch (index) {
            case 6:
                QRMatrixBoard_setReservedCell(8, index + 1, buffer, cell);
                QRMatrixBoard_setReservedCell(board->dimension() - index - 1, 8, buffer, cell);
                break;
            case 7:
                QRMatrixBoard_setReservedCell(8, index + 1, buffer, cell);
                QRMatrixBoard_setReservedCell(8, board->dimension() - (15 - index), buffer, cell);
                break;
            case 8:
                QRMatrixBoard_setReservedCell(14 - index + 1, 8, buffer, cell);
                QRMatrixBoard_setReservedCell(8, board->dimension() - (15 - index), buffer, cell);
                break;
            default:
                break;
            }
        }
    }

    if (ecInfo.version < 7) return;

    UnsignedByte version[3];
    QRMatrixBoard_getVersionBits(ecInfo.version, version);
    UnsignedByte row1 = board->dimension() - 9;
    UnsignedByte col1 = 5;
    UnsignedByte row2 = 5;
    UnsignedByte col2 = board->dimension() - 9;

    for (UnsignedByte index = 0; index < 18; index += 1) {
        UnsignedByte byteIndex = index / 8;
        UnsignedByte bitIndex = index % 8;
        UnsignedByte mask = 0b10000000 >> bitIndex;
        UnsignedByte curByte = version[byteIndex] & mask;
        UnsignedByte cell = curByte > 0 ? BoardCell::set : BoardCell::unset;

        QRMatrixBoard_setReservedCell(row1, col1, buffer, cell);
        if (row1 == (board->dimension() - 11)) {
            row1 = board->dimension() - 9;
            col1 -= 1;
        } else {
            row1 -= 1;
        }

        QRMatrixBoard_setReservedCell(row2, col2, buffer, cell);
        if (col2 == (board->dimension() - 11)) {
            col2 = board->dimension() - 9;
            row2 -= 1;
        } else {
            col2 -= 1;
        }
    }
}

// INIT =============================================================================================

QRMatrixBoard::QRMatrixBoard(
    UnsignedByte* data,
    UnsignedByte* errorCorrection,
    ErrorCorrectionInfo ecInfo,
    UnsignedByte maskId,
    bool isMicro
) {
    dimension_ = isMicro ?
        Common::microDimensionByVersion(ecInfo.version) :
        Common::dimensionByVersion(ecInfo.version);
    buffer_ = new UnsignedByte* [dimension_];
    for (UnsignedByte index = 0; index < dimension_; index += 1) {
        buffer_[index] = new UnsignedByte [dimension_];
        for (UnsignedByte jndex = 0; jndex < dimension_; jndex += 1) {
            buffer_[index][jndex] = BoardCell::neutral;
        }
    }
    QRMatrixBoard_addFinderPatterns(this, isMicro);
    QRMatrixBoard_addSeparators(this, isMicro);
    if (!isMicro) {
        QRMatrixBoard_addAlignmentPatterns(this, ecInfo);
    }
    QRMatrixBoard_addTimingPatterns(this, isMicro);
    if (isMicro) {
        QRMatrixBoard_addMicroReservedAreas(this, ecInfo);
    } else {
        QRMatrixBoard_addDarkAndReservedAreas(this, ecInfo);
    }
    QRMatrixBoard_placeData(
        this, data, errorCorrection, ecInfo,
        isMicro ? 0 : QRMatrixBoard_remainderBitsLength(ecInfo.version),
        isMicro
    );
    UnsignedByte lastMaskId = QRMatrixBoard_evaluate(this, maskId, isMicro);
    if (isMicro) {
        QRMatrixBoard_placeMicroFormat(this, lastMaskId, ecInfo);
    } else {
        QRMatrixBoard_placeFormatAndVersion(this, lastMaskId, ecInfo);
    }
}

// PRINT =============================================================================================

string QRMatrixBoard::description(bool isTypeVisible) {
    if (dimension_ == 0) {
        return "";
    }
    string result = "  ";
    for (UnsignedByte index = 0; index < dimension_; index += 1) {
        if (index % 2 > 0) {
            result.append("..");
            continue;
        }
        if (index < 10) {
            result.append("0");
        }
        result.append(to_string(index));
    }
    result.append("\n");
    for (UnsignedByte index = 0; index < dimension_; index += 1) {
        if (index < 10) {
            result.append("0");
        }
        result.append(to_string(index));
        for (UnsignedByte jndex = 0; jndex < dimension_; jndex += 1) {
            UnsignedByte byte = buffer_[index][jndex];
            if (!isTypeVisible) {
                UnsignedByte low = byte & BoardCell::lowMask;
                if (low == BoardCell::unset) {
                    result.append(" □");
                } else if (low == BoardCell::set) {
                    result.append(" ■");
                } else {
                    result.append("  ");
                }
            } else {
                if (byte == BoardCell::neutral) {
                    result.append(" +");
                } else {
                    UnsignedByte low = byte & BoardCell::lowMask;
                    UnsignedByte high = byte & BoardCell::highMask;
                    bool isFunc = (byte & BoardCell::funcMask) > 0;
                    if (high == BoardCell::format || high == BoardCell::version) {
                        result.append("®");
                    } else if (isFunc) {
                        result.append("•");
                    } else if (high == BoardCell::errorCorrection) {
                        result.append(".");
                    } else {
                        result.append(" ");
                    }
                    switch (low) {
                    case BoardCell::unset:
                        result.append("□");
                        break;
                    case BoardCell::set:
                        result.append("■");
                        break;
                    default:
                        if (high == BoardCell::format || high == BoardCell::version) {
                            result.append("®");
                        } else {
                            result.append(" ");
                        }
                        break;
                    }
                }
            }
        }
        result.append("\n");
    }
    return result;
}
