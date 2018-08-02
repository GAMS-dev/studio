#include <QDir>
#include <QTime>
#include <QDebug>

#include "gamsproperties.h"
#include "exception.h"
#include "option/option.h"
#include "file/projectgroupnode.h"

namespace gams {
namespace studio {

GamsProperties::GamsProperties(ProjectGroupNode* parent) : mParent(parent)
{
    // set default parameters
    mGamsArgs.insert("lo", "3");
    mGamsArgs.insert("ide", "1");
    mGamsArgs.insert("er", "99");
    mGamsArgs.insert("errmsg", "1");
    mGamsArgs.insert("pagesize", "0");
    mGamsArgs.insert("LstTitleLeftAligned", "1");
}

QStringList GamsProperties::analyzeParameters(const QString &inputFile, QList<OptionItem> itemList)
{
    mParent->setInputFile(inputFile);

    QFileInfo fi(inputFile);
    // set default lst name to revert deleted o parameter values
    mParent->setLstFile(fi.absolutePath() + "/" + fi.baseName() + ".lst");

    // iterate options
    foreach (OptionItem item, itemList) {
        // output (o) found
        if (QString::compare(item.key, "o", Qt::CaseInsensitive) == 0
                || QString::compare(item.key, "output", Qt::CaseInsensitive) == 0) {
            mParent->setLstFile(item.value);
        } else if (QString::compare(item.key, "curdir", Qt::CaseInsensitive) == 0
                   || QString::compare(item.key, "wdir", Qt::CaseInsensitive) == 0) {
            // TODO: save workingdir somewhere
        }
        mGamsArgs[item.key] = item.value;
        // TODO: warning if overriding default argument?
    }

    // prepare return value
    QStringList output { inputFile };
    for(QString k : mGamsArgs.keys()) {
        output.append(k + "=" + mGamsArgs.value(k));
    }

    return output;
}

}
}
