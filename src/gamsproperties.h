#ifndef GAMSARGMANAGER_H
#define GAMSARGMANAGER_H

#include <QMap>

namespace gams {
namespace studio {

class ProjectGroupNode;
class OptionItem;
class GamsProperties
{

public:
    GamsProperties(ProjectGroupNode *origin);

    void analyzeCmdParameters(const QString &inputFile, QList<OptionItem> itemList);
    QStringList gamsParameters();

    QString inputFile() const;

    ProjectGroupNode *originGroup() const;
    void setOriginGroup(ProjectGroupNode *originGroup);

private:
    QMap<QString, QString> mGamsArgs;
    QString mInputFile;

    ProjectGroupNode *mOriginGroup;
};

} // namespace studio
} // namespace gams

#endif // GAMSARGMANAGER_H
