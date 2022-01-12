/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2022 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2022 GAMS Development Corp. <support@gams.com>
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
 */
#ifndef GAMS_STUDIO_PATH_PATHREQUEST_H
#define GAMS_STUDIO_PATH_PATHREQUEST_H

#include <QDialog>
#include <QLineEdit>

namespace gams {
namespace studio {

class ProjectRepo;

namespace path {

namespace Ui {
class PathRequest;
}

class PathRequest : public QDialog
{
    Q_OBJECT

public:
    explicit PathRequest(QWidget *parent = nullptr);
    ~PathRequest() override;
    void init(ProjectRepo *repo, const QString &name, const QString &baseDir, const QVariantList &data);
    bool checkProject();
    QString baseDir() const;

private slots:
    void on_bDir_clicked();
    void on_edBaseDir_textEdited(const QString &text);

private:
    void showDirDialog(const QString &title, QLineEdit *lineEdit);
    void updateEditColor(QLineEdit *edit, const QString &text);

private:
    Ui::PathRequest *ui;
    ProjectRepo *mProjectRepo = nullptr;
    QVariantList mData;
    QString mInitialText;

};


} // namespace path
} // namespace studio
} // namespace gams
#endif // GAMS_STUDIO_PATH_PATHREQUEST_H
