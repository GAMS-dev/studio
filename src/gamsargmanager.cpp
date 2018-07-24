#include "gamsargmanager.h"
#include "file/projectgroupnode.h"

#include <QDir>
#include <QTime>
#include <QDebug>

namespace gams {
namespace studio {

GamsArgManager::GamsArgManager(ProjectGroupNode *origin) : mOriginGroup(origin)
{
    // set default path
    mGamsArgs.insert("lo", "3");
    mGamsArgs.insert("ide", "1");
    mGamsArgs.insert("er", "99");
    mGamsArgs.insert("errmsg", "1");
    mGamsArgs.insert("pagesize", "0");
    mGamsArgs.insert("LstTitleLeftAligned", "1");
}

void GamsArgManager::setGamsParameters(const QString &cmd)
{
#ifdef __unix__
    mInputFile = "\""+QDir::toNativeSeparators(mInputFile)+"\"";
#else
    mInputFile = QDir::toNativeSeparators(mInputFile);
#endif
    if (!cmd.isEmpty()) {
        QStringList paramList = cmd.split(QRegExp("\\s+"));

        for (QString s : paramList) {
            QStringList item = s.split("=");

            // TODO: warning if overriding default argument?
            mGamsArgs[item[0]] = item[1];
        }
    }

}
//     << "curdir="+mWorkingDir;

QStringList GamsArgManager::getGamsParameters()
{
    QStringList output;
    // 1. mInputFile
    output.append(mInputFile);

    for(QString k : mGamsArgs.keys()) {
        output.append(k + "=" + mGamsArgs.value(k));
        qDebug() << output.last(); // rogo: delete
    }

    return output;
}

QString GamsArgManager::getInputFile() const
{
    return mInputFile;
}

void GamsArgManager::setInputFile(const QString &inputFile)
{
    mInputFile = inputFile;
}

}
}
