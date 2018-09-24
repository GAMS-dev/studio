#ifndef SOLVEROPTIONWIDGET_H
#define SOLVEROPTIONWIDGET_H

#include <QWidget>

#include "common.h"


namespace gams {
namespace studio {
namespace option {

namespace Ui {
class SolverOptionWidget;
}

class OptionTokenizer;

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

    OptionTokenizer* mOptionTokenizer;
};


}
}
}
#endif // SOLVEROPTIONWIDGET_H
