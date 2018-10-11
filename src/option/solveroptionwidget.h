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

    bool isModified() const;
    void setModified(bool modified);

signals:
    void optionLoaded(const QString &location);
    void optionTableModelChanged(const QString &commandLineStr);

public slots:
    void showOptionContextMenu(const QPoint &pos);
    void addOptionFromDefinition(const QModelIndex &index);
    void on_dataItemChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);
    void on_optionSaveButton_clicked();

private:
    Ui::SolverOptionWidget *ui;
    FileId mFileId;
    NodeId mGroupId;
    QString mLocation;
    QString mSolverName;

    bool mModified;
    OptionTokenizer* mOptionTokenizer;
};


}
}
}
#endif // SOLVEROPTIONWIDGET_H
