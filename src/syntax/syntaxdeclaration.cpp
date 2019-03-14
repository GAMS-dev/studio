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
#include "syntaxdeclaration.h"
#include "logger.h"
#include "exception.h"
#include "syntaxdata.h"

namespace gams {
namespace studio {
namespace syntax {

bool cmpStr(const QPair<QString, QString>& lhs,const QPair<QString, QString>& rhs)
{
    return lhs.first.compare(rhs.first, Qt::CaseInsensitive) < 0;
}

DictList::DictList(QList<QPair<QString, QString> > list) : mEntries(QVector<DictEntry*>(list.size())), mEqualStart(QVector<int>(list.size()))
{
    std::sort(list.begin(), list.end(), cmpStr);
    QString prevS;
    for (int i = 0; i < list.size(); ++i) {
        const QString &s(list.at(i).first);
        mEntries[i] = new DictEntry(s);
        mEqualStart[i] = equalStart(s, prevS);
        prevS = s;
    }
}

SyntaxKeywordBase::~SyntaxKeywordBase()
{
    while (!mKeywords.isEmpty())
        delete mKeywords.take(int(mKeywords.keys().first()));
}

SyntaxBlock SyntaxKeywordBase::validTail(const QString &line, int index, bool &hasContent)
{
    hasContent = false;
    int end = index;
    while (isWhitechar(line, end)) end++;
    return SyntaxBlock(this, index, end, SyntaxShift::shift);
}

QStringList SyntaxKeywordBase::swapStringCase(QStringList list)
{
    QStringList res;
    for (QString s: list) {
        QString swapped("");
        for (QChar c: s) {
            swapped += (c.isUpper() ? c.toLower() : c.toUpper());
        }
        res << swapped;
    }
    return res;
}

int SyntaxKeywordBase::findEnd(SyntaxKind kind, const QString& line, int index, int &iKey)
{
    iKey = 0;
    int iChar = 0;
    while (true) {
        const DictEntry *dEntry = &mKeywords.value(int(kind))->at(iKey);
        if (iChar+index >= line.length() || !isKeywordChar(line.at(iChar+index))) {
            if (dEntry->length() > iChar) return -1;
            return iChar+index; // reached an valid end
        } else if (iChar < dEntry->length() &&  dEntry->is(line.at(iChar+index), iChar) ) {
            // character equals
            iChar++;
        } else {
            // different character  at iChar: switch to next keyword
            iKey++;
            if (iKey >= mKeywords.value(int(kind))->count()) break; // no more keywords
            // next keyword starts with less equal characters than already matched
            if (mKeywords.value(int(kind))->equalToPrevious(iKey) < iChar) break;
        }
    }
    return -1;
}


SyntaxDeclaration::SyntaxDeclaration() : SyntaxKeywordBase(SyntaxKind::Declaration)
{
    QList<QPair<QString, QString>> list;
    list = SyntaxData::declaration();
    mKeywords.insert(int(kind()), new DictList(list));

    list = QList<QPair<QString, QString>> {{"Set", ""}, {"Sets", ""}};
    mKeywords.insert(int(SyntaxKind::DeclarationSetType), new DictList(list));

    list = QList<QPair<QString, QString>> {{"Variable", ""}, {"Variables", ""}};
    mKeywords.insert(int(SyntaxKind::DeclarationVariableType), new DictList(list));
    mSubKinds << SyntaxKind::Directive << SyntaxKind::CommentLine << SyntaxKind::CommentEndline
               << SyntaxKind::CommentInline << SyntaxKind::DeclarationTable << SyntaxKind::Identifier;
}

SyntaxBlock SyntaxDeclaration::find(SyntaxKind entryKind, const QString& line, int index)
{
    int start = index;
    int end = -1;
    while (isWhitechar(line, start))
        ++start;
    int iKey;
    if (entryKind == SyntaxKind::DeclarationSetType || entryKind == SyntaxKind::DeclarationVariableType) {

        // search for kind-valid declaration keyword
        end = findEnd(entryKind, line, start, iKey);
        if (end > start) return SyntaxBlock(this, start, end, SyntaxShift::shift);

        // search for invalid new declaration keyword
        end = findEnd(kind(), line, start, iKey);
        if (end > start) return SyntaxBlock(this, start, end, true, SyntaxShift::reset);
        return SyntaxBlock(this);
    }

    end = findEnd(kind(), line, start, iKey);
    if (end > start) {
        if (entryKind == SyntaxKind::Declaration || entryKind == SyntaxKind::DeclarationTable)
            return SyntaxBlock(this, start, end, true, SyntaxShift::reset);
        return SyntaxBlock(this, start, end, false, SyntaxShift::in, kind());
    }
    return SyntaxBlock(this);
}

SyntaxPreDeclaration::SyntaxPreDeclaration(SyntaxKind kind) : SyntaxKeywordBase(kind)
{
    QList<QPair<QString, QString>> list;
    switch (kind) {
    case SyntaxKind::DeclarationSetType:
        list = SyntaxData::declaration4Set();
        mKeywords.insert(int(kind), new DictList(list));
        mSubKinds << SyntaxKind::Declaration;
        break;
    case SyntaxKind::DeclarationVariableType:
        list = SyntaxData::declaration4Var();
        mKeywords.insert(int(kind), new DictList(list));
        mSubKinds << SyntaxKind::Declaration;
        break;
    default:
        FATAL() << "invalid SyntaxKind to initialize SyntaxDeclaration";
    }
    mSubKinds << SyntaxKind::Directive << SyntaxKind::CommentLine << SyntaxKind::CommentEndline
               << SyntaxKind::CommentInline;

}

SyntaxBlock SyntaxPreDeclaration::find(SyntaxKind entryKind, const QString &line, int index)
{
    int start = index;
    int end = -1;
    while (isWhitechar(line, start))
        ++start;
    int iKey;
    end = findEnd(kind(), line, start, iKey);
    if (end > start) {
        if (entryKind == SyntaxKind::DeclarationSetType || entryKind == SyntaxKind::DeclarationVariableType
                || entryKind == SyntaxKind::Declaration || entryKind == SyntaxKind::DeclarationTable)
            return SyntaxBlock(this, start, end, true, SyntaxShift::reset);
        return SyntaxBlock(this, start, end,  false, SyntaxShift::in, kind());
    } else if (entryKind == kind()) {
        return SyntaxBlock(this, index, start, SyntaxShift::shift);
    }
    return SyntaxBlock(this);
}

SyntaxDeclarationTable::SyntaxDeclarationTable() : SyntaxKeywordBase(SyntaxKind::DeclarationTable)
{
    QList<QPair<QString, QString>> list;
    list = SyntaxData::declarationTable();
    mKeywords.insert(int(kind()), new DictList(list));
    mSubKinds << SyntaxKind::IdentifierTable << SyntaxKind::Directive
               << SyntaxKind::CommentLine << SyntaxKind::CommentEndline << SyntaxKind::CommentInline;
}

SyntaxBlock SyntaxDeclarationTable::find(SyntaxKind entryKind, const QString &line, int index)
{
    int start = index;
    int end = -1;
    while (isWhitechar(line, start))
        ++start;
    int iKey;
    end = findEnd(kind(), line, start, iKey);
    if (end > start) {
        if (entryKind == SyntaxKind::DeclarationSetType || entryKind == SyntaxKind::DeclarationVariableType
                || entryKind == SyntaxKind::DeclarationTable)
            return SyntaxBlock(this, start, end, true, SyntaxShift::reset);
        return SyntaxBlock(this, start, end, false, SyntaxShift::in, kind());
    }
    return SyntaxBlock(this);
}

SyntaxReserved::SyntaxReserved(SyntaxKind kind) : SyntaxKeywordBase(kind)
{
    mSubKinds << SyntaxKind::Semicolon << SyntaxKind::String << SyntaxKind::Embedded << SyntaxKind::Solve
              << SyntaxKind::Reserved << SyntaxKind::CommentLine << SyntaxKind::CommentEndline
              << SyntaxKind::CommentInline << SyntaxKind::Directive << SyntaxKind::Declaration
              << SyntaxKind::DeclarationSetType << SyntaxKind::DeclarationVariableType << SyntaxKind::DeclarationTable;
    QList<QPair<QString, QString>> list;
    switch (kind) {
    case SyntaxKind::Reserved:
        list = SyntaxData::reserved();
        mSubKinds << SyntaxKind::Formula;
        break;
    case SyntaxKind::Solve:
        list = QList<QPair<QString, QString>> {{"solve",""}};
        mSubKinds << SyntaxKind::SolveKey << SyntaxKind::SolveBody;
        break;
    default:
        Q_ASSERT_X(false, "SyntaxReserved", ("Invalid SyntaxKind: "+syntaxKindName(kind)).toLatin1());
    }
    mKeywords.insert(int(kind), new DictList(list));
}

SyntaxBlock SyntaxReserved::find(SyntaxKind entryKind, const QString &line, int index)
{
    Q_UNUSED(entryKind);
    int start = index;
    int end = -1;
    while (isWhitechar(line, start))
        ++start;
    int iKey;
    end = findEnd(kind(), line, start, iKey);
    if (end > start) {
        return SyntaxBlock(this, start, end, false, SyntaxShift::in, kind());
    }
    return SyntaxBlock(this);
}


SyntaxEmbedded::SyntaxEmbedded(SyntaxKind kind) : SyntaxKeywordBase(kind)
{
    QList<QPair<QString, QString>> list;
    if (kind == SyntaxKind::Embedded) {
        list = SyntaxData::embedded();
        mSubKinds << SyntaxKind::EmbeddedBody;
    } else {
        list = SyntaxData::embeddedEnd();
    }
    mKeywords.insert(int(kind), new DictList(list));
}

SyntaxBlock SyntaxEmbedded::find(SyntaxKind entryKind, const QString &line, int index)
{
    Q_UNUSED(entryKind);
    int start = index;
    while (isWhitechar(line, start))
        ++start;
    int end = -1;
    int iKey;
    end = findEnd(kind(), line, start, iKey);
    if (end > start) {
        SyntaxShift kindShift = (kind() == SyntaxKind::Embedded) ? SyntaxShift::in : SyntaxShift::out;
        return SyntaxBlock(this, start, end, false, kindShift, kind());
    }
    return SyntaxBlock(this);
}

SyntaxEmbeddedBody::SyntaxEmbeddedBody() : SyntaxAbstract(SyntaxKind::EmbeddedBody)
{
    mSubKinds << SyntaxKind::EmbeddedEnd << SyntaxKind::Directive;
}

SyntaxBlock SyntaxEmbeddedBody::find(SyntaxKind entryKind, const QString &line, int index)
{
    Q_UNUSED(entryKind);
    return SyntaxBlock(this, index, line.length());
}

SyntaxBlock SyntaxEmbeddedBody::validTail(const QString &line, int index, bool &hasContent)
{
    Q_UNUSED(hasContent);
    return SyntaxBlock(this, index, line.length());
}

SyntaxSolveKey::SyntaxSolveKey() : SyntaxKeywordBase(SyntaxKind::SolveKey)
{
    QList<QPair<QString, QString>> list;
    list = SyntaxData::modelType();
    mLastModelType = list.length()-3;
    mKeywords.insert(int(kind()), new DictList(list));
    mSubKinds << SyntaxKind::Semicolon << SyntaxKind::Directive << SyntaxKind::CommentLine
              << SyntaxKind::CommentEndline << SyntaxKind::CommentInline;
    mSubKinds << SyntaxKind::SolveKey << SyntaxKind::SolveBody;
}

int canBreak(QChar ch, int &prev) {
    // ASCII:   "   $   '   .   0  9   ;   =   A  Z   _   a   z
    // Code:   34, 36, 39, 46, 48-57, 59, 61, 65-90, 95, 97-122
    static QVector<QChar> cList = {'"', '$', '\'', '.', ';', '='};  // other breaking kind

    if (ch < '"' || ch > 'z')
        prev = 0;
    else if (ch >= 'a' || (ch >= 'A' && ch <= 'Z') || ch == '_')
        prev = 2;  // break by keyword kind
    else if (ch >= '0' && ch <= '9') {
        if (prev!=2) prev = 0;
    } else prev = cList.contains(ch) ? 1 : 0;
    return prev;
}


SyntaxBlock SyntaxSolveKey::find(SyntaxKind entryKind, const QString &line, int index)
{
    Q_UNUSED(entryKind);
    int start = index;
    while (isWhitechar(line, start))
        ++start;
    if (start < line.length()) return SyntaxBlock(this);
    int end = -1;
    int iKey;
    end = findEnd(kind(), line, start, iKey);
//    if (end < 0 && start+2 < line.length() && (line.at(start) == 'm' || line.at(start) == 'M')) {
//        if (line.mid(start,3).compare("min", Qt::CaseInsensitive)==0 ||
//                line.mid(start,3).compare("max", Qt::CaseInsensitive)==0) {
//            end = start+3;
//            int prev = 2;
//            while (end < line.length() && canBreak(line.at(end), prev) == 2) ++end;
//        }
//    }
    if (end > start) {
        return SyntaxBlock(this, start, end, false, SyntaxShift::shift, kind());
    }
    return SyntaxBlock(this);
}


} // namespace syntax
} // namespace studio
} // namespace gans
