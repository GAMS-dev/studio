/**
 * GAMS Studio
 *
 * Copyright (c) 2017-2025 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2025 GAMS Development Corp. <support@gams.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef MIRODEPLOYDIALOG_H
#define MIRODEPLOYDIALOG_H

#include <QDialog>

#include "mirodeployprocess.h"
#include "mirocommon.h"

namespace gams {
namespace studio {
namespace miro {

namespace Ui {
class MiroDeployDialog;
}

class MiroDeployDialog : public QDialog
{
    Q_OBJECT
public:
    MiroDeployDialog(QWidget *parent = nullptr);
    ~MiroDeployDialog();

    MiroTargetEnvironment targetEnvironment();
    void setDefaults();
    QString assemblyFileName() const { return mModelAssemblyFile; }
    void setAssemblyFileName(const QString &file);
    void setModelName(const QString &modelName);

    QStringList selectedFiles();
    void setSelectedFiles(const QStringList &files);

    void setWorkingDirectory(const QString &workingDirectory);

signals:
    void deploy(bool test, gams::studio::miro::MiroDeployMode mode);
    void newAssemblyFileData();

protected:
    void showEvent(QShowEvent *event) override;

private slots:
    void createClicked();
    void updateTestDeployButtons();

    void on_testBaseButton_clicked();
    void on_deployButton_clicked();


private:
    bool isDataContractAvailable();

private:
    Ui::MiroDeployDialog *ui;
    QString mModelAssemblyFile;
    QString mModelName;
    bool mValidAssemblyFile = false;
};

}
}
}

#endif // MIRODEPLOYDIALOG_H
