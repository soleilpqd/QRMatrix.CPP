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

#include "polynomial.h"
#include "../Exception/qrmatrixexception.h"
#include "../common.h"

using namespace QRMatrix;

UnsignedByte Polynomial_Exp[256];
UnsignedByte Polynomial_Log[256];

bool initTables = []() {
    static const unsigned int prim = 0x11D;
    unsigned int xVal = 1;
    for (unsigned int index = 0; index < 255; index += 1) {
        Polynomial_Exp[index] = (UnsignedByte)xVal;
        Polynomial_Log[xVal] = (UnsignedByte)index;
        xVal <<= 1;
        if (xVal >= 256) {
            xVal ^= prim;
        }
    }
    return true;
} ();

UnsignedByte Polynomial_Multiple(UnsignedByte left, UnsignedByte right) {
    if (left == 0 || right == 0) {
        return 0;
    }
    return Polynomial_Exp[(Polynomial_Log[left] + Polynomial_Log[right]) % 255];
}

UnsignedByte Polynomial_Power(UnsignedByte value, UnsignedByte power) {
    return Polynomial_Exp[(Polynomial_Log[value] * power) % 255];
}

Polynomial Polynomial_PolyMultiple(Polynomial self, Polynomial other) {
    Polynomial result(self.length + other.length - 1);
    for (unsigned int jndex = 0; jndex < other.length; jndex += 1) {
        for (unsigned int index = 0; index < self.length; index += 1) {
            result.terms[index + jndex] ^= Polynomial_Multiple(self.terms[index], other.terms[jndex]);
        }
    }
    return result;
}

Polynomial Polynomial_getGeneratorPoly(unsigned int count) {
    Polynomial result(1);
    result.terms[0] = 1;
    for (unsigned int index = 0; index < count; index += 1) {
        Polynomial arg(2);
        arg.terms[0] = 1;
        arg.terms[1] = Polynomial_Power(2, index);
        result = Polynomial_PolyMultiple(result, arg);
    }
    return result;
}

Polynomial Polynomial::getErrorCorrections(unsigned int count) {
    if (length + count > 255) {
        throw QR_EXCEPTION("Internal error: invalid message length to calculate Error Corrections");
    }
    Polynomial gen = Polynomial_getGeneratorPoly(count);
    Polynomial buffer(length + gen.length - 1);
    for (unsigned int index = 0; index < length; index += 1) {
        buffer.terms[index] = terms[index];
    }
    for (unsigned int index = 0; index < length; index += 1) {
        UnsignedByte coef = buffer.terms[index];
        if (coef != 0) {
            for (unsigned int jndex = 1; jndex < gen.length; jndex += 1) {
                buffer.terms[index + jndex] ^= Polynomial_Multiple(gen.terms[jndex], coef);
            }
        }
    }
    Polynomial result(buffer.length - length);
    for (unsigned int index = length; index < buffer.length; index += 1) {
        result.terms[index - length] = buffer.terms[index];
    }
    return result;
}

// --------------------------------------------------------------------------------------------

Polynomial::~Polynomial() {
    delete[] terms;
}

Polynomial::Polynomial(Polynomial &other) {
    length = other.length;
    terms = new UnsignedByte [length];
    for (unsigned int index = 0; index < length; index += 1) {
        terms[index] = other.terms[index];
    }
}

Polynomial::Polynomial(unsigned int count) {
    length = count;
    terms = Common::allocate(count);
}

void Polynomial::operator=(Polynomial other) {
    delete[] terms;
    length = other.length;
    terms = new UnsignedByte [length];
    for (unsigned int index = 0; index < length; index += 1) {
        terms[index] = other.terms[index];
    }
}
