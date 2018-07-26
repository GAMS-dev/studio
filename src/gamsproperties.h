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

    void setAndAnalyzeParameters(const QString &inputFile, QList<OptionItem> itemList);
    QStringList gamsParameters();

    QString inputFile() const;
    void setInputFile(const QString &inputFile);

    QString lstFile() const;
    void setLstFile(const QString &lstFile);

    FileId fileId() const;
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
