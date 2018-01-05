/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017 GAMS Development Corp. <support@gams.com>
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
#include "filetreemodel.h"

#include "exception.h"
#include "filerepository.h"
#include "filecontext.h"
#include "filegroupcontext.h"
#include "fileactioncontext.h"
#include "logger.h"

namespace gams {
namespace studio {

FileTreeModel::FileTreeModel(FileRepository* parent, FileGroupContext* root)
    : QAbstractItemModel(parent), mFileRepo(parent), mRoot(root)
{
    if (!mFileRepo)
        FATAL() << "nullptr not allowed. The FileTreeModel needs a valid FileRepository.";
}

QModelIndex FileTreeModel::index(FileSystemContext *entry) const
{
     if (!entry)
         return QModelIndex();
     if (!entry->parentEntry())
         return createIndex(0, 0, entry);
     for (int i = 0; i < entry->parentEntry()->childCount(); ++i) {
         if (entry->parentEntry()->childEntry(i) == entry) {
             return createIndex(i, 0, entry);
         }
     }
     return QModelIndex();
}

QModelIndex FileTreeModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();
    FileSystemContext *fc = mFileRepo->context(parent)->childEntry(row);
    if (!fc)
        FATAL() << "invalid child for row " << row;
    return createIndex(row, column, fc);
}

QModelIndex FileTreeModel::parent(const QModelIndex& child) const
{
    FileSystemContext* eChild = mFileRepo->context(child);
    if (eChild == mRoot)
        return QModelIndex();
    FileGroupContext* eParent = eChild->parentEntry();
    if (!eParent)
        return QModelIndex();
    if (eParent == mRoot)
        return createIndex(0, child.column(), eParent);
    int row = eParent->indexOf(eChild);
    if (row < 0)
        FATAL() << "could not find child in parent";
    return createIndex(row, child.column(), eParent);
}

int FileTreeModel::rowCount(const QModelIndex& parent) const
{
    FileSystemContext* entry = mFileRepo->context(parent);
    if (!entry) return 0;
    return entry->childCount();
}

int FileTreeModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return 1;
}

QVariant FileTreeModel::data(const QModelIndex& ind, int role) const
{
    if (!ind.isValid()) return QVariant();
    switch (role) {
    case Qt::BackgroundColorRole:
        if (isSelected(ind)) return QColor("#4466BBFF");

    case Qt::DisplayRole:
        return mFileRepo->context(ind)->caption();

    case Qt::FontRole: {
        if (isCurrent(ind) || isCurrentGroup(ind)) {
//        if (mFileRepo->context(index)->flags().testFlag(FileSystemContext::cfActive)) {
            QFont f;
            f.setBold(true);
            return f;
        }
        if (mFileRepo->context(ind)->type() == FileSystemContext::FileAction) {
            QFont f;
            f.setItalic(true);
            return f;
        }
    }
        break;

    case Qt::ForegroundRole: {
        FileSystemContext::ContextFlags flags = mFileRepo->context(ind)->flags();
        if (flags.testFlag(FileSystemContext::cfMissing))
            return QColor(Qt::red);
        if (flags.testFlag(FileSystemContext::cfActive)) {
            return (isCurrent(ind)) ? QColor(Qt::blue)
                                      : QColor(Qt::black);
        }
        break;
    }

    case Qt::DecorationRole:
        return mFileRepo->context(ind)->icon();

    case Qt::ToolTipRole:
        return mFileRepo->context(ind)->location();

    default:
        break;
    }
    return QVariant();
}

QModelIndex FileTreeModel::rootModelIndex() const
{
    return createIndex(0, 0, mRoot);
}

FileGroupContext* FileTreeModel::rootContext() const
{
    return mRoot;
}

bool FileTreeModel::removeRows(int row, int count, const QModelIndex& parent)
{
    Q_UNUSED(row);
    Q_UNUSED(count);
    Q_UNUSED(parent);
    EXCEPT() << "FileTreeModel::removeRows is unsupported, please use FileTreeModel::removeChild";
    return false;
}

bool FileTreeModel::insertChild(int row, FileGroupContext* parent, FileSystemContext* child)
{
    QModelIndex parMi = index(parent);
    if (!parMi.isValid()) return false;
    beginInsertRows(parMi, row, row);
    child->setParentEntry(parent);
    endInsertRows();
    return true;
}

bool FileTreeModel::removeChild(FileSystemContext* child)
{
    QModelIndex mi = index(child);
    if (!mi.isValid()) return false;
    beginRemoveRows(index(child->parentEntry()), mi.row(), mi.row());
    child->setParentEntry(nullptr);
    endRemoveRows();
    return true;
}

bool FileTreeModel::isCurrent(const QModelIndex& ind) const
{
    return (mCurrent.isValid() && ind == mCurrent);
}

void FileTreeModel::setCurrent(const QModelIndex& ind)
{
    if (!isCurrent(ind)) {
        QModelIndex mi = mCurrent;
        mCurrent = ind;
        if (mi.isValid()) {
            dataChanged(mi, mi);                        // invalidate old
            QModelIndex par = index(mFileRepo->context(mi)->parentEntry());
            if (par.isValid()) dataChanged(par, par);
        }
        if (mCurrent.isValid()) {
            dataChanged(mCurrent, mCurrent);            // invalidate new
            QModelIndex par = index(mFileRepo->context(mCurrent)->parentEntry());
            if (par.isValid()) dataChanged(par, par);
        }
    }
}

bool FileTreeModel::isCurrentGroup(const QModelIndex& ind) const
{
    if (mCurrent.isValid()) {
        FileSystemContext* fsc = mFileRepo->context(mCurrent);
        if (fsc->parentEntry() == ind.internalPointer()) {
            return true;
        }
    }
    return false;
}

bool FileTreeModel::isSelected(const QModelIndex& ind) const
{
    return (mSelected.isValid() && ind == mSelected);
}

void FileTreeModel::setSelected(const QModelIndex& ind)
{
    if (!isSelected(ind)) {
        QModelIndex mi = mSelected;
        mSelected = ind;
        if (mi.isValid()) dataChanged(mi, mi);                      // invalidate old
        if (mSelected.isValid()) dataChanged(mSelected, mSelected); // invalidate new
    }
}

} // namespace studio
} // namespace gams
