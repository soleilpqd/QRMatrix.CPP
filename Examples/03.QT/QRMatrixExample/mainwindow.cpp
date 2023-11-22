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

#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <iostream>
#include <stdexcept>
#include "../../../QRMatrix/qrmatrixencoder.h"
#include <QtConcurrent/QtConcurrent>
#include "../../../String/utf8string.h"

using namespace QRMatrix;

QPixmap* imageFromBoard(QRMatrixBoard board, int scale, bool isMicro) {
    UnsignedByte quietZone = isMicro ? 2 : 4;
    int qrSize = (board.dimension() + 2 * quietZone) * scale;
    QPixmap* result = new QPixmap(qrSize, qrSize);
    result->fill(QColorConstants::White);
    QPainter painter(result);
    for (unsigned int row = 0; row < board.dimension(); row += 1) {
        for (unsigned int column = 0; column < board.dimension(); column += 1) {
            UnsignedByte cell = board.buffer()[row][column];
            UnsignedByte low = cell & BoardCell::lowMask;
            if (low == BoardCell::set) {
                painter.fillRect((column + quietZone) * scale, (row + quietZone) * scale, scale, scale, QColorConstants::Black);
            }
        }
    }
    painter.end();
    return result;
}

QMap<QString, QPixmap*> MainWindow_encode(QString text, const UnsignedByte* raw, EncodingMode mode, unsigned int eci, bool isMicro) {
    QMap<QString, QPixmap*> result;
    try {
        QRMatrixSegment segment(mode, raw, std::strlen((const char*)raw), eci != 0 ? eci : defaultEciAssigmentValue);
        QRMatrixSegment segments[] = {segment};
        QRMatrixBoard board = QRMatrixEncoder::encode(
            segments, 1,
            isMicro ? ErrorCorrectionLevel::low : ErrorCorrectionLevel::high,
            isMicro ? QRMatrixExtraMode(EncodingExtraMode::microQr) : QRMatrixExtraMode()
        );
        QPixmap* image = imageFromBoard(board, 10, isMicro);
        result[text] = image;
        std::cout << text.toStdString() << std::endl;
        std::cout << board.description(false) << std::endl;
    } catch (QRMatrixException error) {
        std::cout << "ERROR: " << error.file << " " << std::to_string(error.line) << ": " << error.func << "\n" << error.description << std::endl;
    }
    return result;
}

QMap<QString, QPixmap*> MainWindow_encodeMixModes() {
    QMap<QString, QPixmap*> result;
    try {
        QRMatrixSegment segment1(EncodingMode::alphaNumeric, (const UnsignedByte*)"ABC123 ", 7);
        QRMatrixSegment segment2(EncodingMode::byte, (const UnsignedByte*)"\xbe\xc8\xb3\xe7\xc7\xcf\xbc\xbc\xbf\xe4\x21", 11, 30);
        QRMatrixSegment segments[] = {segment1, segment2};
        QRMatrixBoard board = QRMatrixEncoder::encode(segments, 2, ErrorCorrectionLevel::high);
        QPixmap* image = imageFromBoard(board, 10, false);
        result["Mixed: ABC123 안녕하세요!"] = image;
    } catch (QRMatrixException error) {
        std::cout << "ERROR: " << error.file << " " << std::to_string(error.line) << ": " << error.func << "\n" << error.description << std::endl;
    }
    return result;
}

QMap<QString, QPixmap*> MainWindow_encodeAutoMixModes() {
    QMap<QString, QPixmap*> result;
    try {
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
        QPixmap* image = imageFromBoard(board, 10, false);
        result["Mixed Auto: 123456789 こんにちは世界！A56B 안녕하세요!"] = image;
    } catch (QRMatrixException error) {
        std::cout << "ERROR: " << error.file << " " << std::to_string(error.line) << ": " << error.func << "\n" << error.description << std::endl;
    }
    return result;
}

QMap<QString, QPixmap*> MainWindow_encodeMicroMixModes() {
    QMap<QString, QPixmap*> result;
    try {
        Utf8String text((const UnsignedByte*)"1234こんにちは");
        UnicodePoint unicodes = text.unicodes();
        unsigned int segmentCount = 0;
        ErrorCorrectionLevel ecLevel = ErrorCorrectionLevel::low;
        QRMatrixSegment* segments = unicodes.segments(ecLevel, &segmentCount, true);
        std::cout << "Micro Mixed: " << std::to_string(segmentCount) << std::endl;
        for (unsigned int index = 0; index < segmentCount; index += 1) {
            std::cout << std::to_string(index) << ": " << std::to_string(segments[index].mode()) << " " << std::to_string(segments[index].length()) << std::endl;
        }
        QRMatrixBoard board = QRMatrixEncoder::encode(
            segments, segmentCount, ecLevel,
            QRMatrixExtraMode(EncodingExtraMode::microQr)
        );
        delete[] segments;
        QPixmap* image = imageFromBoard(board, 10, true);
        result["Micro Mixed: 1234こんにちは"] = image;
    } catch (QRMatrixException error) {
        std::cout << "ERROR: " << error.file << " " << std::to_string(error.line) << ": " << error.func << "\n" << error.description << std::endl;
    }
    return result;
}

QMap<QString, QPixmap*> MainWindow_encodeFNC1First() {
    QMap<QString, QPixmap*> result;
    try {
        QRMatrixSegment segment1(EncodingMode::numeric, (const UnsignedByte*)"01049123451234591597033130128", 29);
        QRMatrixSegment segment2(EncodingMode::alphaNumeric, (const UnsignedByte*)"%10ABC123", 9);
        QRMatrixSegment segments[] = {segment1, segment2};
        QRMatrixExtraMode extra(EncodingExtraMode::fnc1First);
        QRMatrixBoard board = QRMatrixEncoder::encode(segments, 2, ErrorCorrectionLevel::high, extra);
        QPixmap* image = imageFromBoard(board, 10, false);
        result["FNC1 1st"] = image;
    } catch (QRMatrixException error) {
        std::cout << "ERROR: " << error.file << " " << std::to_string(error.line) << ": " << error.func << "\n" << error.description << std::endl;
    }
    return result;
}

QMap<QString, QPixmap*> MainWindow_encodeFNC1Second() {
    QMap<QString, QPixmap*> result;
    try {
        QRMatrixSegment segment1(EncodingMode::alphaNumeric, (const UnsignedByte*)"AA1234BBB112", 12);
        QRMatrixSegment segment2(EncodingMode::byte, (const UnsignedByte*)"text text text text\n", 20);
        QRMatrixSegment segments[] = {segment1, segment2};
        QRMatrixExtraMode extra((UnsignedByte*)"37", 2);
        QRMatrixBoard board = QRMatrixEncoder::encode(segments, 2, ErrorCorrectionLevel::high, extra);
        QPixmap* image = imageFromBoard(board, 10, false);
        result["FNC1 2nd"] = image;
    } catch (QRMatrixException error) {
        std::cout << "ERROR: " << error.file << " " << std::to_string(error.line) << ": " << error.func << "\n" << error.description << std::endl;
    }
    return result;
}

QMap<QString, QPixmap*> MainWindow_encodeStructuredAppend() {
    QMap<QString, QPixmap*> result;
    try {
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
        QPixmap* image1 = imageFromBoard(boards[0], 10, false);
        QPixmap* image2 = imageFromBoard(boards[1], 10, false);
        result["Structured Append 1"] = image1;
        result["Structured Append 2"] = image2;
        delete[] boards;
    } catch (QRMatrixException error) {
        std::cout << "ERROR: " << error.file << " " << std::to_string(error.line) << ": " << error.func << "\n" << error.description << std::endl;
    }
    return result;
}

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);
    connect(ui->previousButton, &QPushButton::clicked, this, &MainWindow::buttonPreviousOnClick);
    connect(ui->nextButton, &QPushButton::clicked, this, &MainWindow::buttonNextOnClick);
    scenes = new QMap<QString, QGraphicsScene*>();
    texts = new QList<QString>();
    currentIndex = 0;

    QFuture<QMap<QString, QPixmap*>> operation;
    // Numeric mode
    const UnsignedByte* raw = (const UnsignedByte* )"12345678901234567";
    operation = QtConcurrent::run(MainWindow_encode, "NUMERIC: 12345678901234567", raw, EncodingMode::numeric, 0, false);
    operation.then(this, [this](QMap<QString, QPixmap*> result) { finalizeOps(result); });
    // AlphaNumeric mode
    raw = (const UnsignedByte* )"ABC$ 67890";
    operation = QtConcurrent::run(MainWindow_encode, "ALPHA NUMERIC: ABC$ 67890", raw, EncodingMode::alphaNumeric, 0, false);
    operation.then(this, [this](QMap<QString, QPixmap*> result) { finalizeOps(result); });
    // Byte mode with standard encoding (Latin1)
    raw = (const UnsignedByte* )"L1! \xA9\xC2\xE2";
    operation = QtConcurrent::run(MainWindow_encode, "Latin-1: L1! ©Ââ", raw, EncodingMode::byte, 0, false);
    operation.then(this, [this](QMap<QString, QPixmap*> result) { finalizeOps(result); });
    // Kanji mode (with ShiftJIS encoding)
    raw = (const UnsignedByte* )"\x82\x4f\x82\x60\x82\xa0\x83\x41"; // \x88\x9f 亜
    operation = QtConcurrent::run(MainWindow_encode, "ShiftJIS: ０Ａあア", raw, EncodingMode::kanji, 0, false);
    operation.then(this, [this](QMap<QString, QPixmap*> result) { finalizeOps(result); });
    // Byte mode using UTF-8 encoding directly
    raw = (const UnsignedByte* )"Hello world!\nXin chào thế giới!\nこんにちは世界！\n안녕하세요!\n你好世界！";
    operation = QtConcurrent::run(
        MainWindow_encode,
        "UTF-8 no ECI: Hello world!\nXin chào thế giới!\nこんにちは世界！\n안녕하세요!\n你好世界！",
        raw, EncodingMode::byte, 0, false
    );
    operation.then(this, [this](QMap<QString, QPixmap*> result) { finalizeOps(result); });
    // ECI mode with UTF-8 encoding
    raw = (const UnsignedByte* )"Hello world!\nXin chào thế giới!\nこんにちは世界！\n안녕하세요!\n你好世界！";
    operation = QtConcurrent::run(
        MainWindow_encode,
        "UTF-8 with ECI: Hello world!\nXin chào thế giới!\nこんにちは世界！\n안녕하세요!\n你好世界！",
        raw, EncodingMode::byte, 26, false
    );
    operation.then(this, [this](QMap<QString, QPixmap*> result) { finalizeOps(result); });
    // ECI mode with custom encoding (EUC-KR)
    raw = (const UnsignedByte* )"\xbe\xc8\xb3\xe7\xc7\xcf\xbc\xbc\xbf\xe4\x21";
    operation = QtConcurrent::run(MainWindow_encode, "Custom ECI (KS X 1001): 안녕하세요!", raw, EncodingMode::byte, 30, false);
    operation.then(this, [this](QMap<QString, QPixmap*> result) { finalizeOps(result); });
    // Mixed modes
    operation = QtConcurrent::run(MainWindow_encodeMixModes);
    operation.then(this, [this](QMap<QString, QPixmap*> result) { finalizeOps(result); });
    // Mixed modes auto
    operation = QtConcurrent::run(MainWindow_encodeAutoMixModes);
    operation.then(this, [this](QMap<QString, QPixmap*> result) { finalizeOps(result); });

    // Micro QR
    // Numeric
    raw = (const UnsignedByte* )"12345";
    operation = QtConcurrent::run(MainWindow_encode, "MICRO NUMERIC: 12345", raw, EncodingMode::numeric, 0, true);
    operation.then(this, [this](QMap<QString, QPixmap*> result) { finalizeOps(result); });
    // AlphaNumeric
    raw = (const UnsignedByte* )"A12345";
    operation = QtConcurrent::run(MainWindow_encode, "MICRO ALPHA NUMERIC: A12345", raw, EncodingMode::alphaNumeric, 0, true);
    operation.then(this, [this](QMap<QString, QPixmap*> result) { finalizeOps(result); });
    // Byte
    raw = (const UnsignedByte* )"안녕";
    operation = QtConcurrent::run(MainWindow_encode, "MICRO BYTE: 안녕", raw, EncodingMode::byte, 0, true);
    operation.then(this, [this](QMap<QString, QPixmap*> result) { finalizeOps(result); });
    // Kanji
    raw = (const UnsignedByte* )"\x82\x60\x82\xa0\x83\x41\x82\x4f";
    operation = QtConcurrent::run(MainWindow_encode, "MICRO KANJI: Ａあア０", raw, EncodingMode::kanji, 0, true);
    operation.then(this, [this](QMap<QString, QPixmap*> result) { finalizeOps(result); });
    // Mix
    operation = QtConcurrent::run(MainWindow_encodeMicroMixModes);
    operation.then(this, [this](QMap<QString, QPixmap*> result) { finalizeOps(result); });

    // FNC1 Fist Position
    operation = QtConcurrent::run(MainWindow_encodeFNC1First);
    operation.then(this, [this](QMap<QString, QPixmap*> result) { finalizeOps(result); });
    // FNC1 Second Position
    operation = QtConcurrent::run(MainWindow_encodeFNC1Second);
    operation.then(this, [this](QMap<QString, QPixmap*> result) { finalizeOps(result); });
    // Structured append
    operation = QtConcurrent::run(MainWindow_encodeStructuredAppend);
    operation.then(this, [this](QMap<QString, QPixmap*> result) { finalizeOps(result); });
}

MainWindow::~MainWindow() {
    delete ui;
    qDeleteAll(*scenes);
    delete scenes;
    delete texts;
}

void MainWindow::buttonPreviousOnClick() {
    if (currentIndex == 0) {
        if (scenes->size() > 0) {
            currentIndex = scenes->size() - 1;
            configureCurrentIndex();
        }
    } else {
        currentIndex -= 1;
        configureCurrentIndex();
    }
}

void MainWindow::buttonNextOnClick() {
    if (currentIndex + 1 < scenes->size()) {
        currentIndex += 1;
        configureCurrentIndex();
    } else {
        currentIndex = 0;
        configureCurrentIndex();
    }
}

void MainWindow::finalizeOps(QMap<QString, QPixmap*> result) {
    unsigned int before = texts->size();
    if (result.size() > 0) {
        foreach (QString text, result.keys()) {
            QPixmap* image = result[text];
            QGraphicsScene* scene = new QGraphicsScene(this);
            scene->addPixmap(*image);
            scene->setSceneRect(image->rect());
            texts->append(text);
            scenes->insert(text, scene);
            delete image;
        }
    }
    if (before == 0) {
        configureCurrentIndex();
    }
}

void MainWindow::configureCurrentIndex() {
    if (texts->size() == 0) { return; }
    QString qtext = texts->at(currentIndex);
    QGraphicsScene* scene = scenes->value(qtext);
    ui->imageView->setScene(scene);
    ui->statusbar->showMessage(qtext);
}
