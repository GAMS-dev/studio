/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2020 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2020 GAMS Development Corp. <support@gams.com>
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
 */
#ifndef BLOCKCODE_H
#define BLOCKCODE_H

#include "logger.h"

namespace gams {
namespace studio {
namespace syntax {

///
/// \brief The BlockCode class
///
class BlockCode
{
    // in sum the bounds must not use more than 31 bits (to fit in positive integer)
    static const int b1 = 16384;    // kind stack-index bound
    static const int b2 = 32;       // TODO(JM) new region for flavor (sub-kind) bound
    static const int b3 = 256;      // nesting-depth bound (extended info if depth==255)
    static const int b4 = 8;        // parser-type bound

    int mCode;
public:
    BlockCode(int _code = -1) {
        operator=(_code);
    }
    bool isValid() { return mCode >= 0;}
    int code() const { return mCode; }
    int kind() const { return mCode % b1; }               // index in the stack to the kind
    int flavor() const { return (mCode / b1) % b2; }      // the flavor allows to separate kinds that differ slightly
    int depth() const { return (mCode / b1 / b2) % b3; }  // up to one element on the kind-stack may have nesting
    bool externDepth() const { return depth() == b3-1; }
    int parser() const { return (mCode / b1 / b2 / b3); } // parser==0 is GAMS syntax, others like python may be added

    void operator =(int _code) {
        mCode = (_code < 0) ? -1 : _code;
    }
    bool setKind(int _kind) {
        int val = qBound(0, _kind, b1-1);
        mCode = mCode + (val - kind());
        return val == _kind;
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
};


} // namespace syntax
} // namespace studio
} // namespace gans

#endif // BLOCKCODE_H
