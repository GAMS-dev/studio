#include "syntaxdeclaration.h"
#include "logger.h"
#include "exception.h"

namespace gams {
namespace studio {

SyntaxDeclaration::SyntaxDeclaration(SyntaxState state): mState(state)
{
    QStringList list;
    switch (state) {
    case SyntaxState::DeclarationSetType:
        list = QStringList() << "Singleton";
        mKeywords.insert(state, new DictList(list));
        mSubStates << SyntaxState::Declaration << SyntaxState::CommentEndline << SyntaxState::CommentInline;
        break;
    case SyntaxState::DeclarationVariableType:
        list = QStringList() << "free" << "positive" << "nonnegative" << "negative"
                             << "binary" << "integer" << "sos1" << "sos2" << "semicont" << "semiint";
        mKeywords.insert(state, new DictList(list));
        mSubStates << SyntaxState::Declaration << SyntaxState::CommentEndline << SyntaxState::CommentInline;
        break;
    case SyntaxState::Declaration:
        list = QStringList() << "Table" << "Scalar" << "Scalars" << "Acronym" << "Alias" << "Set" << "Sets"
                             << "Variable" << "Variables" << "Parameter" << "Parameters" << "Equation" << "Equations"
                             << "Model" << "Solve" << "Display";
        mKeywords.insert(state, new DictList(list));

        list = QStringList() << "Set" << "Sets";
        mKeywords.insert(SyntaxState::DeclarationSetType, new DictList(list));

        list = QStringList() << "Variable" << "Variables";
        mKeywords.insert(SyntaxState::DeclarationVariableType, new DictList(list));
        mSubStates << SyntaxState::Identifier << SyntaxState::CommentEndline << SyntaxState::CommentInline;
        break;
    default:
        FATAL() << "invalid SyntaxState to initialize SyntaxDeclaration";
        break;
    }
}

SyntaxDeclaration::~SyntaxDeclaration()
{
    while (!mKeywords.isEmpty())
        delete mKeywords.take(mKeywords.keys().first());
}

QStringList SyntaxDeclaration::swapStringCase(QStringList list)
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


SyntaxBlock SyntaxDeclaration::find(SyntaxState entryState, const QString& line, int index)
{
    // skip whitespaces
    int start = index;
    int end = -1;
    while (isWhitechar(line, start))
        ++start;
    if (entryState != state()) {
        if (entryState == SyntaxState::DeclarationSetType || entryState == SyntaxState::DeclarationVariableType) {
            end = findEnd(entryState, line, start);

//            check syntax coordination;


            if (end > start)
                return SyntaxBlock(this, start, end, SyntaxStateShift::out);
            else
                return SyntaxBlock(this, start, end, SyntaxStateShift::stay, true);
        } else {
            end = findEnd(state(), line, start);
            if (end > start) {
                // TODO(JM) if not in SyntaxState::Declaration mark following as error if not an SyntaxState::Declaration keyword
                if (state() != SyntaxState::Declaration)
                    return SyntaxBlock(this, start, end, SyntaxState::Declaration);
                else
                    return SyntaxBlock(this, start, end, SyntaxStateShift::out);
            }
        }
    } else {
        end = findEnd(state(), line, start);
        if (end > start)
            return SyntaxBlock(this, start, end, SyntaxStateShift::out);
        else {
            return SyntaxBlock(this, start, start, SyntaxStateShift::out);
        }
    }
    return SyntaxBlock();
}

//bool SyntaxDeclaration::isWhitechar(const QString& line, int index)
//{
//    return index<line.length() && (line.at(index).category()==QChar::Separator_Space
//                                   || line.at(index) == '\t' || line.at(index) == '\n' || line.at(index) == '\r');
//}

int SyntaxDeclaration::findEnd(SyntaxState state, const QString& line, int index)
{
    int iKey = 0;
    int iChar = 0;
    while (true) {
        if ((isWhitechar(line, iChar+index) || (iChar > 0 && line.length() <= iChar+index))) {
            if (mKeywords.value(state)->at(iKey).length() > iChar) return -1;
            return iChar+index; // reached an valid end
        } else if (mKeywords.value(state)->at(iKey).length() > iChar
                   &&  mKeywords.value(state)->at(iKey).is(line.at(iChar+index), iChar) ) {
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


} // namespace studio
} // namespace gans
