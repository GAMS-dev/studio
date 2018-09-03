/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2018 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2018 GAMS Development Corp. <support@gams.com>
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

#ifndef COMMON_H
#define COMMON_H

namespace gams {
namespace studio {


template <typename PHANTOM_TYPE>
class PhantomInt
{
    int mValue;
public:
    PhantomInt (int value = -1) : mValue(value) { }
    inline operator int() const {return mValue;}
    inline bool isValid() const {return mValue>=0;}
    inline PhantomInt<PHANTOM_TYPE>& operator++() {
        mValue++;
        return *this;
    }
    inline PhantomInt<PHANTOM_TYPE> operator++(int) {
        int prev = mValue;
        operator++();
        return PhantomInt(prev);
    }
};

struct PiFileId {};
struct PiNodeId {};
struct PiTextMarkId {};

typedef PhantomInt<PiFileId> FileId;
typedef PhantomInt<PiNodeId> NodeId;
typedef PhantomInt<PiTextMarkId> TextMarkId;


enum struct NameModifier {
    raw,
    editState
};
enum struct NodeType {
    root,
    group,
    runGroup,
    file,
    log
};
enum struct FileKind {
    None,
    Gsp,
    Gms,
    Txt,
    Lst,
    Lxi,
    Log,
    Gdx,
    Ref,
};
enum struct EditorType {
    undefined = 0,
    source = 1,
    log = 2,
    lxiLst = 5,
    gdx = 6,
    ref = 7,
};



}
}

#endif // COMMON_H
