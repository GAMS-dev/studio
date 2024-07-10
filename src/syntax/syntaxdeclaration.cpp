/**
 * GAMS Studio
 *
 * Copyright (c) 2017-2024 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2024 GAMS Development Corp. <support@gams.com>
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

DictList::DictList(QList<QPair<QString, QString> > list, const QString &prefix)
    : mEntries(QVector<DictEntry*>(list.size())), mEqualStart(QVector<int>(list.size()))
{
    std::sort(list.begin(), list.end(), cmpStr);
    QString prevS;
    mEntries.reserve(list.size());
    mEqualStart.reserve(list.size());
    for (int i = 0; i < list.size(); ++i) {
        const QString &s( prefix.isNull() ? list.at(i).first : prefix + list.at(i).first);
        mEntries[i] = new DictEntry(s);
        mDescript << list.at(i).second;
        mEqualStart[i] = equalStart(s, prevS);
        prevS = s;
    }
}

SyntaxKeywordBase::~SyntaxKeywordBase()
{
    while (!mKeywords.isEmpty())
        delete mKeywords.take(mKeywords.begin().key());
}

SyntaxBlock SyntaxKeywordBase::validTail(const QString &line, int index, SyntaxState state, bool &hasContent)
{
    hasContent = false;
    int end = index;
    while (isWhitechar(line, end)) end++;
    return SyntaxBlock(this, state, index, end, SyntaxShift::shift);
}

QStringList SyntaxKeywordBase::docForLastRequest() const
{
    QStringList res;
    if (mLastIKey >= 0 && mLastIKey < mKeywords.value(mLastKind)->count())
        res << mKeywords.value(mLastKind)->at(mLastIKey).entry() << mKeywords.value(mLastKind)->docAt(mLastIKey);
    return res;
}

QStringList SyntaxKeywordBase::swapStringCase(const QStringList &list)
{
    QStringList res;
    for (const QString &s: list) {
        QString swapped("");
        for (QChar c: s) {
            swapped += (c.isUpper() ? c.toLower() : c.toUpper());
        }
        res << swapped;
    }
    return res;
}

int SyntaxKeywordBase::findEnd(SyntaxKind kind, const QString& line, int index, int &iKey, bool openEnd, bool relaxDot)
{
    static const QString relaxChars("-_ ()");
    iKey = 0;
    int iChar = 0;
    int iSpace = 0;
    bool relax = false;
    bool allowDigit = false;
    while (true) {
        const DictEntry *dEntry = &mKeywords.value(int(kind))->at(iKey);
        int linePos = iChar + index + iSpace;
        if (relax && linePos < line.length() && relaxChars.contains(line.at(linePos))) {
            ++iSpace;
        } else if (allowDigit && linePos < line.length() && line.at(linePos) >= '0' && line.at(linePos) <= '9') {
            ++iSpace;
        } else if (linePos >= line.length() || !isKeywordChar(line.at(linePos), mExtraKeywordChars)) {
            if (dEntry->length() > iChar) return -1;
            mLastKind = int(kind);
            mLastIKey = iKey;
            return linePos; // reached an valid end
        } else if (iChar < dEntry->length() &&  dEntry->is(line.at(linePos), iChar) ) {
            // character equals
            if (relaxDot && line.at(linePos) == '.') {
                relaxDot = false; relax = true; allowDigit = true;
            } else allowDigit = false;
            iChar++;
        } else if (openEnd && iChar == dEntry->length()) {
            mLastKind = int(kind);
            mLastIKey = iKey;
            return linePos; // reached an valid end of keyword-start
        } else {
            // different character  at iChar: switch to next keyword
            iKey++;
            if (iKey >= mKeywords.value(int(kind))->count()) break; // no more keywords
            // next keyword starts with less equal characters than already matched
            if (mKeywords.value(int(kind))->equalToPrevious(iKey) < iChar) break;
        }
    }
    mLastIKey = -1;
    return -1;
}


SyntaxDeclaration::SyntaxDeclaration(SharedSyntaxData *sharedData)
    : SyntaxKeywordBase(SyntaxKind::Declaration, sharedData)
{
    static const QStringList preTables {"Set", "Parameter", "Variable", "Equation",
                                        "Sets", "Parameters", "Variables", "Equations"};
    QList<QPair<QString, QString>> list;
    list = SyntaxData::declaration();
    mKeywords.insert(int(kind()), new DictList(list));
    for (int i = 0; i < list.size(); ++i) {
        if (list.at(i).first.compare("Table", Qt::CaseInsensitive) == 0) {
            mFlavors.insert(i, flavorTable);
        } else if (list.at(i).first.compare("Model", Qt::CaseInsensitive) == 0) {
            mFlavors.insert(i, flavorModel);
        } else if (list.at(i).first.compare("Models", Qt::CaseInsensitive) == 0) {
            mFlavors.insert(i, flavorModel);
        } else if (preTables.contains(list.at(i).first, Qt::CaseInsensitive)) {
            mFlavors.insert(i, flavorPreTable);
        }
    }

    list = QList<QPair<QString, QString>> {{"Set", ""}, {"Sets", ""}};
    mKeywords.insert(int(SyntaxKind::DeclarationSetType), new DictList(list));

    list = QList<QPair<QString, QString>> {{"Variable", ""}, {"Variables", ""}};
    mKeywords.insert(int(SyntaxKind::DeclarationVariableType), new DictList(list));
    mSubKinds << SyntaxKind::Dco << SyntaxKind::CommentLine << SyntaxKind::CommentEndline
               << SyntaxKind::CommentInline << SyntaxKind::Declaration << SyntaxKind::Identifier;
}

SyntaxBlock SyntaxDeclaration::find(const SyntaxKind entryKind, SyntaxState state, const QString& line, int index)
{
    int start = index;
    int end = -1;
    while (isWhitechar(line, start))
        ++start;
    int iKey;
    if (entryKind == SyntaxKind::DeclarationSetType || entryKind == SyntaxKind::DeclarationVariableType) {

        // search for kind-valid declaration keyword
        end = findEnd(entryKind, line, start, iKey);
        if (entryKind != SyntaxKind::DeclarationSetType)
            state.flavor = state.flavor | mFlavors.value(iKey);
        if (end > start) return SyntaxBlock(this, state, start, end, SyntaxShift::shift);

        // search for invalid new declaration keyword
        end = findEnd(kind(), line, start, iKey);
        if (end > start) return SyntaxBlock(this, state, start, end, true, SyntaxShift::reset);

        return SyntaxBlock(this);
    }

    end = findEnd(kind(), line, start, iKey);
    if (end > start) {
        state.flavor |= mFlavors.value(iKey);
        if (entryKind == SyntaxKind::Declaration) {
            if (state.flavor & flavorPreTable && mFlavors.value(iKey) == flavorTable) {
                state.flavor -= flavorPreTable;
                return SyntaxBlock(this, state, start, end, false, SyntaxShift::shift, kind());
            }
            return SyntaxBlock(this, state, start, end, true, SyntaxShift::reset);
        }
        return SyntaxBlock(this, state, start, end, false, SyntaxShift::in, kind());
    }
    return SyntaxBlock(this);
}

SyntaxPreDeclaration::SyntaxPreDeclaration(SyntaxKind kind, SharedSyntaxData *sharedData)
    : SyntaxKeywordBase(kind, sharedData)
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
        Q_ASSERT_X(false, "SyntaxPreDeclaration", QString("invalid SyntaxKind: %1").arg(syntaxKindName(kind)).toLatin1());
    }
    mSubKinds << SyntaxKind::Dco << SyntaxKind::CommentLine << SyntaxKind::CommentEndline
               << SyntaxKind::CommentInline;

}

SyntaxBlock SyntaxPreDeclaration::find(const SyntaxKind entryKind, SyntaxState state, const QString &line, int index)
{
    int start = index;
    int end = -1;
    while (isWhitechar(line, start))
        ++start;
    int iKey;
    end = findEnd(kind(), line, start, iKey);
    if (end > start) {
        if (entryKind == SyntaxKind::DeclarationSetType || entryKind == SyntaxKind::DeclarationVariableType
                || entryKind == SyntaxKind::Declaration)
            return SyntaxBlock(this, state, start, end, true, SyntaxShift::reset);
        return SyntaxBlock(this, state, start, end,  false, SyntaxShift::in, kind());
    } else if (entryKind == kind()) {
        return SyntaxBlock(this, state, index, start, SyntaxShift::shift);
    }
    return SyntaxBlock(this);
}

SyntaxReserved::SyntaxReserved(SyntaxKind kind, SharedSyntaxData *sharedData) : SyntaxKeywordBase(kind, sharedData)
{
    // TODO(JM) check if other specialized reserved-types beneath solve need to be added here
    mSubKinds << SyntaxKind::Semicolon << SyntaxKind::String << SyntaxKind::Embedded << SyntaxKind::Solve
              << SyntaxKind::Reserved << SyntaxKind::Abort << SyntaxKind::CommentLine << SyntaxKind::CommentEndline
              << SyntaxKind::CommentInline << SyntaxKind::Dco << SyntaxKind::Declaration
              << SyntaxKind::DeclarationSetType << SyntaxKind::DeclarationVariableType;
    QList<QPair<QString, QString>> list;
    switch (kind) {
    case SyntaxKind::Reserved:
        list = SyntaxData::reserved();
        list.remove(0);
        mSubKinds << SyntaxKind::Formula;
        break;
    case SyntaxKind::Abort:
        list << SyntaxData::reserved().first();
        mSubKinds << SyntaxKind::Abort << SyntaxKind::AbortKey << SyntaxKind::Formula;
        break;
    case SyntaxKind::AbortKey:
        list = {{"noError", "Don't throw an execution error"}};
        mSubKinds << SyntaxKind::Formula;
        break;
    case SyntaxKind::Solve:
        list = SyntaxData::keySolve();
        mSubKinds << SyntaxKind::SolveKey << SyntaxKind::SolveBody;
        break;
    case SyntaxKind::Option:
        list = SyntaxData::keyOption();
        mSubKinds << SyntaxKind::OptionKey << SyntaxKind::OptionBody;
        break;
    case SyntaxKind::Execute:
        list = SyntaxData::keyExecute();
        mSubKinds << SyntaxKind::Execute << SyntaxKind::ExecuteKey << SyntaxKind::ExecuteBody;
        break;
    case SyntaxKind::ExecuteKey:
        list = SyntaxData::execute();
        mSubKinds << SyntaxKind::ExecuteBody;
        break;
    case SyntaxKind::ExecuteTool:
        list = {{"executeTool", "Execute a GAMS tool"}};
        mSubKinds << SyntaxKind::ExecuteTool << SyntaxKind::ExecuteToolKey << SyntaxKind::ExecuteBody;
        break;
    case SyntaxKind::ExecuteToolKey:
        list = {{"checkErrorLevel", "Check errorLevel automatically after executing external program"}};
        mSubKinds << SyntaxKind::ExecuteBody;
        break;
    case SyntaxKind::Put:
        list = SyntaxData::keyPut();
        mSubKinds << SyntaxKind::PutFormula;
        break;
    default:
        Q_ASSERT_X(false, "SyntaxReserved", ("Invalid SyntaxKind: "+syntaxKindName(kind)).toLatin1());
    }
    mKeywords.insert(int(kind), new DictList(list));
}

SyntaxBlock SyntaxReserved::find(const SyntaxKind entryKind, SyntaxState state, const QString &line, int index)
{
    int start = index;
    int end = -1;
    while (isWhitechar(line, start))
        ++start;
    if (entryKind == kind() && start > index) {
        return SyntaxBlock(this, state, index, start, false, SyntaxShift::shift);
    }
    if ((kind() == SyntaxKind::Abort || kind() == SyntaxKind::AbortKey) && (state.flavor & flavorAbortCmd) == 0)
        state.flavor += flavorAbortCmd;
    if ((kind() == SyntaxKind::Abort || kind() == SyntaxKind::Execute || kind() == SyntaxKind::ExecuteTool)
        && entryKind == kind() && (state.flavor & flavorExecDot) == 0) {
        if (start < line.length() && line.at(start) == '.') {
            end = start + 1;
            while (isWhitechar(line, end))
                ++end;
            state.flavor += flavorExecDot;
            return SyntaxBlock(this, state, index, end, false, SyntaxShift::shift);
        }
        return SyntaxBlock(this);
    }
    if (kind() != SyntaxKind::AbortKey && kind() != SyntaxKind::ExecuteKey && kind() != SyntaxKind::ExecuteToolKey && state.flavor & flavorExecDot) {
        return SyntaxBlock(this);
    }

    int iKey;
    end = findEnd(kind(), line, start, iKey, (kind() == SyntaxKind::Abort || kind() == SyntaxKind::Execute || kind() == SyntaxKind::ExecuteTool));
    if (end > start) {
        switch (kind()) {
        case SyntaxKind::Reserved:
            return SyntaxBlock(this, state, start, end, false, SyntaxShift::in, SyntaxKind::Formula);
        case SyntaxKind::Solve:
            return SyntaxBlock(this, state, start, end, false, SyntaxShift::in, SyntaxKind::SolveBody);
        case SyntaxKind::Option:
            return SyntaxBlock(this, state, start, end, false, SyntaxShift::in, SyntaxKind::OptionBody);
        case SyntaxKind::Put:
            return SyntaxBlock(this, state, start, end, false, SyntaxShift::in, SyntaxKind::PutFormula);
        case SyntaxKind::Abort:
        case SyntaxKind::Execute:
        case SyntaxKind::ExecuteTool:
        {
            while (isWhitechar(line, end))
                ++end;
            if (end == line.length() || line.at(end) != '_')
                return SyntaxBlock(this, state, start, end, SyntaxShift::shift);
        }   break;
        case SyntaxKind::AbortKey:
        case SyntaxKind::ExecuteKey:
        case SyntaxKind::ExecuteToolKey:
        {
            if ((entryKind == SyntaxKind::Abort || entryKind == SyntaxKind::Execute || entryKind == SyntaxKind::ExecuteTool)
                && state.flavor & flavorExecDot) {
                state.flavor -= flavorExecDot;
                return SyntaxBlock(this, state, index, end, SyntaxShift::shift);
            }
        }   break;
        default:
            break;
        }
    }
    return SyntaxBlock(this);
}


SyntaxEmbedded::SyntaxEmbedded(SyntaxKind kind, SharedSyntaxData *sharedData) : SyntaxKeywordBase(kind, sharedData)
{
    QList<QPair<QString, QString>> list;
//    mSubKinds << SyntaxKind::EmbeddedNameSuffix;
    if (kind == SyntaxKind::Embedded) {
        list = SyntaxData::embedded();
        mSubKinds << SyntaxKind::EmbeddedBody;
    } else {
        list = SyntaxData::embeddedEnd();
    }
    mKeywords.insert(int(kind), new DictList(list));
}

SyntaxBlock SyntaxEmbedded::find(const SyntaxKind entryKind, SyntaxState state, const QString &line, int index)
{
    Q_UNUSED(entryKind)
    int start = index;
    while (isWhitechar(line, start))
        ++start;
    int end = -1;
    int iKey;
    end = findEnd(kind(), line, start, iKey, true);
    if (end > start) {
        SyntaxShift kindShift = (kind() == SyntaxKind::EmbeddedEnd ? SyntaxShift::out : SyntaxShift::in);
        QString suffix = state.syntaxFlagValue(flagSuffixName);
        QChar inType = kind() == SyntaxKind::Embedded ? 'E' : 'e';
        if (hasMatchingSuffix(inType, state, line, end, suffix)) {
            return SyntaxBlock(this, state, start, end, false, kindShift, kind());
        }
    }
    return SyntaxBlock(this);
}

SyntaxEmbeddedBody::SyntaxEmbeddedBody(SharedSyntaxData *sharedData)
    : SyntaxAbstract(SyntaxKind::EmbeddedBody, sharedData)
{
    mSubKinds << SyntaxKind::EmbeddedEnd << SyntaxKind::Dco;
}

SyntaxBlock SyntaxEmbeddedBody::find(const SyntaxKind entryKind, SyntaxState state, const QString &line, int index)
{
    Q_UNUSED(entryKind)
    return SyntaxBlock(this, state, index, line.length());
}

SyntaxBlock SyntaxEmbeddedBody::validTail(const QString &line, int index, SyntaxState state, bool &hasContent)
{
    Q_UNUSED(hasContent)
    return SyntaxBlock(this, state, index, line.length());
}

SyntaxSubsetKey::SyntaxSubsetKey(SyntaxKind kind, SharedSyntaxData *sharedData) : SyntaxKeywordBase(kind, sharedData)
{
    mSubKinds << SyntaxKind::Semicolon << SyntaxKind::Dco << SyntaxKind::CommentLine
              << SyntaxKind::CommentEndline << SyntaxKind::CommentInline;
    QList<QPair<QString, QString>> list;
    list = SyntaxData::modelTypes();
    switch (kind) {
    case SyntaxKind::OptionKey:
        mSubKinds << SyntaxKind::OptionKey << SyntaxKind::OptionBody;
        list << SyntaxData::options();
        mKeywords.insert(int(kind), new DictList(list));
        break;
    case SyntaxKind::SolveKey:
        mSubKinds << SyntaxKind::SolveKey << SyntaxKind::SolveBody;
        list << SyntaxData::extendableKey();
        mKeywords.insert(int(kind), new DictList(list));
        for (const QPair<QString,QString> &entry: SyntaxData::extendableKey()) {
            int iKey;
            findEnd(kind, entry.first, 0, iKey);
            mOtherKey << iKey;
        }
        break;
    default:
        Q_ASSERT_X(false, "SyntaxSubsetKey", ("Invalid SyntaxKind: "+syntaxKindName(kind)).toLatin1());
    }

}

SyntaxBlock SyntaxSubsetKey::find(const SyntaxKind entryKind, SyntaxState state, const QString &line, int index)
{
    Q_UNUSED(entryKind)
    int start = index;
    while (isWhitechar(line, start))
        ++start;
    if (start >= line.length()) return SyntaxBlock(this);
    int end = -1;
    int iKey;
    end = findEnd(kind(), line, start, iKey, kind() == SyntaxKind::SolveKey);
    if (kind() == SyntaxKind::SolveKey && end >= 0 && end < line.length()) {
        int prev = 2;
        if (!mOtherKey.contains(iKey) && charClass(line.at(end), prev) == 2)
            end = -1;
        else while (end < line.length() && charClass(line.at(end), prev) == 2)
            ++end;
    }
    if (end > start) {
        return SyntaxBlock(this, state, start, end, false, SyntaxShift::shift, kind());
    }
    return SyntaxBlock(this);
}

SyntaxBlock SyntaxSubsetKey::validTail(const QString &line, int index, SyntaxState state, bool &hasContent)
{
    return SyntaxKeywordBase::validTail(line, index, state, hasContent);
}

AssignmentSystemData::AssignmentSystemData(SharedSyntaxData *sharedData)
    : SyntaxKeywordBase(SyntaxKind::AssignmentSystemData, sharedData)
{
    mSubKinds << SyntaxKind::Dco << SyntaxKind::CommentLine
              << SyntaxKind::CommentEndline << SyntaxKind::CommentInline;
    mSubKinds << SyntaxKind::IdentifierAssignmentEnd << SyntaxKind::IdentifierAssignment
              << SyntaxKind::AssignmentLabel << SyntaxKind::AssignmentValue;
    QList<QPair<QString, QString>> list = SyntaxData::systemData();
    mKeywords.insert(int(SyntaxKind::AssignmentSystemData), new DictList(list));
}

SyntaxBlock AssignmentSystemData::find(const SyntaxKind entryKind, SyntaxState state, const QString &line, int index)
{
    // TODO(JM) check for which flavors this is valid
    Q_UNUSED(entryKind)
    const QString sys("system.");
    int start = index;
    while (isWhitechar(line, start))
        ++start;
    // allways starts with "system."
    if (line.length() < start+sys.length())
        return SyntaxBlock(this);
    for (int i = 0; i < sys.length(); ++i) {
        if (sys.at(i) != line.at(start+i).toLower())
            return SyntaxBlock(this);
    }
    start += sys.length();
    // look for system-data keyword
    int iKey;
    int end = findEnd(kind(), line, start, iKey);
    if (end > start) {
        if (state.flavor & flavorBindLabel)
            state.flavor -= flavorBindLabel;
        return SyntaxBlock(this, state, index, end, false, SyntaxShift::shift, kind());
    }
    return SyntaxBlock(this);
}

SyntaxBlock AssignmentSystemData::validTail(const QString &line, int index, SyntaxState state, bool &hasContent)
{
    Q_UNUSED(line)
    Q_UNUSED(index)
    Q_UNUSED(state)
    Q_UNUSED(hasContent)
    return SyntaxBlock(this);
}

QString cleanChars(QString source)
{
    return source.remove(' ').remove('-').remove('_').remove('(').remove(')');
}


SyntaxSimpleKeyword::SyntaxSimpleKeyword(SyntaxKind kind, SharedSyntaxData *sharedData) : SyntaxKeywordBase(kind, sharedData)
{
    QList<QPair<QString, QString>> list;
    list = SyntaxData::systemAttributes();
    list.append(SyntaxData::modelTypes());
    if (kind == SyntaxKind::SystemRunAttrib) {
        mKeywords.insert(int(kind), new DictList(list, QStringLiteral(u"system.")));
    } else if (kind == SyntaxKind::SystemCompileAttrib) {
        setExtraKeywordChars("._ ");
        QList<QPair<QString, QString>> list2;
        list2.append(systemEmpData);
        for (const QPair<QString, QString> &entry : std::as_const(list)) {
            list2.append(QPair<QString, QString>(QStringLiteral(u"system.")+entry.first, entry.second));
        }
        mKeywords.insert(int(kind), new DictList(list2));
    } else if (kind == SyntaxKind::SystemCompileAttribR) {
        setExtraKeywordChars("._ ");
        QList<QPair<QString, QString>> list2;
        QHash<QString, QString> descript;
        for (const QPair<QString, QString> &entry : SyntaxData::systemCTConstText())
            descript.insert(entry.first, entry.second);
        for (const QPair<QString, int> &entry : SyntaxData::systemCTConstants()) {
            int split = entry.first.indexOf('.');
            QString key = entry.first.left(split);
            QString value = key + cleanChars(entry.first.mid(split));
            list2.append(QPair<QString, QString>(value, descript.value(key) + ": " + QString::number(entry.second)));
        }
        mKeywords.insert(int(kind), new DictList(list2));
    } else {
        Q_ASSERT_X(false, "SyntaxSimpleKeyword", QString("invalid SyntaxKind: %1").arg(syntaxKindName(kind)).toLatin1());
    }
}

SyntaxBlock SyntaxSimpleKeyword::find(const SyntaxKind entryKind, SyntaxState state, const QString &line, int index)
{
    Q_UNUSED(entryKind)
    int start = index;
    int iKey;
    if (kind() == SyntaxKind::SystemCompileAttrib || kind() == SyntaxKind::SystemCompileAttribR) {
        if (start+1 >= line.length() || line.at(start) != '%')
            return SyntaxBlock(this);
        ++start;
    }
    int end = findEnd(kind(), line, start, iKey, false, kind() == SyntaxKind::SystemCompileAttribR);
    if (end > start) {
        if (kind() == SyntaxKind::SystemCompileAttrib || kind() == SyntaxKind::SystemCompileAttribR) {
            if (end >= line.length() || line.at(end) != '%')
                return SyntaxBlock(this);
            --start;
            ++end;
        }
        return SyntaxBlock(this, state, start, end, false, SyntaxShift::skip);
    }
    return SyntaxBlock(this);
}

SyntaxBlock SyntaxSimpleKeyword::validTail(const QString &line, int index, SyntaxState state, bool &hasContent)
{
    Q_UNUSED(line)
    Q_UNUSED(index)
    Q_UNUSED(state)
    Q_UNUSED(hasContent)
    return SyntaxBlock(this);
}


} // namespace syntax
} // namespace studio
} // namespace gans
