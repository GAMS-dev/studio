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
#ifndef NAVIGATORMODEL_H
#define NAVIGATORMODEL_H

#include <QAbstractTableModel>

#include "mainwindow.h"

namespace gams {
namespace studio {

struct NavigatorContent {
    FileMeta* file;
    QString text;
    QString additionalInfo;
};

class NavigatorModel : public QAbstractTableModel
{
public:
    explicit NavigatorModel(QObject *parent, MainWindow *main);

    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    void setContent(QVector<NavigatorContent> content, QString currentFile);
    QVector<NavigatorContent> content() const;

private:
    QObject* QAbstractTableModel = nullptr;
    QVector<NavigatorContent> mContent;
    gams::studio::MainWindow* mMain = nullptr;
    QDir mCurrentDir;
};

}
}
#endif // NAVIGATORMODEL_H
