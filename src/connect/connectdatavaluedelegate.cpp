/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2023 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2023 GAMS Development Corp. <support@gams.com>
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
#include <QPainter>
#include <QLineEdit>
#include <QComboBox>
#include <QCompleter>
#include <QStringListModel>
#include <QKeyEvent>

#include "connectdatavaluedelegate.h"
#include "connectdatamodel.h"

namespace gams {
namespace studio {
namespace connect {

ConnectDataValueDelegate::ConnectDataValueDelegate(QObject *parent)
    : QStyledItemDelegate{parent}
{
}

QWidget *ConnectDataValueDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option)
    QLineEdit* lineEdit = new QLineEdit(parent);
    QCompleter* completer = new QCompleter(lineEdit);

    QModelIndex allowedval_index = index.sibling( index.row(), (int)DataItemColumn::AllowedValue );
    QStringList allowedval_list = allowedval_index.data().toString().split(",");
    if (!allowedval_index.data().toString().isEmpty() && allowedval_list.size() > 0) {
        completer->setModel( new QStringListModel(allowedval_list) );
    } else {
        QModelIndex type_index = index.sibling( index.row(), (int)DataItemColumn::SchemaType );
        QStringList type_list = type_index.data().toString().split(",");
        if (type_list.contains("boolean", Qt::CaseInsensitive)) {
            QStringList boolean_list({ "true", "false"});
            completer->setModel( new QStringListModel(boolean_list) );
        }
    }
    completer->setCompletionMode(QCompleter::UnfilteredPopupCompletion);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    completer->setMaxVisibleItems(10);

    lineEdit->setCompleter(completer);
    lineEdit->adjustSize();

    mLastEditor = lineEdit;
    mIsLastEditorClosed = false;

    return lineEdit;
}

void ConnectDataValueDelegate::destroyEditor(QWidget *editor, const QModelIndex &index) const
{
    Q_UNUSED(editor)
    Q_UNUSED(index)
    mLastEditor = nullptr;
    mIsLastEditorClosed = true;
}

void ConnectDataValueDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QLineEdit* lineEdit = qobject_cast<QLineEdit*>( editor ) ;
    if (lineEdit) {
        QVariant data = index.model()->data( index );
        lineEdit->setText(  data.toString() ) ;
        return;
    }
    QStyledItemDelegate::setEditorData(editor, index);
}

void ConnectDataValueDelegate::commitAndCloseEditor()
{
    QLineEdit *lineEdit = qobject_cast<QLineEdit *>( mLastEditor ? mLastEditor : sender() ) ;
    if (lineEdit && !mIsLastEditorClosed) {
        emit commitData(lineEdit);
        emit closeEditor(lineEdit);
        emit modificationChanged(true);
    }
    mIsLastEditorClosed = true;
}


void ConnectDataValueDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    QLineEdit* lineEdit = qobject_cast<QLineEdit*>( editor ) ;
    if (lineEdit) {
        model->setData( index, lineEdit->text(), Qt::EditRole ) ;
        return;
    }
    QStyledItemDelegate::setModelData(editor, model, index);
}

bool ConnectDataValueDelegate::eventFilter(QObject *editor, QEvent *event)
{
    if (!editor)
        return false;

    if(event->type()==QEvent::KeyPress) {
       QLineEdit* lineEdit = static_cast<QLineEdit *>(editor);
       QKeyEvent* keyEvent = static_cast<QKeyEvent *>(event);
       if (keyEvent->key() == Qt::Key_Escape) {
             mIsLastEditorClosed = true;
             emit closeEditor(lineEdit);
             return true;
       } else if ((keyEvent->key() == Qt::Key_Tab) || (keyEvent->key() == Qt::Key_Enter) || (keyEvent->key() == Qt::Key_Return)) {
                  commitAndCloseEditor();
                  return true;
       }
    }
    return QStyledItemDelegate::eventFilter(editor, event);

}

} // namespace connect
} // namespace studio
} // namespace gams
