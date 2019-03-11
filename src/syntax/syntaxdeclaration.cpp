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

int SyntaxKeywordBase::findEnd(syntax::SyntaxKind kind, const QString& line, int index)
{
    int iKey = 0;
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

SyntaxBlock SyntaxDeclaration::find(syntax::SyntaxKind entryKind, const QString& line, int index)
{
    int start = index;
    int end = -1;
    while (isWhitechar(line, start))
        ++start;
    if (entryKind == SyntaxKind::DeclarationSetType || entryKind == SyntaxKind::DeclarationVariableType) {

        // search for kind-valid declaration keyword
        end = findEnd(entryKind, line, start);
        if (end > start) return SyntaxBlock(this, start, end, SyntaxShift::shift);

        // search for invalid new declaration keyword
        end = findEnd(kind(), line, start);
        if (end > start) return SyntaxBlock(this, start, end, true, SyntaxShift::reset);
        return SyntaxBlock(this);
    }

    end = findEnd(kind(), line, start);
    if (end > start) {
        if (entryKind == SyntaxKind::Declaration || entryKind == SyntaxKind::DeclarationTable)
            return SyntaxBlock(this, start, end, true, SyntaxShift::reset);
        return SyntaxBlock(this, start, end, false, SyntaxShift::in, kind());
    }
    return SyntaxBlock(this);
}

SyntaxPreDeclaration::SyntaxPreDeclaration(syntax::SyntaxKind kind) : SyntaxKeywordBase(kind)
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
        FATAL() << "invalid syntax::SyntaxKind to initialize SyntaxDeclaration";
    }
    mSubKinds << SyntaxKind::Directive << SyntaxKind::CommentLine << SyntaxKind::CommentEndline
               << SyntaxKind::CommentInline;

}

SyntaxBlock SyntaxPreDeclaration::find(syntax::SyntaxKind entryKind, const QString &line, int index)
{
    int start = index;
    int end = -1;
    while (isWhitechar(line, start))
        ++start;
    end = findEnd(kind(), line, start);
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

SyntaxBlock SyntaxDeclarationTable::find(syntax::SyntaxKind entryKind, const QString &line, int index)
{
    int start = index;
    int end = -1;
    while (isWhitechar(line, start))
        ++start;
    end = findEnd(kind(), line, start);
    if (end > start) {
        if (entryKind == SyntaxKind::DeclarationSetType || entryKind == SyntaxKind::DeclarationVariableType
                || entryKind == SyntaxKind::DeclarationTable)
            return SyntaxBlock(this, start, end, true, SyntaxShift::reset);
        return SyntaxBlock(this, start, end, false, SyntaxShift::in, kind());
    }
    return SyntaxBlock(this);
}

SyntaxReserved::SyntaxReserved() : SyntaxKeywordBase(SyntaxKind::Reserved)
{
    QList<QPair<QString, QString>> list;
    list = SyntaxData::reserved();
    mKeywords.insert(int(kind()), new DictList(list));
    mSubKinds << SyntaxKind::Semicolon << SyntaxKind::Embedded << SyntaxKind::Reserved
               << SyntaxKind::CommentLine << SyntaxKind::CommentEndline << SyntaxKind::CommentInline
               << SyntaxKind::Directive << SyntaxKind::Declaration << SyntaxKind::DeclarationSetType
               << SyntaxKind::DeclarationVariableType << SyntaxKind::DeclarationTable << SyntaxKind::ReservedBody;
}

SyntaxBlock SyntaxReserved::find(syntax::SyntaxKind entryKind, const QString &line, int index)
{
    Q_UNUSED(entryKind);
    int start = index;
    int end = -1;
    while (isWhitechar(line, start))
        ++start;
    end = findEnd(kind(), line, start);
    if (end > start) return SyntaxBlock(this, start, end, false, SyntaxShift::in, kind());
    return SyntaxBlock(this);
}

SyntaxReservedBody::SyntaxReservedBody() : SyntaxAbstract(SyntaxKind::ReservedBody)
{
    mSubKinds << SyntaxKind::Embedded << SyntaxKind::Reserved << SyntaxKind::Semicolon
               << SyntaxKind::CommentLine << SyntaxKind::CommentEndline << SyntaxKind::CommentInline
               << SyntaxKind::Directive << SyntaxKind::Declaration << SyntaxKind::DeclarationSetType
               << SyntaxKind::DeclarationVariableType << SyntaxKind::DeclarationTable;
}

SyntaxBlock SyntaxReservedBody::find(syntax::SyntaxKind entryKind, const QString &line, int index)
{
    Q_UNUSED(entryKind);
    int end = line.length();
//    while (isWhitechar(line, end))
//        ++end;
//    if (index == 0) return SyntaxBlock(this, index, end, SyntaxShift::shift);
//    if (end < line.length()) {
//        if (line.at(end)=='(')
//            return SyntaxBlock(this, index, end+1, SyntaxShift::out);
//        end++;
//    }
    return SyntaxBlock(this, index, end, SyntaxShift::shift);
}

SyntaxBlock SyntaxReservedBody::validTail(const QString &line, int index, bool &hasContent)
{
    int start = index;
    while (isWhitechar(line, start))
        ++start;
    int end = start;
    while (end<line.length() && line.at(end)!=';' && line.at(end)!='(') end++;
    if (end<line.length() && line.at(end)=='(') end++;
    hasContent = start < end;
    if (end < line.length() || index == 0)
        return SyntaxBlock(this, index, end, SyntaxShift::out);
    return SyntaxBlock(this, index, end, SyntaxShift::shift);
}

SyntaxEmbedded::SyntaxEmbedded(syntax::SyntaxKind kind) : SyntaxKeywordBase(kind)
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

SyntaxBlock SyntaxEmbedded::find(syntax::SyntaxKind entryKind, const QString &line, int index)
{
    Q_UNUSED(entryKind);
    int start = index;
    int end = -1;
    while (isWhitechar(line, start))
        ++start;
    end = findEnd(kind(), line, start);
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

SyntaxBlock SyntaxEmbeddedBody::find(syntax::SyntaxKind entryKind, const QString &line, int index)
{
    Q_UNUSED(entryKind);
    return SyntaxBlock(this, index, line.length());
}

SyntaxBlock SyntaxEmbeddedBody::validTail(const QString &line, int index, bool &hasContent)
{
    Q_UNUSED(hasContent);
    return SyntaxBlock(this, index, line.length());
}


} // namespace syntax
} // namespace studio
} // namespace gans
