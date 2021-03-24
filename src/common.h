/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2021 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2021 GAMS Development Corp. <support@gams.com>
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

#include <QHashFunctions>
#include <QObject>
#include <QTextStream>
#include <QMetaEnum>
#include <QMetaType>

namespace gams {
namespace studio {

const QString InvalidGAMS = "The GAMS system you use is not compatible with your version of GAMS Studio. Please use GAMS " GAMS_DISTRIB_VERSION_SHORT " or higher.";

template <typename PHANTOM_TYPE>
class PhantomInt
{
    int mValue;
public:
    PhantomInt (int value = -1) : mValue(value) { }
    PhantomInt (const PhantomInt<PHANTOM_TYPE> &value) : mValue(value.mValue) { }
    virtual ~PhantomInt() {}
    inline operator int() const {return mValue;}
    constexpr PhantomInt<PHANTOM_TYPE>& operator=(const PhantomInt<PHANTOM_TYPE>& other) {
        mValue = other.mValue;
        return *this;
    }
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
template <typename PHANTOM_TYPE>
unsigned int qHash(PhantomInt<PHANTOM_TYPE> key, unsigned int seed) {
    return ::qHash(static_cast<unsigned int>(key), seed);
}

struct PiFileId {};
struct PiNodeId {};
struct PiTextMarkId {};

typedef PhantomInt<PiFileId> FileId;
typedef PhantomInt<PiNodeId> NodeId;
typedef PhantomInt<PiTextMarkId> TextMarkId;

Q_NAMESPACE

enum class NameModifier {
    raw,
    editState
};
Q_ENUM_NS(NameModifier)

enum class NodeType {
    root,
    group,
    runGroup,
    file,
    log
};
Q_ENUM_NS(NodeType)

enum class FileKind {
    None,
    Gsp,
    Gms,
    Txt,
    TxtRO,
    Lst,
    Lxi,
    Log,
    Gdx,
    Ref,
    Opt,
    Guc
};
Q_ENUM_NS(FileKind)

inline unsigned int qHash(FileKind key, unsigned int seed)
{
    return ::qHash(static_cast<unsigned int>(key), seed);
}

enum class EditorType {
    undefined = 0,
    source = 1,
    log = 2,
    syslog = 3,
    txt = 4,
    txtRo = 5,
    lxiLst = 6,
    gdx = 7,
    ref = 8,
    opt = 9,
    gucfg = 10
};
Q_ENUM_NS(EditorType)

enum class FileProcessKind {
    ignore,
    changedExternOnly,
    changedConflict,
    removedExtern,
    fileLocked,
    fileBecameInvalid,
};
Q_ENUM_NS(FileProcessKind)

enum class FileEventKind {
    invalid,
    changed,
    closed,
    created,
    changedExtern,
    removedExtern,  // removed-event is delayed to improve recognition of moved- or rewritten-events
};
Q_ENUM_NS(FileEventKind)

enum ProcState {
    ProcCheck,
    ProcIdle,
    Proc1Compile,
    Proc2Pack,
    Proc3Queued,
    Proc4Monitor,
    Proc5GetResult,
    Proc6Unpack,
};

enum NewTabStrategy {
    tabAtStart,
    tabBeforeCurrent,
    tabAfterCurrent,
    tabAtEnd
};

template <typename T>
typename QtPrivate::QEnableIf<QtPrivate::IsQEnumHelper<T>::Value , QTextStream&>::Type
operator<<(QTextStream &dbg, T enumValue)
{
    const QMetaObject *mo = qt_getEnumMetaObject(enumValue);
    int enumIdx = mo->indexOfEnumerator(qt_getEnumName(enumValue));
    return dbg << mo->enumerator(enumIdx).valueToKey(int(enumValue));
}

const int MAX_SEARCH_RESULTS = 50000;
const double TABLE_ROW_HEIGHT = 1.6;

enum ProcessExitCode {
    ecTooManyScratchDirs = 110,
    ecNeosExitWithErrors = 111,
};


}
}

Q_DECLARE_METATYPE(gams::studio::FileId);
Q_DECLARE_METATYPE(gams::studio::NodeId);
Q_DECLARE_METATYPE(gams::studio::TextMarkId);

#endif // COMMON_H
