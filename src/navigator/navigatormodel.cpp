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

void NavigatorModel::setContent(QVector<NavigatorContent> content, QString currentFile)
{
    beginResetModel();
    mContent = content;
    mCurrentDir.setPath(QFileInfo(currentFile).absolutePath());
    endResetModel();
}

QVector<NavigatorContent> NavigatorModel::content() const
{
    return mContent;
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
        FileMeta* fm = mContent.at(index.row()).file;
        QFileInfo f;
        if (fm) f = QFileInfo(fm->location());

        if (index.column() == 0) { // file name
            if (!mContent.at(index.row()).text.isEmpty())
                return mContent.at(index.row()).text;

            if (!fm) return QVariant();
            return f.fileName();

        } else if (index.column() == 1) { // path
            if (!fm) return QVariant();
            return mCurrentDir.relativeFilePath(f.absolutePath());

        } else if (index.column() == 2) { // additional info
            return mContent.at(index.row()).additionalInfo;
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
