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

class BlockCode
{
    static const int b1 = 65536;
    static const int b2 = 512;
    static const int b3 = 64;
    int mCode;
public:
    BlockCode(int _code = -1) { operator=(_code);}
    bool isValid() { return mCode >= 0;}
    int code() const { return mCode; }
    int kind() const { return mCode % b1; }
    int depth() const { return (mCode / b1) % b2; } // up to one element on the kind-stack may have nesting
    int parser() const { return mCode / (b1 * b2); } // parser==0 is GAMS syntax, others like python may be added

    void operator =(int _code) { mCode = (_code < 0) ? -1 : _code; }
    bool setKind(int _kind) {
        int val = qBound(0, _kind, b1-1);
        mCode = mCode - kind() + val;
        return val == _kind;
    }
    bool setDepth(int _depth) {
        int val = qBound(0, _depth, b2-1);
        mCode = mCode - (depth() * b1) + (val * b1);
        return val == _depth;
    }
    bool setParser(int _parser) {
        int val = qBound(0, _parser, b3-1);
        mCode = mCode - (parser() * b1 * b2) + (val * b1 * b2);
        return val == _parser;
    }
};


} // namespace syntax
} // namespace studio
} // namespace gans

#endif // BLOCKCODE_H
