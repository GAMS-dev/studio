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
#include <QMouseEvent>
#include <QDebug>
#include <QToolTip>
#include <QLineEdit>

#include "connectdatakeydelegate.h"
#include "connectdatamodel.h"

namespace gams {
namespace studio {
namespace connect {

ConnectDataKeyDelegate::ConnectDataKeyDelegate(QObject *parent)
    : QStyledItemDelegate{parent}
{
    mIconMargin = 2;
    mIconWidth  = 16 + mIconMargin;
    mIconHeight = 16 + mIconMargin;
}

ConnectDataKeyDelegate::~ConnectDataKeyDelegate()
{
    mSchemaHelpPosition.clear();
    mSchemaAppendPosition.clear();
}

void ConnectDataKeyDelegate::initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const
{
    QStyledItemDelegate::initStyleOption(option, index);
    option->text = index.data(Qt::DisplayRole).toString();
    if (index.column()!=(int)DataItemColumn::Key)
        return;

    QModelIndex checkstate_index = index.sibling(index.row(),(int)DataItemColumn::CheckState );
    if (checkstate_index.data( Qt::DisplayRole ).toInt()==(int)DataCheckState::SchemaName) {
        option->icon = QIcon(qvariant_cast<QIcon>(index.data(Qt::DecorationRole)));
        option->decorationPosition = QStyleOptionViewItem::Right;
        mSchemaHelpPosition[index.data(Qt::DisplayRole).toString()] =
                    QRect(option->rect.bottomRight().x()-mIconWidth, option->rect.bottomRight().y()-mIconHeight, mIconWidth , mIconHeight);
    } else if (checkstate_index.data( Qt::DisplayRole ).toInt()==(int)DataCheckState::ListAppend ||
               checkstate_index.data( Qt::DisplayRole ).toInt()==(int)DataCheckState::MapAppend     ) {
               option->icon = QIcon(qvariant_cast<QIcon>(index.data(Qt::DecorationRole)));
               mSchemaAppendPosition[index] = QRect(option->rect.topLeft().x(), option->rect.topLeft().y(), mIconWidth , mIconHeight);
    }
}

QWidget *ConnectDataKeyDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option)
    QLineEdit* lineEdit = new QLineEdit(parent);

    QModelIndex checkstate_index = index.sibling( index.row(), (int)DataItemColumn::CheckState );
    if (checkstate_index.data(Qt::DisplayRole).toInt()!=(int)DataCheckState::ElementKey &&
       checkstate_index.data(Qt::DisplayRole).toInt()!=(int)DataCheckState::ElementMap     )
        return lineEdit;

    lineEdit->adjustSize();
    mLastEditor = lineEdit;
    mIsLastEditorClosed = false;

    connect( lineEdit, &QLineEdit::editingFinished,
             this, &ConnectDataKeyDelegate::commitAndCloseEditor );

    return lineEdit;
}

void ConnectDataKeyDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QLineEdit* lineEdit = qobject_cast<QLineEdit*>( editor ) ;
    if (lineEdit) {
        QVariant data = index.model()->data( index );
        lineEdit->setText(  data.toString() ) ;
        return;
    }
    QStyledItemDelegate::setEditorData(editor, index);
}

void ConnectDataKeyDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    QLineEdit* lineEdit = qobject_cast<QLineEdit*>( editor ) ;
    if (lineEdit) {
        model->setData( index, lineEdit->text() ) ;
        return;
    }
    QStyledItemDelegate::setModelData(editor, model, index);
}

QWidget *ConnectDataKeyDelegate::lastEditor() const
{
   return mLastEditor;
}

bool ConnectDataKeyDelegate::isLastEditorClosed() const
{
    return mIsLastEditorClosed;
}

void ConnectDataKeyDelegate::commitAndCloseEditor()
{
    QLineEdit *lineEdit = qobject_cast<QLineEdit *>( mLastEditor ? mLastEditor : sender() ) ;
    emit commitData(lineEdit);
    emit closeEditor(lineEdit);
    emit modificationChanged(true);
    mIsLastEditorClosed = true;
}


bool ConnectDataKeyDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    if (event->type()==QEvent::MouseButtonRelease) {
        QModelIndex checkstate_index = index.sibling(index.row(), (int)DataItemColumn::CheckState);
        if (checkstate_index.data(Qt::DisplayRole).toInt()==(int)DataCheckState::ElementKey ||
            checkstate_index.data(Qt::DisplayRole).toInt()==(int)DataCheckState::ElementMap    ) {
             return true;
        }
        const QMouseEvent* const mouseevent = static_cast<const QMouseEvent*>( event );
        const QPoint p = mouseevent->pos();  // ->globalPos()
        bool found = false;
        QMap<QString, QRect>::const_iterator it;
        for (it= mSchemaHelpPosition.cbegin();  it != mSchemaHelpPosition.cend(); ++it) {
            if (it.value().contains(p)) {
                emit requestSchemaHelp(it.key());
                found = true;
                break;
            }
        }
        if (!found) {
            QMap<QModelIndex, QRect>::const_iterator it;
            for (it= mSchemaAppendPosition.cbegin();  it != mSchemaAppendPosition.cend(); ++it) {
                if (it.key()== index && it.value().contains(p)) {
                    emit requestAppendItem(index);
                    found = true;
                    break;
                }
            }
            return found;
        }
        return (index.data( Qt::DisplayRole ).toBool());
    }
    return QStyledItemDelegate::editorEvent(event,model, option, index);

}

bool ConnectDataKeyDelegate::eventFilter(QObject *editor, QEvent *event)
{
    if (!editor)
        return false;

    if(event->type()==QEvent::KeyPress) {
       QLineEdit* lineEdit = static_cast<QLineEdit *>(editor);
       QKeyEvent* keyEvent = static_cast<QKeyEvent *>(event);
       if (keyEvent->key() == Qt::Key_Escape) {
             emit closeEditor(lineEdit);
             return true;
       } else if ((keyEvent->key() == Qt::Key_Tab) || (keyEvent->key() == Qt::Key_Enter) || (keyEvent->key() == Qt::Key_Return)) {
                  emit lineEdit->editingFinished();
                  return true;
       }
    }
    return QStyledItemDelegate::eventFilter(editor, event);
}

} // namespace connect
} // namespace studio
} // namespace gams
