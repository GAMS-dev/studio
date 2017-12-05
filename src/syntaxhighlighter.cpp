#include "syntaxhighlighter.h"

namespace gams {
namespace studio {

SyntaxHighlighter::SyntaxHighlighter(QTextDocument *parent)
    :QSyntaxHighlighter(parent)
{
    mStates << new SyntaxStandard();
    mStates << new SyntaxDirective();
    mStates << new SyntaxCommentLine();
}

SyntaxHighlighter::~SyntaxHighlighter()
{
    while (!mStates.isEmpty()) {
        delete mStates.takeFirst();
    }
}

void SyntaxHighlighter::highlightBlock(const QString& text)
{
    int stateIdx = previousBlockState();
    SyntaxAbstract* syntax = stateIdx<0 ? mStates.at(0) : mStates.at(stateIdx);
    SyntaxState nextState = syntax->state();

    int index = 0;
    bool direct = false;
    while (index < text.length()) {
        int end = index;
        // get end of this syntax-state
        nextState = syntax->process(syntax->state(), text, index, end);
        SyntaxAbstract* candidate = syntax;
        // scan valid sub-states for nearest hit
        for (SyntaxState subState: syntax->subStates()) {
            SyntaxAbstract* subSyntax = getSyntax(stateIdx, subState);
            if (subSyntax) {
                int subStart = index;
                int subEnd = index;
                SyntaxState locState = subSyntax->process(syntax->state(), text, subStart, subEnd);

                if (subStart == index && subEnd > index) {
                    // another state starts at index and has a length -> highlight and jump to end
                    if (subSyntax->charFormat())
                        setFormat(subStart, subEnd-subStart, *subSyntax->charFormat());
                    nextState = subSyntax->state();
                    index = subEnd;
                    direct = true;
                    break;
                } else if (subStart > index && subStart < end) {
                    end = subStart;
                    candidate = subSyntax;
                    nextState = locState;
                }
            }
        }
        if (!direct) {
            if (syntax->charFormat())
                setFormat(index, end-index, *syntax->charFormat());
        }
    }
    if (nextState == SyntaxState::Standard) {
        setCurrentBlockState(getStateIdx(stateIdx, nextState));
    }
}

SyntaxAbstract*SyntaxHighlighter::getSyntax(int maxIdx, SyntaxState state) const
{
    while (maxIdx > 0) {
        --maxIdx;
        if (mStates.at(maxIdx)->state() == state) return mStates.at(maxIdx);
    }
    return nullptr;
}

int SyntaxHighlighter::getStateIdx(int maxIdx, SyntaxState state) const
{
    while (maxIdx > 0) {
        --maxIdx;
        if (mStates.at(maxIdx)->state() == state) return maxIdx;
    }
    return -1;
}

} // namespace studio
} // namespace gams
