/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2018 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2018 GAMS Development Corp. <support@gams.com>
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
