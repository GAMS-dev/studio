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
    void clearLog();
    bool mJumpToLogEnd = true;
public slots:
    void addProcessData(QProcess::ProcessChannel channel, QString text);
    void clearRecentMarks();
    void setJumpToLogEnd(bool state);

protected:
    friend class FileRepository;
    LogContext(FileId fileId, QString name);

    struct LinkData {
        TextMark* textMark = nullptr;
        int col = 0;
        int size = 1;
    };
    QString extractError(QString text, ExtractionState &state, QList<LinkData>& marks);

private:
    struct ErrorData {
        int lstLine = 0;
        int errNr = 0;
        QString text;
    };
    bool mInErrorDescription = false;
    QTextDocument *mDocument = nullptr;
    ErrorData mCurrentErrorHint;
    QSet<FileContext*> mMarkedContextList;
    QString mLineBuffer;

};

} // namespace studio
} // namespace gams

#endif // LOGCONTEXT_H
