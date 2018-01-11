#ifndef TEXTMARKLIST_H
#define TEXTMARKLIST_H

#include <QtCore>
#include "textmark.h"

namespace gams {
namespace studio {

class FileContext;

class TextMarkList: public QObject
{
    Q_OBJECT
public:
    TextMarkList();
    TextMarkList(const TextMarkList &marks);
    void updateMarks();
    void rehighlight();
    QList<TextMark*> marksForBlock(QTextBlock block, TextMark::Type refType = TextMark::all);
    QList<TextMark*> marks() { return mTextMarks;}

public slots:
    void shareMarkHash(QHash<int, TextMark*>* marks);
    void textMarksEmpty(bool* empty);

protected:
    friend class LogContext;
    friend class FileContext;
    friend class FileGroupContext;
    TextMark* generateTextMark(FileContext *context, gams::studio::TextMark::Type tmType, int value, int line, int column, int size = 0);
    TextMark* generateTextMark(QString fileName, FileGroupContext *group, gams::studio::TextMark::Type tmType, int value, int line, int column, int size = 0);
    void removeTextMarks(QSet<TextMark::Type> tmTypes);
    QList<TextMark*> findMarks(const QTextCursor& cursor);
    void merge(const TextMarkList &marks);
    TextMark* firstErrorMark();

private:
    QList<TextMark*> mTextMarks;
};

} // namespace studio
} // namespace gams

#endif // TEXTMARKLIST_H
