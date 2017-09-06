#ifndef FILECONTEXT_H
#define FILECONTEXT_H

#include <QString>
#include <QWidget>
#include <QTextEdit>
#include <QFileInfo>
#include <QTabWidget>
#include "codeeditor.h"

// TODO(JM) This is a stubb file that needs to be reimplemented or at least reviewed

namespace ide {

enum class FileType {
    ftGms,
    ftTxt,
    ftInc,
    ftLog,
    ftLst,
    ftLxi,
};

class FileContext
{
public:
    FileContext(QString fileName);
    bool isEmpty();
    ide::CodeEditor* createEditor(QTabWidget *tabWidget);
    bool exist();
    /// Loads the file content into the editor
    bool load();

private:
    FileType mFileType = FileType::ftGms;
    QFileInfo mFileInfo;
    int mTabIndex = -1;
    ide::CodeEditor* mTextEdit = nullptr;
    bool mChanged = false;
};

} // namespace ide

#endif // FILECONTEXT_H
