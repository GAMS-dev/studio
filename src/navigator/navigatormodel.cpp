/**
 * GAMS Studio
 *
 * Copyright (c) 2017-2025 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2025 GAMS Development Corp. <support@gams.com>
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
#include <QSet>
#include "navigatormodel.h"

namespace gams {
namespace studio {


NavigatorModel::NavigatorModel(QObject *parent) :
    QAbstractTableModel(parent)
{ }

void NavigatorModel::setContent(const QSet<NavigatorContent> &content)
{
    beginResetModel();
    mContent = content.values();
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

void NavigatorModel::setCurrentDir(const QDir &dir)
{
    mCurrentDir.setPath(dir.canonicalPath());
}

int NavigatorModel::findIndex(const QString& file)
{
    int index = 0;
    for (const NavigatorContent& nc : std::as_const(mContent)) {
        if (nc.fileInfo().filePath() == file)
            return index;
        index++;
    }
    return -1;
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
        QFileInfo f = nc.fileInfo();

        if (index.column() == 0) { // file name: show text if available, otherwise fileinfo
            return nc.text();

        } else if (index.column() == 1) { // path: relative path if contains less than 4 .., otherwise absolute
            if (f.fileName().contains("..")) // detect ".." and hide path
                return QVariant();

            QString path = mCurrentDir.relativeFilePath(f.absolutePath());
            if (path.count("..") > 2)
                path = f.absolutePath();
            return (path == ".") ? QVariant() : QDir::toNativeSeparators(path);

        } else if (index.column() == 2) { // additional info
            if (f.fileName().contains("..")) // detect ".." and hide info
                return QVariant();

            return nc.additionalInfo();
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
