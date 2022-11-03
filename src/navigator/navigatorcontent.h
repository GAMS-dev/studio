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
    NavigatorContent(QString txt, QString additionalText, QString prefix);
    bool isValid();

    FileMeta* GetFileMeta();
    QFileInfo FileInfo();
    QString Text();
    QString AdditionalInfo();
    QString Prefix();

private:
    FileMeta* mFileMeta = nullptr;
    QFileInfo mFileInfo;
    QString mText;
    QString mAdditionalInfo;
    QString mInsertPrefix;
};

}
}
#endif // NAVIGATORCONTENT_H
