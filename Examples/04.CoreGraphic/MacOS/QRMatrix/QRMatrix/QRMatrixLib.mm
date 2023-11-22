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

#import "QRMatrixLib.h"
#import "qrmatrixencoder.h"
#import "utf8string.h"

using namespace QRMatrix;

@interface QRSegment()

@property ( nonatomic, strong ) NSData* data;
@property ( nonatomic, assign ) QRMode mode;
@property ( nonatomic, assign ) NSUInteger eci;

@end

@implementation QRSegment

-( instancetype )init:( NSData* )data mode:( QRMode )mode eci:( NSUInteger )eci {
    self = [ super init ];
    if ( self != nil ) {
        self.data = data;
        self.mode = mode;
        self.eci = eci;
    }
    return self;
}

@end

// MARK: -

@interface QRExtraData()

@property ( nonatomic, assign ) QRExtraMode mode;
@property ( nonatomic, nullable, strong ) NSString* fnc1SecondAppId;

@end

@implementation QRExtraData

-( instancetype )init:( QRExtraMode )mode {
    self = [ super init ];
    self.mode = mode;
    return self;
}

+( instancetype )fnc1Second:( NSString* )appId {
    QRExtraData* result = [[ QRExtraData alloc ] init:mFnc1Second ];
    result.fnc1SecondAppId = appId;
    return result;
}

@end

// MARK: -

@interface QRStructuredAppend()

@property ( nonatomic, strong ) NSArray<QRSegment*>* segments;
@property ( nonatomic, assign ) QRErrorCorrectionLevel ecLevel;
@property ( nonatomic, strong ) QRExtraData* extra;

@end

@implementation QRStructuredAppend

-( instancetype )init:( NSArray<QRSegment*>* )segments errorCorrectionLevel:( QRErrorCorrectionLevel )level extra:( QRExtraData* )extra {
    self = [ super init ];
    self.segments = segments;
    self.ecLevel = level;
    self.extra = extra;
    return self;
}

@end

// MARK: -

@interface QRMatrixLib()

+( nullable NSImage* )imageFromBoard:( QRMatrixBoard )board scale:( NSUInteger )scale isMicro:( BOOL )isMicro;

+( EncodingMode )mapMode:( QRMode )mode;
+( ErrorCorrectionLevel )convertECLevel:( QRErrorCorrectionLevel )level;

@end

@implementation QRMatrixLib

+( EncodingMode )mapMode:( QRMode )mode {
    switch (mode) {
        case mNumeric:
            return EncodingMode::numeric;
        case mAlphaNumeric:
            return EncodingMode::alphaNumeric;
        case mByte:
            return EncodingMode::byte;
        case mKanji:
            return EncodingMode::kanji;
    }
}

+( ErrorCorrectionLevel )convertECLevel:( QRErrorCorrectionLevel )level {
    ErrorCorrectionLevel qrLevel;
    switch (level) {
        case ecLow:
            qrLevel = low;
            break;
        case ecMedium:
            qrLevel = medium;
            break;
        case ecQuarter:
            qrLevel = quarter;
            break;
        case ecHigh:
            qrLevel = high;
            break;
    }
    return qrLevel;
}

+( QRMatrixExtraMode* )mapExtraMode:( QRExtraData* )extra {
    QRMatrixExtraMode *extraMode;
    if (extra != nil) {
        switch (extra.mode) {
            case mMicroQR:
                extraMode = new QRMatrixExtraMode(EncodingExtraMode::microQr);
                break;
            case mFnc1First:
                extraMode = new QRMatrixExtraMode(EncodingExtraMode::fnc1First);
                break;
            case mFnc1Second: {
                NSData *data = [ extra.fnc1SecondAppId dataUsingEncoding:NSASCIIStringEncoding ];
                extraMode = new QRMatrixExtraMode((UnsignedByte*)data.bytes, data.length);
            }
                break;
        }
    } else {
        extraMode = new QRMatrixExtraMode();
    }
    return extraMode;
}

+( NSImage* )makeQR:( NSArray<QRSegment*>* )segments
errorCorrectionLevel:( QRErrorCorrectionLevel )level
              scale:( NSUInteger )scale
              extra:( nullable QRExtraData* )extra
              error:( NSError** )error; {

    if ( scale < 1 ) {
        *error = [ NSError errorWithDomain:@"QRMatrixLib" code:1 userInfo:@{ NSLocalizedDescriptionKey: @"Invalid scale" }];
        return nil;
    }

    QRMatrixSegment* segs = new QRMatrixSegment [segments.count];
    try {
        for (NSUInteger index = 0; index < segments.count; index += 1) {
            QRSegment* seg = [segments objectAtIndex:index ];
            EncodingMode mode = [ self mapMode:seg.mode ];
            segs[index].fill(mode, (const UnsignedByte*)seg.data.bytes, (unsigned int)seg.data.length, (unsigned int)seg.eci);
        }
    } catch (QRMatrixException exception) {
        *error = [ NSError errorWithDomain:@"QRMatrixLib"
                                      code:3
                                  userInfo:@{ NSLocalizedDescriptionKey: [ NSString stringWithCString:exception.description.data()
                                                                                             encoding:NSUTF8StringEncoding ]}];
        delete[] segs;
        return nil;
    }

    ErrorCorrectionLevel qrLevel = [ self convertECLevel:level ];
    QRMatrixExtraMode* extraMode = [ self mapExtraMode:extra ];
    try {
        QRMatrixBoard board = QRMatrixEncoder::encode(segs, (unsigned int)segments.count, qrLevel, *extraMode);
        delete extraMode;
        NSImage *result = [ self imageFromBoard:board scale:scale isMicro:extra.mode == mMicroQR ];
        delete[] segs;
        return result;
    } catch (QRMatrixException exception) {
        *error = [ NSError errorWithDomain:@"QRMatrixLib"
                                      code:3
                                  userInfo:@{ NSLocalizedDescriptionKey: [ NSString stringWithCString:exception.description.data()
                                                                                             encoding:NSUTF8StringEncoding ]}];
        delete[] segs;
        return nil;
    }
}

+( NSImage* )imageFromBoard:( QRMatrixBoard )board scale:( NSUInteger )scale isMicro:( BOOL )isMicro {
    CGFloat quietZone = isMicro ? 2.0f : 4.0f;
    CGFloat qrSize = board.dimension() * scale + 2.0f * quietZone * scale;
    __block QRMatrixBoard myBoard = board;
    NSImage* result = [ NSImage imageWithSize:NSMakeSize( qrSize, qrSize ) flipped:YES drawingHandler:^BOOL( NSRect dstRect ) {
        CGContextRef context = [[ NSGraphicsContext currentContext ] CGContext ];
        // Background
        [ NSColor.whiteColor setFill ];
        CGContextFillRect( context, CGRectMake( 0, 0, qrSize, qrSize ));
        // Cells
        [ NSColor.blackColor setFill ];
        for ( NSUInteger row = 0; row < myBoard.dimension(); row += 1 ) {
            for ( NSUInteger column = 0; column < myBoard.dimension(); column += 1 ) {
                UnsignedByte cell = myBoard.buffer()[row][column];
                UnsignedByte low = cell & BoardCell::lowMask;
                if ( low == BoardCell::set ) {
                    CGContextFillRect( context, CGRectMake( (column + quietZone) * scale, (row + quietZone) * scale, scale, scale )); // 4 for quiet zone
                }
            }
        }
        return YES;
    }];
    return result;
}

+( NSImage* )makeMixedModesQR:( NSString* )text
         errorCorrectionLevel:( QRErrorCorrectionLevel )level
                        scale:( NSUInteger )scale
                        extra:( nullable QRExtraData* )extra
                        error:( NSError** )error {
    NSData* data = [ text dataUsingEncoding:NSUTF8StringEncoding ];
    Utf8String utf8((const UnsignedByte*)data.bytes, (unsigned int)data.length);
    ErrorCorrectionLevel qrLevel = [ self convertECLevel:level ];
    QRMatrixExtraMode* extraMode = [ self mapExtraMode:extra ];
    bool isMicro = (extraMode->mode == EncodingExtraMode::microQr);
    unsigned int segCount = 0;
    try {
        QRMatrixSegment* segments = utf8.unicodes().segments(qrLevel, &segCount, isMicro);
        QRMatrixBoard board = QRMatrixEncoder::encode(segments, segCount, qrLevel, *extraMode);
        delete extraMode;
        NSImage *result = [ self imageFromBoard:board scale:scale isMicro:isMicro ];
        delete[] segments;
        return result;
    } catch (QRMatrixException exception) {
        *error = [ NSError errorWithDomain:@"QRMatrixLib"
                                      code:3
                                  userInfo:@{ NSLocalizedDescriptionKey: [ NSString stringWithCString:exception.description.data()
                                                                                             encoding:NSUTF8StringEncoding ]}];

        return nil;
    }
}

+( NSArray<NSImage*>* )makeStructuredAppendQR:( NSArray<QRStructuredAppend*>* )parts scale:( NSUInteger )scale error:( NSError** )error {
    QRMatrixStructuredAppend* qrParts = new QRMatrixStructuredAppend [parts.count];
    for (NSInteger index = 0; index < parts.count; index += 1) {
        QRStructuredAppend* part = parts[index];
        QRMatrixSegment* segs = new QRMatrixSegment [part.segments.count];
        for (NSInteger jndex = 0; jndex < part.segments.count; jndex += 1) {
            QRSegment* originSeg = part.segments[jndex];
            segs[jndex] = QRMatrixSegment([ self mapMode:originSeg.mode ], (UnsignedByte*)originSeg.data.bytes, (unsigned int)originSeg.data.length, (unsigned int)originSeg.eci);
        }
        qrParts[index] = QRMatrixStructuredAppend(segs, (unsigned int)part.segments.count, [self convertECLevel:part.ecLevel]);
        QRMatrixExtraMode* extra = [ self mapExtraMode:part.extra ];
        qrParts[index].extraMode = *extra;
        delete extra;
        delete[] segs;
    }
    try {
        QRMatrixBoard* boards = QRMatrixEncoder::encode(qrParts, (unsigned int)parts.count);
        NSMutableArray<NSImage*>* result = [ NSMutableArray new ];
        for (unsigned int index = 0; index < parts.count; index += 1) {
            [ result addObject:[ self imageFromBoard:boards[index] scale:scale isMicro:NO ]];
        }
        delete[] qrParts;
        return result;
    } catch (QRMatrixException exception) {
        delete[] qrParts;
        *error = [ NSError errorWithDomain:@"QRMatrixLib"
                                      code:3
                                  userInfo:@{ NSLocalizedDescriptionKey: [ NSString stringWithCString:exception.description.data()
                                                                                             encoding:NSUTF8StringEncoding ]}];

        return nil;
    }
    return nil;
}

@end
