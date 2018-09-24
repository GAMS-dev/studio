#include "solveroptionwidget.h"
#include "ui_solveroptionwidget.h"
#include "mainwindow.h"

namespace gams {
namespace studio {

SolverOptionWidget::SolverOptionWidget(QWidget *parent) :
          QWidget(parent),
          ui(new Ui::SolverOptionWidget)
{
    ui->setupUi(this);

}

SolverOptionWidget::~SolverOptionWidget()
{
     delete ui;
//     delete mOptionTokenizer;
}

FileId SolverOptionWidget::fileId() const
{
    return mFileId;
}

void SolverOptionWidget::setFileId(const FileId &fileId)
{
    mFileId = fileId;
}

NodeId SolverOptionWidget::groupId() const
{
    return mGroupId;
}

void SolverOptionWidget::setGroupId(const NodeId &groupId)
{
    mGroupId = groupId;
}

}
}
