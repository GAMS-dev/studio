#ifndef MIRODEPLOYDIALOG_H
#define MIRODEPLOYDIALOG_H

#include <QDialog>

#include "mirodeployprocess.h"

namespace Ui {
class MiroDeployDialog;
}

namespace gams {
namespace studio {


class MiroDeployDialog : public QDialog
{
    Q_OBJECT

public:
    MiroDeployDialog(const QString &modelAssemblyFile, QWidget *parent = nullptr);

    bool baseMode() const;
    bool hypercubeMode() const;
    bool testDeployment() const;
    MiroTargetEnvironment targetEnvironment();

private slots:
    void on_deployButton_clicked();
    void on_testDeployButton_clicked();

private:
    bool showMessageBox();

private:
    Ui::MiroDeployDialog *ui;
    QString mModelAssemblyFile;
    bool mTestDeploy;
};

}
}

#endif // MIRODEPLOYDIALOG_H
