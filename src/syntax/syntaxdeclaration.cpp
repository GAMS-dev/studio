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
        delete mKeywords.take(mKeywords.keys().first());
}

SyntaxBlock SyntaxKeywordBase::validTail(const QString &line, int index, bool &hasContent)
{
    hasContent = false;
    int end = index;
    while (isWhitechar(line, end)) end++;
    return SyntaxBlock(this, index, end, SyntaxStateShift::shift);
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

int SyntaxKeywordBase::findEnd(SyntaxState state, const QString& line, int index)
{
    int iKey = 0;
    int iChar = 0;
    while (true) {
        const DictEntry *dEntry = &mKeywords.value(state)->at(iKey);
        if (iChar+index >= line.length() || !isKeywordChar(line.at(iChar+index))) {
            if (dEntry->length() > iChar) return -1;
            return iChar+index; // reached an valid end
        } else if (iChar < dEntry->length() &&  dEntry->is(line.at(iChar+index), iChar) ) {
            // character equals
            iChar++;
        } else {
            // different character  at iChar: switch to next keyword
            iKey++;
            if (iKey >= mKeywords.value(state)->count()) break; // no more keywords
            // next keyword starts with less equal characters than already matched
            if (mKeywords.value(state)->equalToPrevious(iKey) < iChar) break;
        }
    }
    return -1;
}


SyntaxDeclaration::SyntaxDeclaration() : SyntaxKeywordBase(SyntaxState::Declaration)
{
    QList<QPair<QString, QString>> list;
    list = SyntaxData::declaration();
    mKeywords.insert(state(), new DictList(list));

    list = QList<QPair<QString, QString>> {{"Set", ""}, {"Sets", ""}};
    mKeywords.insert(SyntaxState::DeclarationSetType, new DictList(list));

    list = QList<QPair<QString, QString>> {{"Variable", ""}, {"Variables", ""}};
    mKeywords.insert(SyntaxState::DeclarationVariableType, new DictList(list));
    mSubStates << SyntaxState::Directive << SyntaxState::CommentLine << SyntaxState::CommentEndline
               << SyntaxState::CommentInline << SyntaxState::Identifier;
}

SyntaxBlock SyntaxDeclaration::find(SyntaxState entryState, const QString& line, int index)
{
    int start = index;
    int end = -1;
    while (isWhitechar(line, start))
        ++start;
    if (entryState == SyntaxState::DeclarationSetType || entryState == SyntaxState::DeclarationVariableType) {

        // search for state-valid declaration keyword
        end = findEnd(entryState, line, start);
        if (end > start) return SyntaxBlock(this, start, end, SyntaxStateShift::shift);

        // search for invalid new declaration keyword
        end = findEnd(state(), line, start);
        if (end > start) return SyntaxBlock(this, start, end, true, SyntaxStateShift::reset);
        return SyntaxBlock(this);
    }

    end = findEnd(state(), line, start);
    if (end > start) {
        if (entryState == SyntaxState::Declaration || entryState == SyntaxState::DeclarationTable)
            return SyntaxBlock(this, start, end, true, SyntaxStateShift::reset);
        return SyntaxBlock(this, start, end, false, SyntaxStateShift::in, state());
    }
    return SyntaxBlock(this);
}

SyntaxPreDeclaration::SyntaxPreDeclaration(SyntaxState state) : SyntaxKeywordBase(state)
{
    QList<QPair<QString, QString>> list;
    switch (state) {
    case SyntaxState::DeclarationSetType:
        list = SyntaxData::declaration4Set();
        mKeywords.insert(state, new DictList(list));
        mSubStates << SyntaxState::Declaration;
        break;
    case SyntaxState::DeclarationVariableType:
        list = SyntaxData::declaration4Var();
        mKeywords.insert(state, new DictList(list));
        mSubStates << SyntaxState::Declaration;
        break;
    default:
        FATAL() << "invalid SyntaxState to initialize SyntaxDeclaration";
    }
    mSubStates << SyntaxState::Directive << SyntaxState::CommentLine << SyntaxState::CommentEndline
               << SyntaxState::CommentInline;

}

SyntaxBlock SyntaxPreDeclaration::find(SyntaxState entryState, const QString &line, int index)
{
    int start = index;
    int end = -1;
    while (isWhitechar(line, start))
        ++start;
    end = findEnd(state(), line, start);
    if (end > start) {
        if (entryState == SyntaxState::DeclarationSetType || entryState == SyntaxState::DeclarationVariableType
                || entryState == SyntaxState::Declaration || entryState == SyntaxState::DeclarationTable)
            return SyntaxBlock(this, start, end, true, SyntaxStateShift::reset);
        return SyntaxBlock(this, start, end,  false, SyntaxStateShift::in, state());
    } else if (entryState == state()) {
        return SyntaxBlock(this, index, start, SyntaxStateShift::shift);
    }
    return SyntaxBlock(this);
}

SyntaxDeclarationTable::SyntaxDeclarationTable() : SyntaxKeywordBase(SyntaxState::DeclarationTable)
{
    QList<QPair<QString, QString>> list;
    list = SyntaxData::declarationTable();
    mKeywords.insert(state(), new DictList(list));
    mSubStates << SyntaxState::IdentifierTable << SyntaxState::Directive
               << SyntaxState::CommentLine << SyntaxState::CommentEndline << SyntaxState::CommentInline;
}

SyntaxBlock SyntaxDeclarationTable::find(SyntaxState entryState, const QString &line, int index)
{
    int start = index;
    int end = -1;
    while (isWhitechar(line, start))
        ++start;
    end = findEnd(state(), line, start);
    if (end > start) {
        if (entryState == SyntaxState::DeclarationSetType || entryState == SyntaxState::DeclarationVariableType
                || entryState == SyntaxState::Declaration || entryState == SyntaxState::DeclarationTable)
            return SyntaxBlock(this, start, end, true, SyntaxStateShift::reset);
        return SyntaxBlock(this, start, end, false, SyntaxStateShift::in, state());
    }
    return SyntaxBlock(this);
}

SyntaxReserved::SyntaxReserved() : SyntaxKeywordBase(SyntaxState::Reserved)
{
    QList<QPair<QString, QString>> list;
    list = SyntaxData::reserved();
    mKeywords.insert(state(), new DictList(list));
    mSubStates << SyntaxState::Semicolon << SyntaxState::Embedded << SyntaxState::Reserved
               << SyntaxState::Directive << SyntaxState::Declaration << SyntaxState::DeclarationSetType
               << SyntaxState::DeclarationVariableType << SyntaxState::DeclarationTable << SyntaxState::ReservedBody
               << SyntaxState::CommentLine << SyntaxState::CommentEndline << SyntaxState::CommentInline;
}

SyntaxBlock SyntaxReserved::find(SyntaxState entryState, const QString &line, int index)
{
    Q_UNUSED(entryState);
    int start = index;
    int end = -1;
    while (isWhitechar(line, start))
        ++start;
    end = findEnd(state(), line, start);
    if (end > start) return SyntaxBlock(this, start, end, false, SyntaxStateShift::in, state());
    return SyntaxBlock(this);
}

SyntaxReservedBody::SyntaxReservedBody() : SyntaxAbstract(SyntaxState::ReservedBody)
{
    mSubStates << SyntaxState::Embedded << SyntaxState::Reserved << SyntaxState::Semicolon << SyntaxState::Directive
               << SyntaxState::Declaration << SyntaxState::DeclarationSetType << SyntaxState::DeclarationVariableType
               << SyntaxState::DeclarationTable
               << SyntaxState::CommentLine << SyntaxState::CommentEndline << SyntaxState::CommentInline;
}

SyntaxBlock SyntaxReservedBody::find(SyntaxState entryState, const QString &line, int index)
{
    Q_UNUSED(entryState);
    int end = line.length();
//    while (isWhitechar(line, end))
//        ++end;
//    if (index == 0) return SyntaxBlock(this, index, end, SyntaxStateShift::shift);
//    if (end < line.length()) {
//        if (line.at(end)=='(')
//            return SyntaxBlock(this, index, end+1, SyntaxStateShift::out);
//        end++;
//    }
    return SyntaxBlock(this, index, end, SyntaxStateShift::shift);
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
        return SyntaxBlock(this, index, end, SyntaxStateShift::out);
    return SyntaxBlock(this, index, end, SyntaxStateShift::shift);
}

SyntaxEmbedded::SyntaxEmbedded(SyntaxState state) : SyntaxKeywordBase(state)
{
    QList<QPair<QString, QString>> list;
    if (state == SyntaxState::Embedded) {
        list = SyntaxData::embedded();
        mSubStates << SyntaxState::EmbeddedBody;
    } else {
        list = SyntaxData::embeddedEnd();
    }
    mKeywords.insert(state, new DictList(list));
}

SyntaxBlock SyntaxEmbedded::find(SyntaxState entryState, const QString &line, int index)
{
    Q_UNUSED(entryState);
    int start = index;
    int end = -1;
    while (isWhitechar(line, start))
        ++start;
    end = findEnd(state(), line, start);
    if (end > start) {
        SyntaxStateShift stateShift = (state() == SyntaxState::Embedded) ? SyntaxStateShift::in : SyntaxStateShift::out;
        return SyntaxBlock(this, start, end, false, stateShift, state());
    }
    return SyntaxBlock(this);
}

SyntaxEmbeddedBody::SyntaxEmbeddedBody() : SyntaxAbstract(SyntaxState::EmbeddedBody)
{
    mSubStates << SyntaxState::EmbeddedEnd;
}

SyntaxBlock SyntaxEmbeddedBody::find(SyntaxState entryState, const QString &line, int index)
{
    Q_UNUSED(entryState);
    return SyntaxBlock(this, index, line.length());
}

SyntaxBlock SyntaxEmbeddedBody::validTail(const QString &line, int index, bool &hasContent)
{
    Q_UNUSED(hasContent);
    return SyntaxBlock(this, index, line.length());
}


} // namespace studio
} // namespace gans
