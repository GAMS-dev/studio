#ifndef GAMSARGMANAGER_H
#define GAMSARGMANAGER_H

#include <QMap>

namespace gams {
namespace studio {

class ProjectGroupNode;
struct OptionItem;

class GamsProperties
{

public:
    GamsProperties(ProjectGroupNode* parent);

    QStringList analyzeParameters(const QString &inputFile, QList<OptionItem> itemList);
private:
    QMap<QString, QString> mGamsArgs;
    ProjectGroupNode* mParent;
};

} // namespace studio
} // namespace gams

#endif // GAMSARGMANAGER_H
