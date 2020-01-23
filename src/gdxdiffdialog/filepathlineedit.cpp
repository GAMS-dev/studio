/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2020 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2020 GAMS Development Corp. <support@gams.com>
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
#include "filepathlineedit.h"
#include <QMimeData>
#include <QDir>
#include <file/projecttreeview.h>

namespace gams {
namespace studio {
namespace gdxdiffdialog {

FilePathLineEdit::FilePathLineEdit(QWidget *parent) :
    QLineEdit(parent)
{
    this->setAcceptDrops(true);
}

void FilePathLineEdit::dragEnterEvent(QDragEnterEvent *event)
{
    // drag and drop from the project explorer
    if (event->mimeData()->formats().contains(ProjectTreeView::ItemModelDataType)) {
        QByteArray data = event->mimeData()->data(ProjectTreeView::ItemModelDataType);
        QDataStream stream(&data, QIODevice::ReadOnly);
        QStringList pathList;
        while (!stream.atEnd()) {
            int row, col;
            QMap<int,  QVariant> roleDataMap;
            stream >> row >> col >> roleDataMap;
            pathList << roleDataMap.value(Qt::UserRole).toString();
        }
        if (pathList.size() == 1 && pathList.at(0).toLower().endsWith(".gdx"))
            event->acceptProposedAction();
    }
    // drag and drop from outside
    else if (event->mimeData()->hasUrls() && event->mimeData()->urls().size() == 1 && event->mimeData()->urls().at(0).toLocalFile().toLower().endsWith(".gdx")) {
        event->acceptProposedAction();
    } else
        event->ignore();
}

void FilePathLineEdit::dropEvent(QDropEvent *event)
{
    // drag and drop from the project explorer
    if (event->mimeData()->formats().contains(ProjectTreeView::ItemModelDataType)) {
        QByteArray data = event->mimeData()->data(ProjectTreeView::ItemModelDataType);
        QDataStream stream(&data, QIODevice::ReadOnly);
        QStringList pathList;
        while (!stream.atEnd()) {
            int row, col;
            QMap<int,  QVariant> roleDataMap;
            stream >> row >> col >> roleDataMap;
            pathList << roleDataMap.value(Qt::UserRole).toString();
        }
        if (pathList.size() == 1 && pathList.at(0).toLower().endsWith(".gdx")) {
            setText(pathList.at(0));
            event->accept();
        }
    }
    // drag and drop from outside
    else {
        QString localFile = event->mimeData()->urls().at(0).toLocalFile();
        setText(localFile);
        event->accept();
    }
    setText(QDir::toNativeSeparators(text()));
}

} // namespace gdxdiffdialog
} // namespace studio
} // namespace gams
