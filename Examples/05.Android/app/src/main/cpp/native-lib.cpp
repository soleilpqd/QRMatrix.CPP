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

#include <jni.h>
#include <string>
#include <iostream>
#include <list>
#include <map>
#include "../../../../../../QRMatrix/qrmatrixencoder.h"
#include "../../../../../../String/utf8string.h"
#include "../../../../../../String/shiftjisstring.h"
#include "../../../../../../String/latinstring.h"

using namespace std;
using namespace QRMatrix;

enum QRExampleMode {
    mNumeric = 1,
    mAlphaNumeric = 2,
    mByte = 3,
    mKanji = 4
};

enum QRExampleECLevel {
    lLow = 1,
    lMedium = 2,
    lQuarter = 3,
    lHigh = 4
};

enum QRExampleExtraMode {
    mNone = 1,
    mMicroQR = 2,
    mFNC1First = 3,
    mFNC1Second = 4
};

map<int, list<QRMatrixStructuredAppend*>*> structuredAppendJobs;
map<int, list<QRMatrixBoard*>*> structuredAppendResults;

jbyteArray errorResult(string mesg, JNIEnv* env) {
    int len = mesg.size();
    jbyteArray result = env->NewByteArray(len + 1);
    char value = 1;
    env->SetByteArrayRegion(result, 0, 1, (jbyte *)&value);
    env->SetByteArrayRegion(result, 1, len, (jbyte *)mesg.data());
    return result;
}

jbyteArray successResult(QRMatrixBoard board, JNIEnv* env) {
    int len = board.dimension();
    len = len * len + 1;
    char value = 0;
    int offset = 0;
    jbyteArray result = env->NewByteArray(len + 1);
    env->SetByteArrayRegion(result, offset, 1, (jbyte *)&value);
    offset += 1;
    for (unsigned int row = 0; row < board.dimension(); row += 1) {
        env->SetByteArrayRegion(result, offset, board.dimension(), (jbyte *)board.buffer()[row]);
        offset += board.dimension();
    }
    return result;
}

EncodingMode convertMode(jint mode) {
    switch (mode) {
        case QRExampleMode::mNumeric:
            return EncodingMode::numeric;
        case QRExampleMode::mAlphaNumeric:
            return EncodingMode::alphaNumeric;
        case QRExampleMode::mByte:
            return EncodingMode::byte;
        case QRExampleMode::mKanji:
            return EncodingMode::kanji;
    }
}

ErrorCorrectionLevel convertEcLevel(jint level) {
    ErrorCorrectionLevel ecLevel;
    switch (level) {
        case QRExampleECLevel::lLow:
            ecLevel = ErrorCorrectionLevel::low;
            break;
        case QRExampleECLevel::lMedium:
            ecLevel = ErrorCorrectionLevel::medium;
            break;
        case QRExampleECLevel::lQuarter:
            ecLevel = ErrorCorrectionLevel::quarter;
            break;
        case QRExampleECLevel::lHigh:
            ecLevel = ErrorCorrectionLevel::high;
            break;
    }
    return ecLevel;
}

QRMatrixExtraMode* convertExtraMode(jint extraMode, jstring fnc1SecondAppId, JNIEnv* env) {
    switch (extraMode) {
        case QRExampleExtraMode::mNone:
            return new QRMatrixExtraMode();
        case QRExampleExtraMode::mMicroQR:
            return new QRMatrixExtraMode(EncodingExtraMode::microQr);
        case QRExampleExtraMode::mFNC1First:
            return new QRMatrixExtraMode(EncodingExtraMode::fnc1First);
        case QRExampleExtraMode::mFNC1Second:
        {
            jboolean isCopy = false;
            jint len = env->GetStringLength(fnc1SecondAppId);
            const char* appId = env->GetStringUTFChars(fnc1SecondAppId, &isCopy);
            return new QRMatrixExtraMode((UnsignedByte*)appId, len);
        }
        default:
            return nullptr;
    }
}

/* See description in MainActivity.kt  */
extern "C" JNIEXPORT jbyteArray JNICALL
Java_com_example_qrmatrixexample_MainActivity_makeQR(
        JNIEnv* env,
        jobject thiz,
        jobjectArray datas,
        jintArray modes,
        jbooleanArray needConverts,
        jintArray ecis,
        jint level,
        jint extraMode,
        jstring fnc1SecondAppId
) {
    ErrorCorrectionLevel ecLevel = convertEcLevel(level);

    jsize numberOfSegments = env->GetArrayLength(datas);
    QRMatrixSegment* segments = new QRMatrixSegment [numberOfSegments];
    jboolean isCopy = false;
    jint* modeArray = env->GetIntArrayElements(modes, &isCopy);
    jboolean * convertArray = env->GetBooleanArrayElements(needConverts, &isCopy);
    jint* eciArray = env->GetIntArrayElements(ecis, &isCopy);

    for (jsize index = 0; index < numberOfSegments; index += 1) {
        jbyteArray data = (jbyteArray)env->GetObjectArrayElement(datas, index);
        jsize dataLen = env->GetArrayLength(data);
        jint mode = modeArray[index];
        EncodingMode eMode = convertMode(mode);
        jboolean needConvert = convertArray[index];
        jint eci = eciArray[index];

        UnsignedByte *raw = new UnsignedByte [dataLen];
        env->GetByteArrayRegion(data, 0, dataLen, (jbyte*)raw);

        if (needConvert) {
            try {
                Utf8String utf8(raw, dataLen);
                switch (eMode) {
                    case EncodingMode::numeric:
                    case EncodingMode::alphaNumeric:
                    case EncodingMode::byte: {
                        // Actually, this is not necessary
                        LatinString latin1(utf8.unicodes());
                        segments[index].fill(eMode, latin1.bytes(), latin1.charactersCount(), eci);
                    }
                        break;
                    case EncodingMode::kanji: {
                        ShiftJisString shiftjis(utf8.unicodes());
                        if (shiftjis.maxBytesPerChar() < 2 || shiftjis.minBytesPerChar() < 2) {
                            delete[] raw;
                            return errorResult("Only support 2-bytes ShiftJIS Kanji", env);
                        }
                        segments[index].fill(eMode, shiftjis.bytes(), shiftjis.charactersCount() * 2, eci);
                    }
                        break;
                }
            } catch (QRMatrixException exception) {
                delete[] raw;
                return errorResult(exception.description, env);
            }
            delete[] raw;
        } else {
            try {
                segments[index].fill(eMode, raw, dataLen, eci);
            } catch (QRMatrixException exception) {
                delete[] raw;
                return errorResult(exception.description, env);
            }
            delete[] raw;
        }
    }
    QRMatrixExtraMode* extra = convertExtraMode(extraMode, fnc1SecondAppId, env);
    try {
        QRMatrixBoard board = QRMatrixEncoder::encode(segments, numberOfSegments, ecLevel, *extra);
        delete[] segments;
        delete extra;
        return successResult(board, env);
    } catch (QRMatrixException exception) {
        delete[] segments;
        delete extra;
        return errorResult(exception.description, env);
    }
}

/* See description in MainActivity.kt  */
extern "C" JNIEXPORT jbyteArray JNICALL
Java_com_example_qrmatrixexample_MainActivity_makeQRAuto(
        JNIEnv* env,
        jobject thiz,
        jstring text,
        jint level,
        jint extraMode,
        jstring fnc1SecondAppId
) {
    ErrorCorrectionLevel ecLevel = convertEcLevel(level);
    jsize utf8len = env->GetStringUTFLength(text);
    jboolean isCopy = false;
    const char* utf8bytes = env->GetStringUTFChars(text, &isCopy);
    Utf8String utf8String((const UnsignedByte*)utf8bytes, utf8len);
    UnicodePoint unicodes = utf8String.unicodes();
    unsigned int segCount = 0;
    QRMatrixSegment* segments = unicodes.segments(ecLevel, &segCount, extraMode == QRExampleExtraMode::mMicroQR);
    QRMatrixExtraMode* extra = convertExtraMode(extraMode, fnc1SecondAppId, env);
    try {
        QRMatrixBoard board = QRMatrixEncoder::encode(segments, segCount, ecLevel, *extra);
        delete[] segments;
        delete extra;
        return successResult(board, env);
    } catch (QRMatrixException exception) {
        delete[] segments;
        delete extra;
        return errorResult(exception.description, env);
    }
}

extern "C" JNIEXPORT jint JNICALL
Java_com_example_qrmatrixexample_MainActivity_startMakeStructuredAppendQR(
        JNIEnv* env,
        jobject thiz
){
    for (jint index = 0; index < INT_MAX; index += 1) {
        try {
            list<QRMatrixStructuredAppend*>* list = structuredAppendJobs.at(index);
        } catch (std::exception exception) {
            structuredAppendJobs[index] = new list<QRMatrixStructuredAppend*>;
            return index;
        }
    }
    return -1;
}

extern "C" JNIEXPORT jbyteArray JNICALL
Java_com_example_qrmatrixexample_MainActivity_addStructuredAppendQR(
    JNIEnv* env,
    jobject thiz,
    jint id,
    jobjectArray datas,
    jintArray modes,
    jbooleanArray needConverts,
    jintArray ecis,
    jint level,
    jint extraMode,
    jstring  fnc1SecondAppId
) {
    ErrorCorrectionLevel ecLevel = convertEcLevel(level);

    jsize numberOfSegments = env->GetArrayLength(datas);
    QRMatrixSegment* segments = new QRMatrixSegment [numberOfSegments];
    jboolean isCopy = false;
    jint* modeArray = env->GetIntArrayElements(modes, &isCopy);
    jboolean * convertArray = env->GetBooleanArrayElements(needConverts, &isCopy);
    jint* eciArray = env->GetIntArrayElements(ecis, &isCopy);

    for (jsize index = 0; index < numberOfSegments; index += 1) {
        jbyteArray data = (jbyteArray)env->GetObjectArrayElement(datas, index);
        jsize dataLen = env->GetArrayLength(data);
        jint mode = modeArray[index];
        EncodingMode eMode = convertMode(mode);
        jboolean needConvert = convertArray[index];
        jint eci = eciArray[index];

        UnsignedByte *raw = new UnsignedByte [dataLen];
        env->GetByteArrayRegion(data, 0, dataLen, (jbyte*)raw);

        if (needConvert) {
            try {
                Utf8String utf8(raw, dataLen);
                switch (eMode) {
                    case EncodingMode::numeric:
                    case EncodingMode::alphaNumeric:
                    case EncodingMode::byte: {
                        // Actually, this is not necessary
                        LatinString latin1(utf8.unicodes());
                        segments[index].fill(eMode, latin1.bytes(), latin1.charactersCount(), eci);
                    }
                        break;
                    case EncodingMode::kanji: {
                        ShiftJisString shiftjis(utf8.unicodes());
                        if (shiftjis.maxBytesPerChar() < 2 || shiftjis.minBytesPerChar() < 2) {
                            delete[] raw;
                            return errorResult("Invalid shiftjis", env);
                        }
                        segments[index].fill(eMode, shiftjis.bytes(), shiftjis.charactersCount() * 2, eci);
                    }
                        break;
                }
            } catch (QRMatrixException exception) {
                delete[] raw;
                return errorResult(exception.description, env);
            }
            delete[] raw;
        } else {
            try {
                segments[index].fill(eMode, raw, dataLen, eci);
            } catch (QRMatrixException exception) {
                delete[] raw;
                return errorResult(exception.description, env);
            }
            delete[] raw;
        }
    }
    QRMatrixExtraMode* extra = convertExtraMode(extraMode, fnc1SecondAppId, env);
    list<QRMatrixStructuredAppend*>* inputList = nullptr;
    try {
        inputList = structuredAppendJobs.at(id);
    } catch (std::exception exception) {
        inputList = new list<QRMatrixStructuredAppend*>;
        structuredAppendJobs[id] = inputList;
    }
    try {
        QRMatrixStructuredAppend *part = new QRMatrixStructuredAppend(segments, numberOfSegments, ecLevel);
        inputList->push_back(part);
        delete[] segments;
        delete extra;
    } catch (QRMatrixException exception) {
        delete[] segments;
        delete extra;
        return errorResult(exception.description, env);
    }
    return errorResult("", env);
}

extern "C" JNIEXPORT jbyteArray JNICALL
Java_com_example_qrmatrixexample_MainActivity_commitStructuredAppendQR(
    JNIEnv* env,
    jobject thiz,
    jint id
) {
    list<QRMatrixStructuredAppend*>* inputList = nullptr;
    try {
        inputList = structuredAppendJobs.at(id);
    } catch (std::exception exception) {
        return errorResult("No input", env);
    }
    QRMatrixStructuredAppend* parts = new QRMatrixStructuredAppend [inputList->size()];
    if (inputList->size() == 0) {
        return errorResult("No input", env);
    }
    unsigned int index = 0;
    for (QRMatrixStructuredAppend* part: *inputList) {
        parts[index] = *part;
        index += 1;
    }
    try {
        QRMatrixBoard* results = QRMatrixEncoder::encode(parts, inputList->size());
        list<QRMatrixBoard*>* resList = nullptr;
        try {
            resList = structuredAppendResults.at(id);
            for (QRMatrixBoard* board: *resList) {
                delete board;
            }
            delete resList;
            structuredAppendResults.erase(id);
        } catch (std::exception exception) {}
        resList = new list<QRMatrixBoard*>;
        structuredAppendResults[id] = resList;
        for (unsigned int idx = 0; idx < inputList->size(); idx += 1) {
            QRMatrixBoard* board = new QRMatrixBoard(results[idx]);
            resList->push_back(board);
        }
        return errorResult("", env);
    } catch (QRMatrixException exception) {
        return errorResult(exception.description, env);
    }
}

extern "C" JNIEXPORT jbyteArray JNICALL
Java_com_example_qrmatrixexample_MainActivity_getStructuredAppedQRBoard(
    JNIEnv* env,
    jobject thiz,
    jint id,
    jint index
) {
    list<QRMatrixBoard*>* resList = structuredAppendResults[id];
    unsigned int idx = 0;
    for (QRMatrixBoard* board: *resList) {
        if (index == idx) {
            return successResult(*board, env);
        }
        idx += 1;
    }
    return errorResult("Not found", env);
}

extern "C" JNIEXPORT void JNICALL
Java_com_example_qrmatrixexample_MainActivity_clearStructuredAppendQR(
    JNIEnv* env,
    jobject thiz,
    jint id
) {
    try {
        list<QRMatrixStructuredAppend*>* inputList = structuredAppendJobs.at(id);
        for (QRMatrixStructuredAppend* part: *inputList) {
            delete part;
        }
        delete inputList;
        structuredAppendJobs.erase(id);
    } catch (std::exception exception) {}
    try {
        list<QRMatrixBoard*>* resList = structuredAppendResults[id];
        for (QRMatrixBoard* board: *resList) {
            delete board;
        }
        delete resList;
        structuredAppendResults.erase(id);
    } catch (std::exception exception) {}
}