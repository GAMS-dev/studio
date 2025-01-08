/**
 * GAMS Studio
 *
 * Copyright (c) 2017-2025 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2025 GAMS Development Corp. <support@gams.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef BLOCKCODE_H
#define BLOCKCODE_H

#include "logger.h"
#include "syntaxformats.h"

namespace gams {
namespace studio {
namespace syntax {

///
/// \brief The BlockCode class
///
class BlockCode
{
    // in sum the bounds must not use more than 31 bits (to fit in positive integer)
    static const int b1 = 512;      // [ 9 bits] kind bound
    static const int b2 = 512;      // [ 9 bits] flavor (sub-kind) bound
    static const int b3 = 1024;     // [10 bits] nesting-depth bound (extended info if depth==1023)
    static const int b4 = 8;        // [ 3 bits] parser-type bound
    // bits: 4443333333333222222222111111111

    int mCode;
public:
    BlockCode(SyntaxKind _kind, int _flavor) : mCode(0) { // need to be extended when using depth or parser
        setKind(_kind);
        setFlavor(_flavor);
    }
    BlockCode(int _code = -1) {
        operator=(_code);
    }
    bool isValid() { return mCode >= 0;}
    int code() const { return mCode; }
    SyntaxKind kind() const { return static_cast<SyntaxKind>(mCode % b1); } // index in the stack to the kind
    int flavor() const { return (mCode / b1) % b2; }      // the flavor allows to separate kinds that differ slightly
    int depth() const { return (mCode / b1 / b2) % b3; }  // up to one element on the kind-stack may have nesting
    bool externDepth() const { return depth() == b3-1; }
    int parser() const { return (mCode / b1 / b2 / b3) % b4; } // parser==0 is GAMS syntax, others like python may be added

    bool operator !=(BlockCode other) const {
        return mCode != other.mCode;
    }
    bool operator ==(BlockCode other) const {
        return mCode == other.mCode;
    }
    void operator =(int _code) {
        mCode = (_code < 0) ? -1 : _code;
    }
    bool setKind(SyntaxKind _kind) {
        int val = qBound(0, int(_kind), b1-1);
        mCode = mCode + (val - int(kind()));
        return val == int(_kind);
    }
    bool setFlavor(int _flavor) {
        int val = qBound(0, _flavor, b2-1);
        mCode = mCode + ((val - flavor()) * b1);
        return val == _flavor;
    }
    bool setDepth(int _depth) {
        int val = qBound(0, _depth, b3-1);
        mCode = mCode + ((val - depth()) * b1*b2);
        return val == _depth;
    }
    bool setParser(int _parser) {
        int val = qBound(0, _parser, b4-1);
        mCode = mCode + ((val - parser()) * b1*b2*b3);
        return val == _parser;
    }

    static qint64 maxValue() {
        return qint64(b1)*b2*b3*b4 - 1;
    }
};


} // namespace syntax
} // namespace studio
} // namespace gans

#endif // BLOCKCODE_H
