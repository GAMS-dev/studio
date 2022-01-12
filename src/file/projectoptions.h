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
#ifndef GAMS_STUDIO_PROJECTOPTIONS_H
#define GAMS_STUDIO_PROJECTOPTIONS_H

#include <QFrame>
#include <QLineEdit>
#include "common.h"

namespace gams {
namespace studio {
class PExProjectNode;

namespace project {

namespace Ui {
class ProjectOptions;
}


class ProjectOptions : public QFrame
{
    Q_OBJECT

public:
    explicit ProjectOptions(QWidget *parent = nullptr);
    ~ProjectOptions() override;
    void setProject(PExProjectNode *project);
    bool isModified() const;
    void save();

signals:
    void modificationChanged(bool modification);

private slots:
    void on_edName_textChanged(const QString &text);
    void on_edWorkDir_textChanged(const QString &text);
    void on_edBaseDir_textChanged(const QString &text);
    void on_bWorkDir_clicked();
    void on_bBaseDir_clicked();
    void projectChanged(gams::studio::NodeId id);

private:
    void updateEditColor(QLineEdit *edit, const QString &text);
    void updateState();
    void showDirDialog(const QString &title, QLineEdit *lineEdit, QString defaultDir);

    Ui::ProjectOptions *ui;
    PExProjectNode *mProject = nullptr;
    bool mModified = false;
    QString mName;

};


} // namespace project
} // namespace studio
} // namespace gams
#endif // GAMS_STUDIO_PROJECTOPTIONS_H
