#include "syntaxhighlighter.h"
#include "logger.h"

namespace gams {
namespace studio {

SyntaxHighlighter::SyntaxHighlighter(QTextDocument *parent)
    :QSyntaxHighlighter(parent)
{
    SyntaxAbstract* syntax = new SyntaxStandard();
    mStates << syntax;

    syntax = new SyntaxDirective();
    syntax->charFormat().setForeground(Qt::darkBlue);
    syntax->charFormat().setFontWeight(QFont::Bold);
    mStates << syntax;

    syntax = new SyntaxCommentLine();
    syntax->charFormat().setForeground(Qt::darkGreen);
    syntax->charFormat().setFontItalic(true);
    mStates << syntax;

    syntax = new SyntaxCommentBlock();
    syntax->copyCharFormat(mStates.last()->charFormat());
    mStates << syntax;

    mLastBaseState = mStates.length();
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
    while (index < text.length()-1) {
        // TODO(JM) use SyntaxBlock instead
        SyntaxBlock thisBlock = syntax->find(syntax->state(), text, index);
        if (!thisBlock.isValid()) {
            DEB() << "Syntax not found for type " << int(syntax->state());
            index++;
            continue;
        }
        SyntaxBlock nextBlock;
        for (SyntaxState subState: syntax->subStates()) {
            SyntaxAbstract* subSyntax = getSyntax(stateIdx, subState);
            if (subSyntax) {
                SyntaxBlock subBlock = subSyntax->find(syntax->state(), text, index);
                if (subBlock.isValid()) {
                    if (!nextBlock.isValid() || nextBlock.start > subBlock.start)
                        nextBlock = subBlock;
                }
            }
        }
        if (nextBlock.isValid()) {
            // new state inside current block -> shorten end
            if (nextBlock.start < thisBlock.end) thisBlock.end = nextBlock.start;
            // current block has zero size
            if (thisBlock.length()<1) thisBlock = nextBlock;
        }
        if (thisBlock.isValid() && thisBlock.length()>0) {
            if (thisBlock.syntax->state() != SyntaxState::Standard)
                setFormat(thisBlock.start, thisBlock.length(), thisBlock.syntax->charFormat());
            if (thisBlock.error && thisBlock.length()>0)
                setFormat(thisBlock.start, thisBlock.length(), thisBlock.syntax->charFormatError());
            index = thisBlock.end;
            nextState = thisBlock.next;
            DEB() << "line " << currentBlock().blockNumber() << ": marked state " << int(thisBlock.syntax->state()) << " - next is " << int(nextState);
        } else {
            index++;
        }
    }
    if (nextState != SyntaxState::Standard) {
        setCurrentBlockState(getStateIdx(stateIdx, nextState));
        DEB() << "currentBlockState set to " << currentBlockState();
    } else if (currentBlockState() != -1) {
        setCurrentBlockState(-1);
    }
}

SyntaxAbstract*SyntaxHighlighter::getSyntax(int maxIdx, SyntaxState state) const
{
    int i = qMax(maxIdx, mLastBaseState);
    while (i > 0) {
        --i;
        if (mStates.at(i)->state() == state) return mStates.at(i);
    }
    return nullptr;
}

int SyntaxHighlighter::getStateIdx(int maxIdx, SyntaxState state) const
{
    int i = qMax(maxIdx, mLastBaseState);
    while (i > 0) {
        --i;
        if (mStates.at(i)->state() == state) return i;
    }
    return -1;
}

} // namespace studio
} // namespace gams
