#include "syntaxhighlighter.h"
#include "logger.h"

namespace gams {
namespace studio {


ErrorHighlighter::ErrorHighlighter(QTextDocument* parent)
    : QSyntaxHighlighter(parent)
{
    mTestBlock = parent->findBlockByNumber(3);
    connect(parent, &QTextDocument::contentsChange, this, &ErrorHighlighter::docContentsChange);
    connect(parent, &QTextDocument::blockCountChanged, this, &ErrorHighlighter::docBlockCountChanged);
}

void ErrorHighlighter::highlightBlock(const QString& text)
{
    QTextBlockUserData *data = currentBlock().userData();
    DEB() << "BlockData: " << mTestBlock.isValid()
          << " / " << mTestBlock.blockNumber()
          << " /p " << mTestBlock.position()
          << " /l " << mTestBlock.length();
    DEB() << "           " << mTestBlock.text();
    // TODO(JM) analyze data for marks
}

void ErrorHighlighter::docBlockCountChanged(int newCount)
{
    TRACE();
}

void ErrorHighlighter::docContentsChange(int from, int removed, int added)
{
    TRACE();
}


SyntaxHighlighter::SyntaxHighlighter(QTextDocument *parent)
    : ErrorHighlighter(parent)
{
    SyntaxAbstract* syntax = new SyntaxStandard();
    addState(syntax);

    syntax = new SyntaxDirective();
    syntax->charFormat().setForeground(Qt::darkMagenta);
//    syntax->charFormat().setFontWeight(QFont::Bold);
    addState(syntax);

    syntax = new SyntaxTitle();
    syntax->charFormat().setForeground(Qt::blue);
    syntax->charFormat().setFontWeight(QFont::Medium);
    syntax->charFormat().setFont(syntax->charFormat().font());
    syntax->charFormat().setFontStretch(120);
    addState(syntax);

    syntax = new SyntaxCommentLine();
    syntax->charFormat().setForeground(Qt::darkGreen);
    syntax->charFormat().setFontItalic(true);
    addState(syntax);

    syntax = new SyntaxCommentBlock();
    syntax->copyCharFormat(mStates.last()->charFormat());
    addState(syntax);

    syntax = new SyntaxDeclaration(SyntaxState::Declaration);
    syntax->charFormat().setForeground(Qt::darkBlue);
    syntax->charFormat().setFontWeight(QFont::Bold);
    addState(syntax);

    syntax = new SyntaxDeclaration(SyntaxState::DeclarationSetType);
    syntax->copyCharFormat(mStates.last()->charFormat());
    addState(syntax);

    syntax = new SyntaxDeclaration(SyntaxState::DeclarationVariableType);
    syntax->copyCharFormat(mStates.last()->charFormat());
    addState(syntax);
}

SyntaxHighlighter::~SyntaxHighlighter()
{
    while (!mStates.isEmpty()) {
        delete mStates.takeFirst();
    }
}

void SyntaxHighlighter::highlightBlock(const QString& text)
{

    int code = previousBlockState();

    int index = 0;
    while (index < text.length()-1) {
        StateCode stateCode = (code < 0) ? mCodes.at(0) : mCodes.at(code);
        SyntaxAbstract* syntax = mStates.at(stateCode.first);

        SyntaxBlock thisBlock = syntax->find(syntax->state(), text, index);
        if (!thisBlock.isValid()) {
            DEB() << "Syntax not found for type " << int(syntax->state());
            index++;
            continue;
        }
        SyntaxBlock nextBlock;
        for (SyntaxState subState: syntax->subStates()) {
            // TODO(JM) store somehow when states have been redefined with directives
            // (e.g. StdState with indexes to changed subsetStates)
            SyntaxAbstract* subSyntax = getSyntax(subState);
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
        if (thisBlock.isValid() && thisBlock.length() > 0) {
            if (thisBlock.syntax->state() != SyntaxState::Standard)
                setFormat(thisBlock.start, thisBlock.length(), thisBlock.syntax->charFormat());
            if (thisBlock.error && thisBlock.length()>0)
                setFormat(thisBlock.start, thisBlock.length(), thisBlock.syntax->charFormatError());
            index = thisBlock.end;

            code = getCode(code, thisBlock.shift, getStateIdx(thisBlock.next));

//            stateCode = (code < 0) ? mCodes.at(0) : mCodes.at(code);
//            DEB() << "line " << currentBlock().blockNumber() << ": marked state " << int(thisBlock.syntax->state())
//                  << " - next is " << int(mStates.at(stateCode.first)->state());
        } else {
            index++;
        }
    }
    StateCode stateCode = (code < 0) ? mCodes.at(0) : mCodes.at(code);
    if (mStates.at(stateCode.first)->state() != SyntaxState::Standard) {
        setCurrentBlockState(code);
        DEB() << "currentBlockState set to " << currentBlockState();
    } else if (currentBlockState() != -1) {
        setCurrentBlockState(-1);
    }
    ErrorHighlighter::highlightBlock(text);
}

SyntaxAbstract*SyntaxHighlighter::getSyntax(SyntaxState state) const
{
    int i = mStates.length();
    while (i > 0) {
        --i;
        if (mStates.at(i)->state() == state) return mStates.at(i);
    }
    return nullptr;
}

int SyntaxHighlighter::getStateIdx(SyntaxState state) const
{
    int i = mStates.length();
    while (i > 0) {
        --i;
        if (mStates.at(i)->state() == state) return i;
    }
    return -1;
}

void SyntaxHighlighter::addState(SyntaxAbstract* syntax, CodeIndex ci)
{
    addCode(mStates.length(), ci);
    mStates << syntax;
}

int SyntaxHighlighter::addCode(StateIndex si, CodeIndex ci)
{
    StateCode sc(si, ci);
    int index = mCodes.indexOf(sc);
    if (index >= 0)
        return index;
    DEB() << mCodes.length() << ": added state " << si << " in code " << ci;
    mCodes << sc;
    return mCodes.length()-1;
}

int SyntaxHighlighter::getCode(CodeIndex code, SyntaxStateShift shift, StateIndex state)
{
    if (code < 0) code = 0;
    if (shift == SyntaxStateShift::out)
        return mCodes.at(code).second;
    if (shift == SyntaxStateShift::in)
        return addCode(state, code);
    return code;
}


} // namespace studio
} // namespace gams
