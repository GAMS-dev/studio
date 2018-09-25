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
    explicit SolverOptionWidget(QString optionDefitionFile, QString optionFilePath, QWidget *parent = nullptr);
    ~SolverOptionWidget();

    bool isInFocused(QWidget* focusWidget);

    FileId fileId() const;
    void setFileId(const FileId &fileId);

    NodeId groupId() const;
    void setGroupId(const NodeId &groupId);

signals:
    void optionLoaded(const QString &location);
    void optionTableModelChanged(const QString &commandLineStr);

public slots:
    void showOptionContextMenu(const QPoint &pos);
    void addOptionFromDefinition(const QModelIndex &index);

private:
    Ui::SolverOptionWidget *ui;
    FileId mFileId;
    NodeId mGroupId;

    QString mSolverName;
    OptionTokenizer* mOptionTokenizer;
};


}
}
}
#endif // SOLVEROPTIONWIDGET_H
