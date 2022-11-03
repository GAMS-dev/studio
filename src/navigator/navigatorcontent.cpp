#include "navigatorcontent.h"

namespace gams {
namespace studio {

NavigatorContent::NavigatorContent() { }

// known files
NavigatorContent::NavigatorContent(FileMeta* file, QString additionalText) {
    mFileMeta = file;
    if (file) mFileInfo = QFileInfo(file->location());
    mAdditionalInfo = additionalText;
}

// unknown files
NavigatorContent::NavigatorContent(QFileInfo file, QString additionalText) {
    mFileInfo = file;
    mAdditionalInfo = additionalText;
}

// help content
NavigatorContent::NavigatorContent(QString txt, QString additionalText, QString prefix) {
    mText = txt;
    mAdditionalInfo = additionalText;
    mInsertPrefix = prefix;
}

bool NavigatorContent::isValid()
{
    return !mAdditionalInfo.isEmpty();
}

FileMeta *NavigatorContent::GetFileMeta()
{
    return mFileMeta;
}

QFileInfo NavigatorContent::FileInfo()
{
    return mFileInfo;
}

QString NavigatorContent::Text()
{
    return mText;
}

QString NavigatorContent::AdditionalInfo()
{
    return mAdditionalInfo;
}

QString NavigatorContent::Prefix()
{
    return mInsertPrefix;
}

}
}
