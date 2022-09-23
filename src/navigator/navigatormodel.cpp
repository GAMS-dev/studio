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
#include "navigatormodel.h"

namespace gams {
namespace studio {


NavigatorModel::NavigatorModel(QObject *parent, MainWindow* main) :
    QAbstractTableModel(parent), mMain(main)
{ }

void NavigatorModel::setContent(QVector<NavigatorContent> content, QString workDir)
{
    beginResetModel();
    mContent = content;
    mCurrentDir = QDir(workDir);
    endResetModel();
}

QVector<NavigatorContent> NavigatorModel::content() const
{
    return mContent;
}

QDir NavigatorModel::currentDir() const
{
    return mCurrentDir;
}

int NavigatorModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return mContent.count();
}

int NavigatorModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 3;
}

QVariant NavigatorModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole) {
        NavigatorContent nc = mContent.at(index.row());
        FileMeta* fm = nc.fileMeta;
        QFileInfo f = nc.fileInfo;

        if (index.column() == 0) { // file name
            if (!nc.text.isEmpty())
                return nc.text;
            return f.fileName();

        } else if (index.column() == 1) { // path
            if (!fm) return QVariant();
            QString path = mCurrentDir.relativeFilePath(f.absolutePath());
            if (path.count("..") > 3)
                path = f.absolutePath();

            return path == "." ? QVariant() : path;

        } else if (index.column() == 2) { // additional info
            return nc.additionalInfo;
        }

    } else if (role == Qt::TextAlignmentRole) {
        if (index.column() == 0 || index.column() == 1)
            return Qt::AlignLeft;
        else
            return Qt::AlignRight;

    } else if (role == Qt::FontRole) {
        QFont font;
        if (index.column() == 0) {
            font.setBold(true);
            return font;
        } else if (index.column() == 2) {
            font.setItalic(true);
            return font;
        }
    }

    return QVariant();
}

}
}
