#include "syntaxformats.h"
#include "logger.h"

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
    mRex.setPattern(QString("(^%1|%1%1)([\\w\\.]+)\\s*").arg(QRegularExpression::escape(directiveChar)));
    mDirectives << "dollar" << "ontext" << "title" << "hidden";
    mSpecialStates.insert("title", SyntaxState::Title);
    mSpecialStates.insert("ontext", SyntaxState::CommentBlock);
    mSpecialStates.insert("hidden", SyntaxState::CommentLine);
}

SyntaxBlock SyntaxDirective::find(SyntaxState entryState, const QString& line, int index)
{
    QRegularExpressionMatch match = mRex.match(line, index);
    if (!match.hasMatch()) return SyntaxBlock();
    if (entryState == SyntaxState::CommentBlock) {
//        DEB() << "Directive from CommentBlock";
        if (match.captured(2).compare("offtext", Qt::CaseInsensitive)==0)
            return SyntaxBlock(this, match.capturedStart(1), match.capturedEnd(0), SyntaxStateShift::out);
        return SyntaxBlock();
    }
    SyntaxState next = mSpecialStates.value(match.captured(2).toLower(), SyntaxState::Standard);
//    DEB() << match.captured(2) << " cap-end 2: " << match.capturedEnd(2) << "  cap-end 0: " << match.capturedEnd(0);
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
