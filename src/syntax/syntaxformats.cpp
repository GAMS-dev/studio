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
#include "syntaxformats.h"
#include "syntaxdata.h"
#include "logger.h"
#include "exception.h"

namespace gams {
namespace studio {
namespace syntax {

const QVector<QChar> SyntaxAbstract::cSpecialCharacters = {'"', '$', '\'', '.', ';'};

QString syntaxKindName(SyntaxKind kind)
{
    return QVariant::fromValue(kind).toString();
}

QString syntaxKindName(int kind)
{
    return syntaxKindName(SyntaxKind(kind));
}

void SyntaxAbstract::assignColorSlot(Theme::ColorSlot slot)
{
    mColorSlot = slot;
    charFormat().setProperty(QTextFormat::UserProperty, intSyntaxType());
    if (toColor(slot).isValid())
        charFormat().setForeground(toColor(slot));
    else
        charFormat().setForeground(Qt::black);
    charFormat().setFontWeight(Theme::hasFlag(slot, Theme::fBold) ? QFont::Bold : QFont::Normal);
    charFormat().setFontItalic(Theme::hasFlag(slot, Theme::fItalic));
}

const SyntaxTransitions SyntaxAbstract::nextKinds(bool emptyLine)
{
    if (emptyLine && !mEmptyLineKinds.isEmpty()) return mEmptyLineKinds;
    return mSubKinds;
}

QTextCharFormat SyntaxAbstract::charFormatError()
{
    QTextCharFormat errorFormat;
    errorFormat.setUnderlineColor(Theme::color(Theme::Normal_Red));
    errorFormat.setUnderlineStyle(QTextCharFormat::WaveUnderline);
    return errorFormat;
}

int SyntaxAbstract::stateToInt(SyntaxKind _state)
{
    return static_cast<int>(_state);
}

SyntaxKind SyntaxAbstract::intToState(int intState)
{
    int i = intState - QTextFormat::UserObject;
    if (i >= 0 && i < static_cast<int>(SyntaxKind::KindCount))
        return static_cast<SyntaxKind>(intState);
    else
        return SyntaxKind::Standard;
}

int SyntaxAbstract::endOfQuotes(const QString &line, const int &start)
{
    if (start > line.size()) return start;
    QChar ch = line.at(start);
    QString bounds("\"\'");
    if (bounds.indexOf(ch) < 0) return start; // no start character
    for (int i = start+1; i < line.length(); ++i) {
        if (line.at(i) == ch) return i;
    }
    return line.length();
}

int SyntaxAbstract::endOfParentheses(const QString &line, const int &start, const QString &validPairs, int &nest)
{
    if (start > line.size()) return start;
    QChar ch = line.at(start);
    if (validPairs.indexOf(ch) % 2) return start; // no start character
    for (int i = start+1; i < line.length(); ++i) {
        if (!validPairs.contains(line.at(i))) continue;
        if (validPairs.indexOf(line.at(i)) % 2)
            --nest;
        else
            ++nest;
        if (!nest) return i;
    }
    return line.length();
}


SyntaxStandard::SyntaxStandard(SharedSyntaxData *sharedData) : SyntaxAbstract(SyntaxKind::Standard, sharedData)
{
    mSubKinds << SyntaxKind::Semicolon
              << SyntaxKind::CommentLine
              << SyntaxKind::CommentEndline
              << SyntaxKind::CommentInline
              << SyntaxKind::Declaration
              << SyntaxKind::DeclarationSetType
              << SyntaxKind::DeclarationVariableType
              << SyntaxKind::Dco
              << SyntaxKind::Solve
              << SyntaxKind::Option
              << SyntaxKind::Execute
              << SyntaxKind::Put
              << SyntaxKind::Reserved
              << SyntaxKind::Embedded
              << SyntaxKind::Formula;
}

SyntaxBlock SyntaxStandard::find(const SyntaxKind entryKind, int flavor, const QString& line, int index)
{
    static QVector<SyntaxKind> invalidEntries {SyntaxKind::Declaration, SyntaxKind::DeclarationSetType,
                SyntaxKind::DeclarationVariableType};
    Q_UNUSED(entryKind)
    Q_UNUSED(flavor)
    bool error = invalidEntries.contains(entryKind);
    int end = index;
    while (isKeywordChar(line, end)) end++;
    while (!isKeywordChar(line, end) && end < line.length()) end++;
    return SyntaxBlock(this, 0, index, end, error);
}

SyntaxBlock SyntaxStandard::validTail(const QString &line, int index, int flavor, bool &hasContent)
{
    Q_UNUSED(line)
    Q_UNUSED(index)
    Q_UNUSED(flavor)
    Q_UNUSED(hasContent)
    return SyntaxBlock();
}

SyntaxDco::SyntaxDco(SharedSyntaxData *sharedData, QChar dcoChar)
    : SyntaxAbstract(SyntaxKind::Dco, sharedData)
{
    mRex.setPattern(QString("(^%1|%1%1)\\s*([\\w]+)").arg(QRegularExpression::escape(dcoChar)));

    const QList<QPair<QString, QString>> data = SyntaxData::directives();
    QStringList blockEndingDCOs;
    blockEndingDCOs << "offText" << "offPut" << "pauseEmbeddedCode" << "endEmbeddedCode" << "offEmbeddedCode";
    for (const QPair<QString, QString> &list: data) {
        if (blockEndingDCOs.contains(list.first)) {
            // block-ending DCOs are checked separately -> ignore here
            blockEndingDCOs.removeAll(list.first);
            mEndDCOs << list.first;
            mEndDCOlow << list.first.toLower();
            mEndDescription << list.second;
        } else {
            mDCOs << list.first;
            mDCOlow << list.first.toLower();
            mDescription << list.second;
        }
    }
    // ensure offtext to be first and offput second
    int i = mEndDCOlow.indexOf("offput");
    if (i > 0) {
        mEndDCOs.move(i, 0);
        mEndDCOlow.move(i, 0);
        mEndDescription.move(i, 0);
    }
    i = mEndDCOlow.indexOf("offtext");
    if (i > 0) {
        mEndDCOs.move(i, 0);
        mEndDCOlow.move(i, 0);
        mEndDescription.move(i, 0);
    }

    if (!blockEndingDCOs.isEmpty()) {
        DEB() << "Initialization error in SyntaxDco. Unknown DCO(s): " << blockEndingDCOs.join(",");
    }
    // !!! Enter flavored names always in lowercase
    mFlavors.insert(QString("onText").toLower(), 1);
    mFlavors.insert(QString("offText").toLower(), 2);
    mFlavors.insert(QString("onEcho").toLower(), 3);
    mFlavors.insert(QString("onEchoV").toLower(), 3);
    mFlavors.insert(QString("onEchoS").toLower(), 3);
    mFlavors.insert(QString("offEcho").toLower(), 4);
    mFlavors.insert(QString("onPut").toLower(), 5);
    mFlavors.insert(QString("onPutV").toLower(), 5);
    mFlavors.insert(QString("onPutS").toLower(), 5);
    mFlavors.insert(QString("offPut").toLower(), 6);
    mFlavors.insert(QString("onExternalInput").toLower(), 7);
    mFlavors.insert(QString("offExternalInput").toLower(), 8);
    mFlavors.insert(QString("onExternalOutput").toLower(), 9);
    mFlavors.insert(QString("offExternalOutput").toLower(), 10);
    mFlavors.insert(QString("ifThen").toLower(), 11);
    mFlavors.insert(QString("ifThenI").toLower(), 11);
    mFlavors.insert(QString("ifThenE").toLower(), 11);
    mFlavors.insert(QString("endIf").toLower(), 12);
    mFlavors.insert(QString("onFold").toLower(), 13);
    mFlavors.insert(QString("offFold").toLower(), 14);
    mFlavors.insert(QString("include").toLower(), 15);
    mFlavors.insert(QString("abort").toLower(), 16);
    mFlavors.insert(QString("call").toLower(), 17);
    mFlavors.insert(QString("hiddenCall").toLower(), 17);
    mFlavors.insert(QString("eval").toLower(), 18);
    mFlavors.insert(QString("evalLocal").toLower(), 18);
    mFlavors.insert(QString("evalGlobal").toLower(), 18);
    mFlavors.insert(QString("onEmbeddedCode").toLower(), 19);
    mFlavors.insert(QString("onEmbeddedCodeS").toLower(), 19);
    mFlavors.insert(QString("onEmbeddedCodeV").toLower(), 19);
    mFlavors.insert(QString("embeddedCode").toLower(), 20);
    mFlavors.insert(QString("embeddedCodeS").toLower(), 20);
    mFlavors.insert(QString("embeddedCodeV").toLower(), 20);
    mFlavors.insert(QString("continueEmbeddedCode").toLower(), 20);
    mFlavors.insert(QString("continueEmbeddedCodeS").toLower(), 20);
    mFlavors.insert(QString("continueEmbeddedCodeV").toLower(), 20);
    // !!! Enter special kinds always in lowercase
    mSpecialKinds.insert(QString("title").toLower(), SyntaxKind::Title);
    mSpecialKinds.insert(QString("onText").toLower(), SyntaxKind::CommentBlock);
    mSpecialKinds.insert(QString("onEcho").toLower(), SyntaxKind::IgnoredHead);
    mSpecialKinds.insert(QString("onEchoV").toLower(), SyntaxKind::IgnoredHead);
    mSpecialKinds.insert(QString("onEchoS").toLower(), SyntaxKind::IgnoredHead);
    mSpecialKinds.insert(QString("onPut").toLower(), SyntaxKind::IgnoredBlock);
    mSpecialKinds.insert(QString("onPutV").toLower(), SyntaxKind::IgnoredBlock);
    mSpecialKinds.insert(QString("onPutS").toLower(), SyntaxKind::IgnoredBlock);
    mSpecialKinds.insert(QString("embeddedCode").toLower(), SyntaxKind::EmbeddedBody);
    mSpecialKinds.insert(QString("embeddedCodeS").toLower(), SyntaxKind::EmbeddedBody);
    mSpecialKinds.insert(QString("embeddedCodeV").toLower(), SyntaxKind::EmbeddedBody);
    mSpecialKinds.insert(QString("continueEmbeddedCode").toLower(), SyntaxKind::EmbeddedBody);
    mSpecialKinds.insert(QString("continueEmbeddedCodeS").toLower(), SyntaxKind::EmbeddedBody);
    mSpecialKinds.insert(QString("continueEmbeddedCodeV").toLower(), SyntaxKind::EmbeddedBody);
    mSpecialKinds.insert(QString("onEmbeddedCode").toLower(), SyntaxKind::EmbeddedBody);
    mSpecialKinds.insert(QString("onEmbeddedCodeS").toLower(), SyntaxKind::EmbeddedBody);
    mSpecialKinds.insert(QString("onEmbeddedCodeV").toLower(), SyntaxKind::EmbeddedBody);
    mSpecialKinds.insert(QString("hidden").toLower(), SyntaxKind::DcoComment);
    mSpecialKinds.insert(QString("abort").toLower(), SyntaxKind::SubDCO);
    mSpecialKinds.insert(QString("call").toLower(), SyntaxKind::SubDCO);
    mSpecialKinds.insert(QString("hiddenCall").toLower(), SyntaxKind::SubDCO);
    mSpecialKinds.insert(QString("eval").toLower(), SyntaxKind::SubDCO);
    mSpecialKinds.insert(QString("evalLocal").toLower(), SyntaxKind::SubDCO);
    mSpecialKinds.insert(QString("evalGlobal").toLower(), SyntaxKind::SubDCO);
}

SyntaxBlock SyntaxDco::find(const SyntaxKind entryKind, int flavor, const QString& line, int index)
{
    mLastIKey = -1;
    mLastEndIKey = -1;
    QRegularExpressionMatch match = mRex.match(line, index);
    if (!match.hasMatch()) return SyntaxBlock(this);
    int outFlavor = mFlavors.value(match.captured(2).toLower(), 0);
    mLastEndIKey = mEndDCOlow.indexOf(match.captured(2).toLower());
    if (entryKind == SyntaxKind::CommentBlock) {
        if (mLastEndIKey == 0)
            return SyntaxBlock(this, outFlavor, match.capturedStart(1), match.capturedEnd(0), SyntaxShift::out);
        return SyntaxBlock(this);
    } else if (entryKind == SyntaxKind::IgnoredBlock || entryKind == SyntaxKind::IgnoredHead) {
        if (flavor == 3 && match.captured(2).compare("offecho", Qt::CaseInsensitive) == 0)
            return SyntaxBlock(this, outFlavor, match.capturedStart(1), match.capturedEnd(0), SyntaxShift::out);
        if (flavor == 5 && mLastEndIKey == 1)
            return SyntaxBlock(this, outFlavor, match.capturedStart(1), match.capturedEnd(0), SyntaxShift::out);
        return SyntaxBlock(this);
    } else if (entryKind == SyntaxKind::EmbeddedBody) {
        if (mLastEndIKey > 1)
            return SyntaxBlock(this, outFlavor, match.capturedStart(1), match.capturedEnd(0), SyntaxShift::out);
        return SyntaxBlock(this);
    } else if (mSharedData->commentEndLine()) {
        if (match.captured(2).startsWith("oneolcom", Qt::CaseInsensitive)) {
            // This only activates the current eolCom
//            mSharedData->commentEndLine()->setCommentChars("!!");
            for (SyntaxFormula * sf: mSharedData->allFormula()) {
                sf->setSpecialDynamicChars(QVector<QChar>() << '!');
            }
            mSharedData->dcoBody()->setCommentChars(QVector<QChar>() << '!');
         } else if (match.captured(2).startsWith("eolcom", Qt::CaseInsensitive)) {
            int i = match.capturedEnd(2);
            while (isWhitechar(line,i)) ++i;
            int comSize = 2;
            if (i+1 == line.length() || isWhitechar(line,i+1)) comSize = 1;
            if (i+comSize <= line.length()) {
                mSharedData->commentEndLine()->setCommentChars(line.mid(i,comSize));
                for (SyntaxFormula * sf: mSharedData->allFormula()) {
                    sf->setSpecialDynamicChars(QVector<QChar>() << line.at(i));
                }
                mSharedData->dcoBody()->setCommentChars(QVector<QChar>() << line.at(i));
            }
        }
    }
    SyntaxKind next = mSpecialKinds.value(match.captured(2).toLower(), SyntaxKind::DcoBody);
    mLastIKey = mDCOlow.indexOf(match.captured(2).toLower());
    if (mLastIKey >= 0) {
        bool atEnd = match.capturedEnd(0) >= line.length();
        if (next == SyntaxKind::IgnoredHead && atEnd)
            next = SyntaxKind::IgnoredBlock;
        bool isMultiLine = next == SyntaxKind::CommentBlock || next == SyntaxKind::IgnoredBlock
                || next == SyntaxKind::EmbeddedBody;
        SyntaxShift shift = (atEnd && !isMultiLine) ? SyntaxShift::skip : SyntaxShift::in;
        return SyntaxBlock(this, outFlavor, match.capturedStart(1), match.capturedEnd(0), false, shift, next);
    } else {
        return SyntaxBlock(this, outFlavor, match.capturedStart(1), match.capturedEnd(0), next, true);
    }
}

SyntaxBlock SyntaxDco::validTail(const QString &line, int index, int flavor, bool &hasContent)
{
    int end = index;
    if (flavor < 16 || flavor > 18)
        while (isWhitechar(line, end)) end++;
    hasContent = false;
    SyntaxShift shift = (line.length() == end) ? SyntaxShift::skip : SyntaxShift::in;
    return SyntaxBlock(this, flavor, index, end, shift);
}

QStringList SyntaxDco::docForLastRequest() const
{
    QStringList res;
    if (mLastIKey >= 0)
        res << '$'+mDCOs.at(mLastIKey) << mDescription.at(mLastIKey);
    else if (mLastEndIKey >= 0)
        res << '$'+mEndDCOs.at(mLastEndIKey) << mEndDescription.at(mLastEndIKey);
    return res;
}


SyntaxDcoBody::SyntaxDcoBody(SyntaxKind kind, SharedSyntaxData *sharedData)
    : SyntaxAbstract(kind, sharedData)
{
    sharedData->registerDcoBody(this);
    if (kind == SyntaxKind::IgnoredHead) {
        mSubKinds << SyntaxKind::Dco << SyntaxKind::IgnoredHead << SyntaxKind::IgnoredBlock;
        mEmptyLineKinds << SyntaxKind::IgnoredBlock;
    }
    else mSubKinds << SyntaxKind::CommentEndline << SyntaxKind::CommentInline << SyntaxKind::DcoBody;
    Q_ASSERT_X((kind == SyntaxKind::DcoBody || kind == SyntaxKind::DcoComment || kind == SyntaxKind::Title || kind == SyntaxKind::IgnoredHead),
               "SyntaxDcoBody", QString("invalid SyntaxKind: %1").arg(syntaxKindName(kind)).toLatin1());
}

void SyntaxDcoBody::setCommentChars(QVector<QChar> chars)
{
    mEolComChars = chars;
}

SyntaxBlock SyntaxDcoBody::find(const SyntaxKind entryKind, int flavor, const QString& line, int index)
{
    int end = index;
    if (index == 0 && entryKind == SyntaxKind::IgnoredHead) return SyntaxBlock();
    if (entryKind == SyntaxKind::DcoBody && end < line.length()
            && mSharedData->dcoBody()->mEolComChars.contains(line.at(end))) ++end;
    while (end < line.length() && !mSharedData->dcoBody()->mEolComChars.contains(line.at(end)))
        ++end;
    return SyntaxBlock(this, flavor, index, end, SyntaxShift::shift);
}

SyntaxBlock SyntaxDcoBody::validTail(const QString &line, int index, int flavor, bool &hasContent)
{
    int start = index;
    while (isWhitechar(line, start)) start++;
    int end = start;
    while (end < line.length() && !mSharedData->dcoBody()->mEolComChars.contains(line.at(end)))
        ++end;
    hasContent = end > start;
    return SyntaxBlock(this, flavor, start, end, SyntaxShift::shift);
}


SyntaxCommentLine::SyntaxCommentLine(SharedSyntaxData *sharedData, QChar commentChar)
    : SyntaxAbstract(SyntaxKind::CommentLine, sharedData), mCommentChar(commentChar)
{ }

SyntaxBlock SyntaxCommentLine::find(const SyntaxKind entryKind, int flavor, const QString& line, int index)
{
    Q_UNUSED(entryKind)
    if (entryKind == SyntaxKind::CommentLine || (index==0 && line.startsWith(mCommentChar)))
        return SyntaxBlock(this, flavor, index, line.length(), false, SyntaxShift::skip);
    return SyntaxBlock();
}

SyntaxBlock SyntaxCommentLine::validTail(const QString &line, int index, int flavor, bool &hasContent)
{
    int end = index;
    while (isWhitechar(line, end)) end++;
    hasContent = end < line.length();
    return SyntaxBlock(this, flavor, index, line.length(), SyntaxShift::out);
}


SyntaxUniformBlock::SyntaxUniformBlock(SyntaxKind kind, SharedSyntaxData *sharedData)
    : SyntaxAbstract(kind, sharedData)
{
    mSubKinds << SyntaxKind::Dco;
}

SyntaxBlock SyntaxUniformBlock::find(const SyntaxKind entryKind, int flavor, const QString& line, int index)
{
    Q_UNUSED(entryKind)
    return SyntaxBlock(this, flavor, index, line.length());
}

SyntaxBlock SyntaxUniformBlock::validTail(const QString &line, int index, int flavor, bool &hasContent)
{
    int end = index;
    while (isWhitechar(line, end)) end++;
    hasContent = end < line.length();
    return SyntaxBlock(this, flavor, index, line.length(), SyntaxShift::shift);
}

SyntaxDelimiter::SyntaxDelimiter(SyntaxKind kind, SharedSyntaxData *sharedData)
    : SyntaxAbstract(kind, sharedData)
{
    mSubKinds << SyntaxKind::CommentEndline << SyntaxKind::CommentLine;
    if (kind == SyntaxKind::Semicolon) {
        mDelimiter = ';';
    } else if (kind == SyntaxKind::CommaIdent) {
        mDelimiter = ',';
        mSubKinds << SyntaxKind::Identifier;
    } else {
        Q_ASSERT_X(false, "SyntaxDelimiter", QString("invalid SyntaxKind: %1").arg(syntaxKindName(kind)).toLatin1());
    }
}

SyntaxBlock SyntaxDelimiter::find(const SyntaxKind entryKind, int flavor, const QString &line, int index)
{
    Q_UNUSED(entryKind)
    int end = index;
    while (isWhitechar(line, end)) end++;
    if (end < line.length() && line.at(end) == mDelimiter) {
        if (kind() == SyntaxKind::Semicolon)
            return SyntaxBlock(this, flavor, index, end+1, SyntaxShift::reset);
        return SyntaxBlock(this, flavor, index, end+1, SyntaxShift::shift);
    }
    return SyntaxBlock(this);
}

SyntaxBlock SyntaxDelimiter::validTail(const QString &line, int index, int flavor, bool &hasContent)
{
    hasContent = false;
    int end = index;
    while (isWhitechar(line, end)) end++;
    if (kind() == SyntaxKind::Semicolon)
        return SyntaxBlock(this, flavor, index, end, SyntaxShift::reset);
    return SyntaxBlock(this, flavor, index, end, SyntaxShift::shift);
}

SyntaxFormula::SyntaxFormula(SyntaxKind kind, SharedSyntaxData *sharedData) : SyntaxAbstract(kind, sharedData)
{
    sharedData->addFormula(this);
    mSubKinds << SyntaxKind::Embedded << SyntaxKind::Semicolon
              << SyntaxKind::CommentLine << SyntaxKind::CommentEndline << SyntaxKind::CommentInline
              << SyntaxKind::Solve << SyntaxKind::Option << SyntaxKind::Execute
              << SyntaxKind::Put << SyntaxKind::Reserved << SyntaxKind::Dco << SyntaxKind::Assignment
              << SyntaxKind::Declaration << SyntaxKind::DeclarationSetType << SyntaxKind::DeclarationVariableType
              << SyntaxKind::String;

    switch (kind) {
    case SyntaxKind::Formula:
        mSubKinds << SyntaxKind::Formula;
        break;
    case SyntaxKind::PutFormula:
        mSubKinds << SyntaxKind::SystemRunAttrib << SyntaxKind::PutFormula;
        break;
    case SyntaxKind::SolveBody:
        mSubKinds << SyntaxKind::SolveKey << SyntaxKind::SolveBody;
        break;
    case SyntaxKind::OptionBody:
        mSubKinds << SyntaxKind::OptionKey << SyntaxKind::OptionBody;
        break;
    case SyntaxKind::ExecuteBody:
        mSubKinds << SyntaxKind::ExecuteBody;
        break;
    default:
        Q_ASSERT_X(false, "SyntaxFormula", ("Invalid SyntaxKind:"+syntaxKindName(kind)).toLatin1());
    }
}

SyntaxBlock SyntaxFormula::find(const SyntaxKind entryKind, int flavor, const QString &line, int index)
{
    Q_UNUSED(entryKind)
    int start = index;
    while (isWhitechar(line, start))
        ++start;
    if (start >= line.length()) return SyntaxBlock(this);
    int prev = 0;

    if (kind() == SyntaxKind::ExecuteBody) {
        if (entryKind == SyntaxKind::Execute) {
            if (!(flavor % 2) && start < line.length() && line.at(start) == '.')
                flavor += 1;
        } else if (flavor % 2)
            flavor -= 1;
    }

    int end = start;
    int chKind = charClass(line.at(end), prev, mSpecialDynamicChars);
    if (/*kind() == SyntaxKind::PutFormula &&*/ start > index && chKind == ccAlpha)
        return SyntaxBlock(this, flavor, index, start, SyntaxShift::shift);
    bool skipWord = (chKind == ccAlpha);
    if (chKind == ccSpecial) --end;

    while (++end < line.length()) {
        chKind = charClass(line.at(end), prev, mSpecialDynamicChars);
        if (flavor % 2 && (line.at(end) == ' ' || line.at(end) == '\t')) {
            break;
        }
        if (chKind == ccSpecial) break;
        if (chKind != ccAlpha) skipWord = false;
        else if (!skipWord) break;
    }
    return SyntaxBlock(this, flavor, index, end, SyntaxShift::shift);
}

SyntaxBlock SyntaxFormula::validTail(const QString &line, int index, int flavor, bool &hasContent)
{
    int end = index;
    while (isWhitechar(line, end))
        ++end;
    if (end >= line.length()) {
        if (end > index) return SyntaxBlock(this, flavor, index, end, SyntaxShift::shift);
        else return SyntaxBlock(this);
    }
    int prev = 0;
    int cb1 = charClass(line.at(end), prev, mSpecialDynamicChars);
    while (++end < line.length() && charClass(line.at(end), prev, mSpecialDynamicChars) == cb1) ;
    hasContent = false;
    return SyntaxBlock(this, flavor, index, end, SyntaxShift::shift);
}

void SyntaxFormula::setSpecialDynamicChars(QVector<QChar> chars)
{
    mSpecialDynamicChars = chars;
    if (kind() == SyntaxKind::Formula)
        mSpecialDynamicChars << '='; // << '\'' << '"';
}

SyntaxQuoted::SyntaxQuoted(SyntaxKind kind, SharedSyntaxData *sharedData)
    : SyntaxAbstract(kind, sharedData)
{
    if (kind == SyntaxKind::String) {
        mDelimiters = " '\"";
    }
    mSubKinds << SyntaxKind::String;
}

SyntaxBlock SyntaxQuoted::find(const SyntaxKind entryKind, int flavor, const QString &line, int index)
{
    Q_UNUSED(entryKind)
    int start = index;
    while (isWhitechar(line, start)) start++;
    int end = start;
    if ((flavor & flavorQuotePart) == 0) {
        if (start < line.length() && mDelimiters.indexOf(line.at(start)) > 0) {
            // starting part, remember flavor
            flavor += mDelimiters.indexOf(line.at(start));
        } else
            return SyntaxBlock(this);
    }
    while (++end < line.length() && line.at(end) != mDelimiters.at(flavor & flavorQuotePart))
        ;
    if (end < line.length()) {
        if (entryKind == SyntaxKind::String) {
            if (line.at(end) == mDelimiters.at(flavor & flavorQuotePart))
                return SyntaxBlock(this, 0, start, end+1, SyntaxShift::out);
        } else {
            if (line.at(end) == mDelimiters.at(flavor & flavorQuotePart))
                return SyntaxBlock(this, 0, start, end+1, SyntaxShift::skip);
        }
    }

    return SyntaxBlock(this);
}

SyntaxBlock SyntaxQuoted::validTail(const QString &line, int index, int flavor, bool &hasContent)
{
    Q_UNUSED(line)
    Q_UNUSED(index)
    Q_UNUSED(flavor)
    Q_UNUSED(hasContent)
    return SyntaxBlock(this);
}

SyntaxAssign::SyntaxAssign(SharedSyntaxData *sharedData) : SyntaxAbstract(SyntaxKind::Assignment, sharedData)
{}

SyntaxBlock SyntaxAssign::find(const SyntaxKind entryKind, int flavor, const QString &line, int index)
{
    Q_UNUSED(entryKind)
    int start = index;
    while (isWhitechar(line, start)) start++;
    if (start >= line.length()) return SyntaxBlock(this);
    if (line.at(start) == '.') {
        if (line.length() > start+1 && line.at(start+1) == '.')
            return SyntaxBlock(this, flavor, start, start+2, SyntaxShift::skip);
    } else if (line.at(start) == '=') {
        if (start+1 >= line.length())
            return SyntaxBlock(this, flavor, start, start+1, SyntaxShift::skip);
        if (line.at(start+1) == '=')
            return SyntaxBlock(this, flavor, start, start+2, SyntaxShift::skip);
        if (start+2 >= line.length() || line.at(start+2) != '=')
            return SyntaxBlock(this, flavor, start, start+1, SyntaxShift::skip);
        if (start+2 <= line.length()) {
            bool error = (QString("eglnxcb").indexOf(line.at(start+1), 0, Qt::CaseInsensitive) < 0);
            return SyntaxBlock(this, flavor, start, start+3, error, SyntaxShift::skip);
        }
    }
    return SyntaxBlock(this);
}

SyntaxBlock SyntaxAssign::validTail(const QString &line, int index, int flavor, bool &hasContent)
{
    Q_UNUSED(line)
    Q_UNUSED(index)
    Q_UNUSED(flavor)
    Q_UNUSED(hasContent)
    return SyntaxBlock(this);
}

SyntaxCommentEndline::SyntaxCommentEndline(SharedSyntaxData *sharedData, QString commentChars)
    : SyntaxAbstract(SyntaxKind::CommentEndline, sharedData)
{
    mSharedData->registerCommentEndLine(this);
    setCommentChars(commentChars);
}

void SyntaxCommentEndline::setCommentChars(QString commentChars)
{
    if (commentChars.length() == 1 || commentChars.length() == 2)
        mCommentChars = commentChars;
}

bool SyntaxCommentEndline::check(const QString &line, int index) const
{
    if (index + mCommentChars.length() > line.length()) return false;
    if (line.at(index) != mCommentChars.at(0)) return false;
    if (mCommentChars.length() > 1 && line.at(index+1) != mCommentChars.at(1)) return false;
    return true;
}

SyntaxBlock SyntaxCommentEndline::find(const SyntaxKind entryKind, int flavor, const QString &line, int index)
{
    Q_UNUSED(entryKind)
    int start = index;
    while (isWhitechar(line, start))
        ++start;

    if (start+mCommentChars.size() <= line.length() && line.at(start) == mCommentChars.at(0) &&
            (mCommentChars.size() == 1 || line.at(start+1) == mCommentChars.at(1)))
        return SyntaxBlock(this, flavor, start, line.length(), SyntaxShift::skip);
    return SyntaxBlock(this);
}

SyntaxBlock SyntaxCommentEndline::validTail(const QString &line, int index, int flavor, bool &hasContent)
{
    Q_UNUSED(line)
    Q_UNUSED(index)
    Q_UNUSED(flavor)
    Q_UNUSED(hasContent)
    return SyntaxBlock(this);
}

SyntaxSubDCO::SyntaxSubDCO(SharedSyntaxData *sharedData): SyntaxAbstract(SyntaxKind::SubDCO, sharedData)
{
    QList<QPair<QString, QString>> list = SyntaxData::execute();
    for (const QPair<QString,QString> &entry : qAsConst(list)) {
        mSubDCOs << entry.first;
    }
    mSubKinds << SyntaxKind::SubDCO << SyntaxKind::DcoBody;
}

SyntaxBlock SyntaxSubDCO::find(const gams::studio::syntax::SyntaxKind entryKind, int flavor, const QString &line, int index)
{
    Q_UNUSED(entryKind)
    int start = index;
    if (start < line.length() && line.at(start) == '.') ++start;

    QStringList subDCOs;
    if (flavor == 16)
        subDCOs << "noerror";
    else if (flavor == 17)
        subDCOs = mSubDCOs;
    else if (flavor == 18)
        subDCOs << "set";

    for (int i = subDCOs.size()-1 ; i >= 0 ; --i) {
        const QString &sub = subDCOs.at(i);
        if (line.length() >= start+sub.length() && sub.compare(line.midRef(start, sub.length()), Qt::CaseInsensitive) == 0) {
            SyntaxShift shift = (line.length() == start+sub.length()) ? SyntaxShift::skip : SyntaxShift::shift;
            return SyntaxBlock(this, flavor, index, start+sub.length(), shift);
        }
    }
    return SyntaxBlock(this);
}

SyntaxBlock SyntaxSubDCO::validTail(const QString &line, int index, int flavor, bool &hasContent)
{
    hasContent = false;
    int end = index;
    while (isWhitechar(line, end)) ++end;
    // TODO(JM) review: add silly additional condition, trying to calm down the compiler
    if (end >= index && end < line.length()-1) return SyntaxBlock(this, flavor, index, end, SyntaxKind::DcoBody);
    if (end > index) return SyntaxBlock(this, flavor, index, end, SyntaxShift::out);
    return SyntaxBlock(this);
}

} // namespace syntax
} // namespace studio
} // namespace gams
