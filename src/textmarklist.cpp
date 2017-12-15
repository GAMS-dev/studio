#include "textmarklist.h"

namespace gams {
namespace studio {

TextMarkList::TextMarkList()
{}

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

void TextMarkList::removeTextMarks(QSet<TextMark::Type> tmTypes)
{
    int i = mTextMarks.size();
    while (i > 0) {
        --i;
        TextMark* tm = mTextMarks.at(i);
        if (tmTypes.contains(tm->type()) || tmTypes.contains(TextMark::all)) {
            delete mTextMarks.takeAt(i);
        }
    }
}

TextMark*TextMarkList::findMark(const QTextCursor& cursor)
{
    for (TextMark* mark: mTextMarks) {
        QTextCursor tc = mark->textCursor();
        if (tc.isNull()) break;
        if (tc.blockNumber() > cursor.blockNumber()) break;
        if (tc.blockNumber() < cursor.blockNumber()) continue;
        if (cursor.atBlockStart())
            return mark;

        int a = tc.block().position() + mark->column();
        int b = a + (mark->size() ? mark->size() : tc.block().length());
        if (cursor.position() >= b) continue;
        if (cursor.position() >= a && (cursor.selectionEnd() < b))
            return mark;
    }
    return nullptr;
}

TextMark*TextMarkList::firstErrorMark()
{
    for (TextMark* mark: mTextMarks)
        if (mark->isErrorRef()) return mark;
    return nullptr;
}

QList<TextMark*> TextMarkList::marksForBlock(QTextBlock block)
{
    QList<TextMark *> marks;
    for (TextMark* tm: mTextMarks) {
        int hit = tm->in(block.position(), block.length());
        if (hit > 0) break;
        if (hit == 0) {
            marks << tm;
        }
    }
    return marks;
}

} // namespace source
} // namespace gams
