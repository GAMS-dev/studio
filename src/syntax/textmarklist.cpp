#include "textmarklist.h"

namespace gams {
namespace studio {

TextMarkList::TextMarkList()
{}

TextMarkList::TextMarkList(const TextMarkList& marks)
{
    // deep copy
    for (TextMark *mark: marks.mTextMarks) {
        mTextMarks.append(new TextMark(*mark));
    }
}

void TextMarkList::updateMarks()
{
    for (TextMark* mark: mTextMarks) {
        mark->updateCursor();
        // TODO(JM) Read the more specific error message from lst-file
        mark->rehighlight();
    }
}

void TextMarkList::rehighlight()
{
    for (TextMark* mark: mTextMarks) {
        mark->rehighlight();
    }
}

void TextMarkList::shareMarkHash(QHash<int, TextMark*>* marks)
{
    for (TextMark* mark: mTextMarks) {
        marks->insert(mark->line(), mark);
    }
}

void TextMarkList::textMarksEmpty(bool* empty)
{
    *empty = mTextMarks.isEmpty();
}

TextMark*TextMarkList::generateTextMark(FileContext* context, studio::TextMark::Type tmType, int value, int line, int column, int size)
{
    TextMark* res = new TextMark(tmType);
    res->setPosition(context, line, column, size);
    res->setValue(value);
    mTextMarks << res;
    return res;
}

TextMark*TextMarkList::generateTextMark(QString fileName, FileGroupContext* group, TextMark::Type tmType, int value, int line, int column, int size)
{
    TextMark* res = new TextMark(tmType);
    res->setPosition(fileName, group, line, column, size);
    res->setValue(value);
    mTextMarks << res;
    return res;
}

void TextMarkList::removeTextMarks(QSet<TextMark::Type> tmTypes)
{
    int i = mTextMarks.size();
    while (i > 0) {
        --i;
        TextMark* tm = mTextMarks.at(i);
        if (tmTypes.isEmpty() || tmTypes.contains(tm->type()) || tmTypes.contains(TextMark::all)) {
            delete mTextMarks.takeAt(i);
        }
    }
}

QList<TextMark*> TextMarkList::findMarks(const QTextCursor& cursor)
{
    QList<TextMark*> res;
    for (TextMark* mark: mTextMarks) {
        QTextCursor tc = mark->textCursor();
        if (tc.isNull()) break;
        if (tc.blockNumber() > cursor.blockNumber()) break;
        if (tc.blockNumber() < cursor.blockNumber()) continue;
        if (cursor.atBlockStart())
            res << mark;

        int a = tc.block().position() + mark->column();
        int b = a + (mark->size() ? mark->size() : tc.block().length());
        if (cursor.position() >= b) continue;
        if (cursor.position() >= a && (cursor.selectionEnd() < b))
            res << mark;
    }
    return res;
}

void TextMarkList::merge(const TextMarkList& marks)
{
    for (TextMark *mark: marks.mTextMarks) {
        if (!mTextMarks.contains(mark))
            mTextMarks.append(mark);
    }
}

TextMark*TextMarkList::firstErrorMark()
{
    for (TextMark* mark: mTextMarks)
        if (mark->isErrorRef()) return mark;
    return nullptr;
}

QList<TextMark*> TextMarkList::marksForBlock(QTextBlock block, TextMark::Type refType)
{
    QList<TextMark *> marks;
    for (TextMark* tm: mTextMarks) {
        int hit = tm->in(block.position(), block.length());
        if (hit > 0) break;
        if (hit == 0 && (refType == TextMark::all || refType == tm->refType())) {
            marks << tm;
        }
    }
    return marks;
}


} // namespace source
} // namespace gams
