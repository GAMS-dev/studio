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
    TextMarkList(FileGroupContext* group, const QString &fileName);
    void unbind();
    void bind(FileContext* fc);
    void updateMarks();
    void rehighlight();
    QList<TextMark*> marksForBlock(QTextBlock block, TextMark::Type refType = TextMark::all);
    QList<TextMark*> marks() { return mMarks;}
    int textMarkCount(QSet<TextMark::Type> tmTypes);
    FileContext* fileContext();
    QTextDocument* document() const;
    FileContext* openFileContext();

signals:
    void getFileContext(QString filePath, FileContext** resultFile, FileGroupContext* fileGroup = nullptr);

public slots:
    void shareMarkHash(QHash<int, TextMark*>* marks);
    void textMarkIconsEmpty(bool* hasIcons);
    void documentOpened();
    void documentChanged(int pos, int charsRemoved, int charsAdded);

protected:
    friend class TextMark;
    friend class LogContext;
    friend class FileContext;
    friend class FileGroupContext;
    TextMark* generateTextMark(TextMark::Type tmType, int value, int line, int column, int size = 0);
    void removeTextMarks(QSet<TextMark::Type> tmTypes);
    void removeTextMark(TextMark* mark);
    QList<TextMark*> findMarks(const QTextCursor& cursor);
    TextMark* firstErrorMark();
    void connectDoc();

private:
    FileGroupContext* mGroupContext = nullptr;
    FileContext* mFileContext = nullptr;
    QString mFileName;
    QList<TextMark*> mMarks;
};

} // namespace studio
} // namespace gams

#endif // TEXTMARKLIST_H
