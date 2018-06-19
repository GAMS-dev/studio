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
#ifndef BOOKMARKDIALOG_H
#define BOOKMARKDIALOG_H

#include <QDialog>

#include "ui_bookmarkdialog.h"

class QStandardItemModel;

namespace gams {
namespace studio {

class BookmarkDialog : public QDialog
{
    Q_OBJECT

public:
    BookmarkDialog(QMultiMap<QString, QString>& bmMap, QWidget *parent = nullptr);
    ~BookmarkDialog();

protected:
    void keyPressEvent(QKeyEvent *event);

signals:
    void openUrl(const QUrl& location);
    void updateBookmarkName(const QString& location, const QString& name);
    void updateBookmarkLocation(const QString& oldLocation, const QString& newLocation, const QString& name);
    void removeBookmark(const QString& location, const QString& name);

private slots:
    void on_bookmarkEntryShowed(const QModelIndex &index);
    void on_contextMenuShowed(const QPoint &pos);

private:
    Ui::bookmarkDialog ui;
    QStandardItemModel* model;
    QMultiMap<QString, QString>& bookmarkMap;
};

} // namespace studio
} // namespace gams

#endif // BOOKMARKDIALOG_H
