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
#include "file/filemeta.h"

namespace gams {
namespace studio {

struct NavigatorContent {
    // known files
    NavigatorContent(FileMeta* file, QString additionalText) {
        fileMeta = file;
        fileInfo = QFileInfo(file->location());
        additionalInfo = additionalText;
    }

    // unknown files
    NavigatorContent(QFileInfo file, QString additionalText) {
        fileInfo = file;
        additionalInfo = additionalText;
    }

    // help content
    NavigatorContent(QString txt, QString additionalText, QString prefix) {
        text = txt;
        additionalInfo = additionalText;
        insertPrefix = prefix;
    }

    FileMeta* fileMeta = nullptr;
    QFileInfo fileInfo;
    QString text;
    QString additionalInfo;
    QString insertPrefix;
};

class NavigatorModel : public QAbstractTableModel
{
public:
    explicit NavigatorModel(QObject *parent);

    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    void setContent(QVector<NavigatorContent> content);
    QVector<NavigatorContent> content() const;
    QDir currentDir() const;
    void setCurrentDir(QDir dir);

private:
    QVector<NavigatorContent> mContent;
    QDir mCurrentDir;
};

}
}
#endif // NAVIGATORMODEL_H
