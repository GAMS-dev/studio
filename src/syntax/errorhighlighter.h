#ifndef ERRORHIGHLIGHTER_H
#define ERRORHIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include "syntaxformats.h"
#include "textmarkrepo.h"

namespace gams {
namespace studio {

class ErrorHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT
public:
    ErrorHighlighter(QTextDocument *doc);
    void highlightBlock(const QString &text);
    const LineMarks *marks() const;
    void setMarks(const LineMarks *marks);

public slots:
    void syntaxState(int position, int &intState);

protected:
    void setCombiFormat(int start, int len, const QTextCharFormat& charFormat, QList<TextMark *> markList);

protected:
    int mPositionForSyntaxState = -1;
    int mLastSyntaxState = 0;

private:
    const LineMarks* mMarks = nullptr;
    QTextBlock mTestBlock;

};

} // namespace studio
} // namespace gams

#endif // ERRORHIGHLIGHTER_H
