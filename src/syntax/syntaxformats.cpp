#include "syntaxformats.h"
#include "logger.h"
#include "syntaxdata.h"

namespace gams {
namespace studio {


QTextCharFormat SyntaxAbstract::charFormatError()
{
    QTextCharFormat errorFormat(charFormat());
    errorFormat.setUnderlineColor(Qt::red);
    errorFormat.setUnderlineStyle(QTextCharFormat::WaveUnderline);
    return errorFormat;
}


SyntaxStandard::SyntaxStandard()
{
    mSubStates << SyntaxState::Directive << SyntaxState::CommentLine << SyntaxState::Declaration
               << SyntaxState::Declaration << SyntaxState::DeclarationSetType << SyntaxState::DeclarationVariableType;
}

SyntaxBlock SyntaxStandard::find(SyntaxState entryState, const QString& line, int index)
{
    Q_UNUSED(entryState);
    return SyntaxBlock(this, index, line.length());
}

SyntaxDirective::SyntaxDirective(QChar directiveChar)
{
    mRex.setPattern(QString("(^%1|%1%1)\\s*([\\w\\.]+)\\s*").arg(QRegularExpression::escape(directiveChar)));

    // TODO(JM) parse source file: src\gamscmex\gmsdco.gms or better create a lib that can be called to get the list
    QList<QStringList> data = SyntaxData::directives();
    for (const QStringList &list: data) {
        mDirectives << list.first();
        mDescription << list.last();
    }
    // offtext is checked separately, so remove it here
    int i = mDirectives.indexOf(QRegExp("offText", Qt::CaseInsensitive));
    if (i>0) {
        mDirectives.removeAt(i);
        mDescription.removeAt(i);
    }
    // !!! Enter special states always in lowercase
    mSpecialStates.insert(QString("title").toLower(), SyntaxState::Title);
    mSpecialStates.insert(QString("onText").toLower(), SyntaxState::CommentBlock);
    mSpecialStates.insert(QString("hidden").toLower(), SyntaxState::CommentLine);
}

SyntaxBlock SyntaxDirective::find(SyntaxState entryState, const QString& line, int index)
{
    QRegularExpressionMatch match = mRex.match(line, index);
    if (!match.hasMatch()) return SyntaxBlock();
    if (entryState == SyntaxState::CommentBlock) {
        if (match.captured(2).compare("offtext", Qt::CaseInsensitive)==0)
            return SyntaxBlock(this, match.capturedStart(1), match.capturedEnd(0), SyntaxStateShift::out);
        return SyntaxBlock();
    }
    SyntaxState next = mSpecialStates.value(match.captured(2).toLower(), SyntaxState::Standard);
    return SyntaxBlock(this, match.capturedStart(1), match.capturedEnd(0), next
                       , !mDirectives.contains(match.captured(2), Qt::CaseInsensitive));
}


SyntaxTitle::SyntaxTitle()
{ }

SyntaxBlock SyntaxTitle::find(SyntaxState entryState, const QString& line, int index)
{
    Q_UNUSED(entryState);
    return SyntaxBlock(this, index, line.length(), SyntaxStateShift::out);
}


SyntaxCommentLine::SyntaxCommentLine(QChar commentChar): mCommentChar(commentChar)
{ }

SyntaxBlock SyntaxCommentLine::find(SyntaxState entryState, const QString& line, int index)
{
    Q_UNUSED(entryState)
    if (entryState == SyntaxState::CommentLine || (index==0 && line.startsWith(mCommentChar)))
        return SyntaxBlock(this, index, line.length(), SyntaxStateShift::out);
    return SyntaxBlock();
}


SyntaxCommentBlock::SyntaxCommentBlock()
{
    mSubStates << SyntaxState::Directive;
}

SyntaxBlock SyntaxCommentBlock::find(SyntaxState entryState, const QString& line, int index)
{
    Q_UNUSED(entryState)
    return SyntaxBlock(this, index, line.length());
}


} // namespace studio
} // namespace gams
