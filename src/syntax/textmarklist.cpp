#include "textmarklist.h"
#include "filecontext.h"

namespace gams {
namespace studio {

TextMarkList::TextMarkList()
    : QObject()
{}

TextMarkList::TextMarkList(const TextMarkList& marks)
    : QObject()
{
    // deep copy
    for (TextMark *mark: marks.mMarks) {
        mMarks.append(new TextMark(*mark));
    }
}

void TextMarkList::unbindFileContext()
{
    for (TextMark *mark: mMarks) {
        mark->unbindFileContext();
    }
}

void TextMarkList::updateMarks()
{
    for (TextMark* mark: mMarks) {
        mark->updateCursor();
        // TODO(JM) Read the more specific error message from lst-file
        mark->rehighlight();
    }
}

void TextMarkList::rehighlight()
{
    for (TextMark* mark: mMarks) {
        mark->rehighlight();
    }
}

void TextMarkList::shareMarkHash(QHash<int, TextMark*>* marks)
{
    for (TextMark* mark: mMarks) {
        marks->insert(mark->line(), mark);
    }
}

void TextMarkList::textMarksEmpty(bool* empty)
{
    *empty = mMarks.isEmpty();
}

TextMark*TextMarkList::generateTextMark(FileContext* context, studio::TextMark::Type tmType, int value, int line, int column, int size)
{
    TextMark* res = new TextMark(tmType);
    res->setPosition(context, line, column, size);
    res->setValue(value);
    mMarks << res;
    return res;
}

TextMark*TextMarkList::generateTextMark(QString fileName, FileGroupContext* group, TextMark::Type tmType, int value, int line, int column, int size)
{
    TextMark* res = new TextMark(tmType);
    res->setPosition(fileName, group, line, column, size);
    res->setValue(value);
    mMarks << res;
    return res;
}

int TextMarkList::textMarkCount(QSet<TextMark::Type> tmTypes)
{
    int i = mMarks.size();
    int res = 0;
    while (i > 0) {
        --i;
        TextMark* tm = mMarks.at(i);
        if (tmTypes.contains(tm->type()) || tmTypes.contains(TextMark::all)) {
            res++;
        }
    }

    return res;
}

void TextMarkList::removeTextMarks(QSet<TextMark::Type> tmTypes)
{
    int i = mMarks.size();
    while (i > 0) {
        --i;
        TextMark* tm = mMarks.at(i);
        if (tmTypes.isEmpty() || tmTypes.contains(tm->type()) || tmTypes.contains(TextMark::all)) {
            int pos = tm->position();
            FileContext* file = tm->fileContext();
            delete mMarks.takeAt(i);
            if (file) file->rehighlightAt(pos);
        }
    }
}

QList<TextMark*> TextMarkList::findMarks(const QTextCursor& cursor)
{
    QList<TextMark*> res;
    for (TextMark* mark: mMarks) {
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
    for (TextMark *mark: marks.mMarks) {
        if (!mMarks.contains(mark))
            mMarks.append(mark);
    }
}

TextMark*TextMarkList::firstErrorMark()
{
    for (TextMark* mark: mMarks)
        if (mark->isErrorRef()) return mark;
    return nullptr;
}

QList<TextMark*> TextMarkList::marksForBlock(QTextBlock block, TextMark::Type refType)
{
    QList<TextMark *> marks;
    for (TextMark* tm: mMarks) {
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
