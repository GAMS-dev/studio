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
    mMain(main), QAbstractTableModel(parent)
{

}

void NavigatorModel::setContent(QVector<NavigatorContent> content)
{
    mContent = content;
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
    return 2;
}

QVariant NavigatorModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole) {
        if (index.column() == 0)
            return mContent.at(index.row()).file->location();
        else if (index.column() == 1)
            return mContent.at(index.row()).additionalInfo;

    } else if (role == Qt::TextAlignmentRole) {
        if (index.column() == 0)
            return Qt::AlignLeft;
        else
            return Qt::AlignRight;

    } else if (role == Qt::FontRole) {
        if (index.column() == 1) {
            QFont font;
            font.setItalic(true);
            return font;
        }
    }

    return QVariant();
}

}
}
