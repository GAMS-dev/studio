#ifndef LOGCONTEXT_H
#define LOGCONTEXT_H

#include "filecontext.h"

namespace gams {
namespace studio {

class LogContext : public FileContext
{
public:
    void markOld();
    QTextDocument* document() override;
    void addEditor(QPlainTextEdit *edit) override;
    void removeEditor(QPlainTextEdit *edit) override;
    void setParentEntry(FileGroupContext *parent) override;
    TextMark* firstErrorMark();

public slots:
    void addProcessData(QProcess::ProcessChannel channel, QString text);
    void clearRecentMarks();

protected:
    friend class FileRepository;
    LogContext(int id, QString name);

    struct LinkData {
        TextMark* textMark = nullptr;
        int col = 0;
        int size = 1;
    };
    QString extractError(QString text, ExtractionState &state, QList<LinkData>& marks);

private:
    bool mInErrorDescription = false;
    QTextDocument *mDocument = nullptr;
    QPair<int, QString> mCurrentErrorHint;
    QSet<FileContext*> mMarkedContextList;
    QString mLineBuffer;

};

} // namespace studio
} // namespace gams

#endif // LOGCONTEXT_H
