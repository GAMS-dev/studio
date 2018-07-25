#include "gamsproperties.h"
#include "file/projectgroupnode.h"
#include "exception.h"
#include <option/option.h>

#include <QDir>
#include <QTime>
#include <QDebug>


namespace gams {
namespace studio {

GamsProperties::GamsProperties(ProjectGroupNode *origin) : mOriginGroup(origin)
{
    // set default path
    mGamsArgs.insert("lo", "3");
    mGamsArgs.insert("ide", "1");
    mGamsArgs.insert("er", "99");
    mGamsArgs.insert("errmsg", "1");
    mGamsArgs.insert("pagesize", "0");
    mGamsArgs.insert("LstTitleLeftAligned", "1");
}

void GamsProperties::analyzeCmdParameters(const QString &inputFile, QList<OptionItem> itemList)
{
    // set input file
#ifdef __unix__
    mInputFile = "\""+QDir::toNativeSeparators(inputFile)+"\"";
#else
    mInputFile = QDir::toNativeSeparators(inputFile);
#endif
    // set lst file
    mOriginGroup->setLstFile(QFileInfo(mInputFile).baseName() + ".lst");

    // iterate options
    foreach (OptionItem item, itemList) {
        // output (o) found
        if (QString::compare(item.key, "o", Qt::CaseInsensitive) == 0
                || QString::compare(item.key, "output", Qt::CaseInsensitive) == 0) {
            mOriginGroup->setLstFile(item.value);
        } else if (QString::compare(item.key, "curdir", Qt::CaseInsensitive) == 0
                   || QString::compare(item.key, "wdir", Qt::CaseInsensitive) == 0) {
            // TODO: save workingdir somewhere
        }
        mGamsArgs[item.key] = item.value;
    }

    // TODO: warning if overriding default argument?
    // TODO: add this: "curdir="+mWorkingDir;
}

QStringList GamsProperties::gamsParameters()
{
    QStringList output;

    output.append(mInputFile);
    for(QString k : mGamsArgs.keys()) {
        output.append(k + "=" + mGamsArgs.value(k));
        qDebug() << output.last(); // rogo: delete
    }

    return output;
}

QString GamsProperties::inputFile() const
{
    return mInputFile;
}

ProjectGroupNode *GamsProperties::originGroup() const
{
    return mOriginGroup;
}

void GamsProperties::setOriginGroup(ProjectGroupNode *originGroup)
{
    mOriginGroup = originGroup;
}

}
}
