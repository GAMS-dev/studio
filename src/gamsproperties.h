#ifndef GAMSARGMANAGER_H
#define GAMSARGMANAGER_H

#include <QMap>
#include "file/projectabstractnode.h"

namespace gams {
namespace studio {

class ProjectGroupNode;
struct OptionItem;

class GamsProperties
{

public:
    GamsProperties(FileId origin);

    QStringList analyzeParameters(const QString &inputFile, QList<OptionItem> itemList);

    void setInputFile(const QString &inputFile);
    QString inputFile() const;

    QString lstFile() const;
    FileId fileId() const;

private:
    void setLstFile(const QString &lstFile);
    void setFileId(const FileId &fileId);

private:
    QMap<QString, QString> mGamsArgs;

    QString mInputFile;
    QString mLstFile;
    FileId mFileId;
};

} // namespace studio
} // namespace gams

#endif // GAMSARGMANAGER_H
