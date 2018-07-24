#ifndef GAMSARGMANAGER_H
#define GAMSARGMANAGER_H

#include <QMap>

namespace gams {
namespace studio {

class ProjectGroupNode;
class GamsArgManager
{

public:
    GamsArgManager(ProjectGroupNode *origin);

    void setGamsParameters(const QString &cmd);
    QStringList getGamsParameters();

    QString getInputFile() const;
    void setInputFile(const QString &inputFile);

    ProjectGroupNode *getOriginGroup() const;
    void setOriginGroup(ProjectGroupNode *originGroup);

private:
    QMap<QString, QString> mGamsArgs;
    QString mInputFile;

    ProjectGroupNode *mOriginGroup;
};

} // namespace studio
} // namespace gams

#endif // GAMSARGMANAGER_H
