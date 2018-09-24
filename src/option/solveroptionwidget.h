#ifndef SOLVEROPTIONWIDGET_H
#define SOLVEROPTIONWIDGET_H

#include <QWidget>

#include "common.h"

namespace Ui {
class SolverOptionWidget;
}

namespace gams {
namespace studio {
namespace option {

class MainWindow;

class SolverOptionWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SolverOptionWidget(QWidget *parent = nullptr);
    ~SolverOptionWidget();

    FileId fileId() const;
    void setFileId(const FileId &fileId);

    NodeId groupId() const;
    void setGroupId(const NodeId &groupId);

private:
    Ui::SolverOptionWidget *ui;
    FileId mFileId;
    NodeId mGroupId;
};


}
}
}
#endif // SOLVEROPTIONWIDGET_H
