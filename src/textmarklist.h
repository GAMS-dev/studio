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
    void updateMarks();
    QList<TextMark*> marksForBlock(QTextBlock block);

public slots:
    void shareMarkHash(QHash<int, TextMark*>* marks);
    void textMarksEmpty(bool* empty);

protected:
    friend class LogContext;
    friend class FileContext;
    TextMark* generateTextMark(FileContext *context, gams::studio::TextMark::Type tmType, int value, int line, int column, int size = 0);
    void removeTextMarks(QSet<TextMark::Type> tmTypes);
    TextMark* findMark(const QTextCursor& cursor);
    TextMark* firstErrorMark();

private:
    QList<TextMark*> mTextMarks;
};

} // namespace studio
} // namespace gams

#endif // TEXTMARKLIST_H
