#ifndef PROJECTCONTEXTMENU_H
#define PROJECTCONTEXTMENU_H

#include <QtWidgets>

namespace gams {
namespace studio {

class FileSystemContext;
class FileGroupContext;
class FileContext;

class ProjectContextMenu : public QMenu
{
    Q_OBJECT
public:
    ProjectContextMenu();
    void setNode(FileSystemContext* context);

signals:
    void closeGroup(FileGroupContext* group);
    void runGroup(FileGroupContext* group);

private slots:
    void onCloseGroup();
    void onRunGroup();

private:
    FileSystemContext* mNode;
    QHash<int, QAction*> mActions;
};

} // namespace studio
} // namespace gams

#endif // PROJECTCONTEXTMENU_H
