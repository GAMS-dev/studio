#ifndef NAVIGATORCONTENT_H
#define NAVIGATORCONTENT_H

#include "file/filemeta.h"

namespace gams {
namespace studio {

class NavigatorContent
{
public:
    NavigatorContent();
    NavigatorContent(FileMeta* file, QString additionalText);
    NavigatorContent(QFileInfo file, QString additionalText);
    NavigatorContent(QString txt, QString additionalText, QString prefix, FileMeta* currentFile = nullptr);
    NavigatorContent(QString txt, std::function<void()> function);
    bool isValid();

    FileMeta* GetFileMeta();
    QFileInfo FileInfo();
    QString Text();
    QString AdditionalInfo();
    QString Prefix();
    void ExecuteQuickAction();

private:
    FileMeta* mFileMeta = nullptr;
    QFileInfo mFileInfo;
    QString mText;
    QString mAdditionalInfo;
    QString mInsertPrefix;
    std::function<void()> mFunction;
};

}
}
#endif // NAVIGATORCONTENT_H
