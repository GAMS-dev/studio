#ifndef FILECONTEXT_H
#define FILECONTEXT_H

#include <QString>
#include <QWidget>
#include <QTextEdit>

// TODO(JM) This is a stubb file that needs to be reimplemented or at least reviewed

namespace ide {

enum class FileType {
    ftGms,
    ftLog,
    ftLst,
    ftLxi,
};

class FileContext
{
    FileContext();

    FileType fileType = FileType::ftGms;
    QString fileName = "";
    QWidget* tabWidget = nullptr;
    QTextEdit* textEdit = nullptr;
    bool changed = false;
};

} // namespace ide

#endif // FILECONTEXT_H
