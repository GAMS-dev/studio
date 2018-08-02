#include <QDir>
#include <QTime>
#include <QDebug>

#include "gamsproperties.h"
#include "exception.h"
#include "option/option.h"

namespace gams {
namespace studio {

GamsProperties::GamsProperties(FileId origin) : mFileId(origin)
{
    // set default path
    mGamsArgs.insert("lo", "3");
    mGamsArgs.insert("ide", "1");
    mGamsArgs.insert("er", "99");
    mGamsArgs.insert("errmsg", "1");
    mGamsArgs.insert("pagesize", "0");
    mGamsArgs.insert("LstTitleLeftAligned", "1");
}

QStringList GamsProperties::analyzeParameters(const QString &inputFile, QList<OptionItem> itemList)
{
    setInputFile(inputFile);

    QFileInfo fi(mInputFile);
    setLstFile(fi.absolutePath() + "/" + fi.baseName() + ".lst");

    // iterate options
    foreach (OptionItem item, itemList) {
        // output (o) found
        if (QString::compare(item.key, "o", Qt::CaseInsensitive) == 0
                || QString::compare(item.key, "output", Qt::CaseInsensitive) == 0) {
            setLstFile(item.value);
        } else if (QString::compare(item.key, "curdir", Qt::CaseInsensitive) == 0
                   || QString::compare(item.key, "wdir", Qt::CaseInsensitive) == 0) {
            // TODO: save workingdir somewhere
        }
        mGamsArgs[item.key] = item.value;
        // TODO: warning if overriding default argument?
    }

    // prepare return value
    QStringList output { mInputFile };
    for(QString k : mGamsArgs.keys()) {
        output.append(k + "=" + mGamsArgs.value(k));
    }

    return output;
}

QString GamsProperties::lstFile() const
{
    return mLstFile;
}

void GamsProperties::setLstFile(const QString &lstFile)
{
    if (QFileInfo(lstFile).isAbsolute()) {
        mLstFile = lstFile;
    } else {
        QFileInfo fi(mInputFile);
        mLstFile = fi.absolutePath() + "/" + lstFile;
    }
}

FileId GamsProperties::fileId() const
{
    return mFileId;
}

void GamsProperties::setFileId(const FileId &fileId)
{
    mFileId = fileId;
}

QString GamsProperties::inputFile() const
{
    return mInputFile;
}

void GamsProperties::setInputFile(const QString &inputFile)
{
#if defined(__unix__) || defined(__APPLE__)
    mInputFile = QDir::toNativeSeparators(inputFile);
#else
    mInputFile = "\""+QDir::toNativeSeparators(inputFile)+"\"";
#endif
}

}
}
