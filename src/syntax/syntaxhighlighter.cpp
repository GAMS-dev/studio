#include "syntaxhighlighter.h"
#include "logger.h"
#include "filecontext.h"
#include "textmark.h"

namespace gams {
namespace studio {

ErrorHighlighter::ErrorHighlighter(FileContext* context, TextMarkList *marks)
    : QSyntaxHighlighter(context->document()), mMarks(marks), mContext(context)
{
    if (!document()) return;
    connect(document(), &QTextDocument::contentsChange, this, &ErrorHighlighter::docContentsChange);
    connect(document(), &QTextDocument::blockCountChanged, this, &ErrorHighlighter::docBlockCountChanged);
}

void ErrorHighlighter::setDocAndConnect(QTextDocument* doc)
{
    if (document() && doc != document()) {
        disconnect(document(), &QTextDocument::contentsChange, this, &ErrorHighlighter::docContentsChange);
        disconnect(document(), &QTextDocument::blockCountChanged, this, &ErrorHighlighter::docBlockCountChanged);
    }
    setDocument(doc);
    if (doc) {
        connect(document(), &QTextDocument::contentsChange, this, &ErrorHighlighter::docContentsChange);
        connect(document(), &QTextDocument::blockCountChanged, this, &ErrorHighlighter::docBlockCountChanged);
        if (mMarks) mMarks->rehighlight();
    }
}

void ErrorHighlighter::highlightBlock(const QString& text)
{
    if (!mMarks) {
        DEB() << "trying to highlight without marks!";
        return;
    }
    QList<TextMark*> marks = mMarks->marksForBlock(currentBlock());
    setCombiFormat(0, text.length(), QTextCharFormat(), marks);
}

void ErrorHighlighter::setCombiFormat(int start, int len, const QTextCharFormat &format, QList<TextMark*> marks)
{
    int end = start+len;
    int marksStart = end;
    int marksEnd = start;
    for (TextMark* mark: marks) {
        if (mark->blockStart() < marksStart) marksStart = qMax(start, mark->blockStart());
        if (mark->blockEnd() > marksEnd) marksEnd = qMin(end, mark->blockEnd());
    }
    if (marksStart == end) {
        setFormat(start, len, format);
        return;
    }
    if (marksStart > start) setFormat(start, marksStart-start, format);
    if (marksEnd < end) setFormat(marksEnd, end-marksEnd, format);
    start = marksStart;
    end = marksEnd;

    for (TextMark* mark: marks) {
        if (mark->blockStart() >= end || mark->blockEnd() < start)
            continue;
        QTextCharFormat combinedFormat(format);
        marksStart = qMax(mark->blockStart(), start);
        marksEnd = qMin(mark->blockEnd(), end);
        if (mark->type() == TextMark::error) {
            combinedFormat.setUnderlineColor(Qt::red);
            combinedFormat.setUnderlineStyle(QTextCharFormat::WaveUnderline);
            combinedFormat.setAnchorName(QString::number(mark->line()));
            setFormat(marksStart, marksEnd-marksStart, combinedFormat);
            if (marksEnd == mark->blockEnd()) {
                combinedFormat.setBackground(QColor(225,200,255));
                combinedFormat.setUnderlineStyle(QTextCharFormat::SingleUnderline);
                setFormat(marksEnd, 1, combinedFormat);
            }
        }
        if (mark->type() == TextMark::link) {
            combinedFormat.setForeground(mark->color());
            combinedFormat.setUnderlineColor(mark->color());
            combinedFormat.setUnderlineStyle(QTextCharFormat::SingleUnderline);
            combinedFormat.setAnchor(true);
            combinedFormat.setAnchorName(QString::number(mark->line()));
            setFormat(marksStart, marksEnd-marksStart, combinedFormat);
        }
        if (mark->type() == TextMark::result) {
            combinedFormat.setBackground(mark->color());
            setFormat(marksStart, marksEnd - marksStart, combinedFormat);
        }

    }
}

void ErrorHighlighter::docBlockCountChanged(int newCount)
{
    Q_UNUSED(newCount);
    if (!mMarks) return;
    QList<TextMark*> marks = mMarks->marksForBlock(currentBlock());
    for (TextMark* mark: marks) {
        mark->modified();
    }
}

void ErrorHighlighter::docContentsChange(int from, int removed, int added)
{
    Q_UNUSED(removed)
    Q_UNUSED(added)
    if (!mMarks) return;
    // TODO(JM) check for use of removed and added to remove display
    QList<TextMark*> marks = mMarks->marksForBlock(document()->findBlock(from));
    for (TextMark* mark: marks) {
        mark->modified();
    }
}

SyntaxHighlighter::SyntaxHighlighter(FileContext* context, TextMarkList* marks)
    : ErrorHighlighter(context, marks)
{
    SyntaxAbstract* syntax = new SyntaxStandard();
    addState(syntax);

    syntax = new SyntaxDirective();
    syntax->charFormat().setForeground(Qt::darkMagenta);
    addState(syntax);

    syntax = new SyntaxTitle();
    syntax->charFormat().setForeground(Qt::darkBlue);
    syntax->charFormat().setFontWeight(QFont::Bold);
    syntax->charFormat().setFontItalic(true);
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
    ErrorHighlighter::highlightBlock(text);
    QList<TextMark*> marks = mMarks ? mMarks->marksForBlock(currentBlock()) : QList<TextMark*>();
    int code = previousBlockState();
    int index = 0;
    while (index < text.length()) {
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
                setCombiFormat(thisBlock.start, thisBlock.length(), thisBlock.syntax->charFormat(), marks);

            if (thisBlock.error && thisBlock.length() > 0)
                setCombiFormat(thisBlock.start, thisBlock.length(), thisBlock.syntax->charFormatError(), marks);

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
    } else if (currentBlockState() != -1) {
        setCurrentBlockState(-1);
    }
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
