#ifndef GAMS_STUDIO_RECENTDATA_H
#define GAMS_STUDIO_RECENTDATA_H

#include <QWidget>
#include "projectgroupnode.h"

namespace gams {
namespace studio {

class MainWindow;

class RecentData
{
public:
    RecentData() { reset(); }

    void reset();
    void setEditor(QWidget* edit, MainWindow* window);

    /**
     * @brief Name of the main model.
     * @remark Call <c>hasValidRunGroup()</c> before.
     */
    bool hasValidRunGroup();

    QWidget* editor() const { return mEditor; }
    ProjectGroupNode* group() const {return mGroup; }
    FileId editFileId() const { return mEditFileId; }
    QString path() const { return mPath; }

private:
    QWidget* mEditor;
    FileId mEditFileId;
    ProjectGroupNode* mGroup;
    QString mPath;
};

} // namespace studio
} // namespace gams

#endif // GAMS_STUDIO_RECENTDATA_H
