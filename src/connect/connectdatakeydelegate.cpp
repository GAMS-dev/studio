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
#include <QToolTip>
#include <QLineEdit>
#include <QComboBox>

#include "connectdatakeydelegate.h"
#include "connectdatamodel.h"

namespace gams {
namespace studio {
namespace connect {

ConnectDataKeyDelegate::ConnectDataKeyDelegate(Connect* connect, QObject *parent)
    : QStyledItemDelegate{parent}, mConnect(connect)
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
    } else if (checkstate_index.data( Qt::DisplayRole ).toInt()==(int)DataCheckState::SchemaAppend) {
        option->icon = QIcon(qvariant_cast<QIcon>(index.data(Qt::DecorationRole)));
        mSchemaAppendPosition[index] = QRect(option->rect.topLeft().x(), option->rect.topLeft().y(), mIconWidth , mIconHeight);
    }
}

QWidget *ConnectDataKeyDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option)
    QModelIndex checkstate_index = index.sibling( index.row(), (int)DataItemColumn::CheckState );

    if (checkstate_index.data().toInt()==(int)DataCheckState::SchemaAppend) {
        QComboBox* editor = new QComboBox(parent);
        QModelIndex schemastr_index = index.parent().siblingAtColumn( (int)DataItemColumn::SchemaKey );
        QStringList schemastr_list  = schemastr_index.data().toString().split(":");
        if (schemastr_list.last().compare("-")==0)
            schemastr_list.removeLast();
        ConnectSchema* schema = mConnect->getSchema(schemastr_list.first());
        if (schema) {
            QString keystr = index.parent().siblingAtColumn( (int)DataItemColumn::Key ).data().toString();
            int number = schema->getNumberOfOneOfSchemaDefined(schemastr_list.last());
            connect(editor,  QOverload<int>::of(&QComboBox::activated),
                    this, &ConnectDataKeyDelegate::commitAndCloseEditor);
            for(int i=0; i<number; ++i)
                editor->addItem(keystr+"["+QString::number(i)+"]");
            mLastEditor = editor;
            mIsLastEditorClosed = false;
        }
        return editor;
    }

    QLineEdit* lineEdit = new QLineEdit(parent);
    if (checkstate_index.data(Qt::DisplayRole).toInt()!=(int)DataCheckState::ElementKey &&
       checkstate_index.data(Qt::DisplayRole).toInt()!=(int)DataCheckState::ElementMap     )
        return lineEdit;

    lineEdit->adjustSize();
    mLastEditor = lineEdit;
    mIsLastEditorClosed = false;
    return lineEdit;
}

void ConnectDataKeyDelegate::destroyEditor(QWidget *editor, const QModelIndex &index) const
{
    Q_UNUSED(editor);
    Q_UNUSED(index);
    mLastEditor = nullptr;
    mIsLastEditorClosed = true;
}

void ConnectDataKeyDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QComboBox* cb = qobject_cast<QComboBox*>(editor);
    if (cb) {
        QString value = index.data(Qt::EditRole).toString();
        int idx = cb->findText(value);
        if (idx >= 0)
            cb->setCurrentIndex(idx);
        cb->showPopup();
        return;
    }
    QLineEdit* lineEdit = qobject_cast<QLineEdit*>( editor ) ;
    if (lineEdit) {
        QVariant data = index.model()->data( index );
        lineEdit->setText( data.toString() ) ;
        return;
    }
    QStyledItemDelegate::setEditorData(editor, index);
}

void ConnectDataKeyDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    QComboBox* cb = qobject_cast<QComboBox*>(editor);
    if (cb) {
        model->setData(index, cb->currentText() );
        return;
    }
    QLineEdit* lineEdit = qobject_cast<QLineEdit*>( editor ) ;
    if (lineEdit) {
        model->setData( index, lineEdit->text() ) ;
        return;
    }
    QStyledItemDelegate::setModelData(editor, model, index);
}

void ConnectDataKeyDelegate::commitAndCloseEditor()
{
    QComboBox* cb = qobject_cast<QComboBox*>(  mLastEditor ? mLastEditor : sender() );
    if (cb) {
        emit commitData(cb);
        emit closeEditor(cb);
        return;
    }
    QLineEdit *lineEdit = qobject_cast<QLineEdit *>( mLastEditor ? mLastEditor : sender() ) ;
    if (lineEdit) {
        emit commitData(lineEdit);
        emit closeEditor(lineEdit);
        emit modificationChanged(true);
        return;
    }
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
                    if (checkstate_index.data(Qt::DisplayRole).toInt()==(int)DataCheckState::SchemaAppend) {
                        QString data = model->data(index).toString();
                        bool *ok = nullptr;
                        int i = data.left(data.indexOf("]")).mid(data.indexOf("[")+1).toInt(ok);
                        emit requestInsertSchemaItem( ok ? 0 : i, index);
                    } else {
                        emit requestAppendItem(index);
                    }
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
                  commitAndCloseEditor();
                  return true;
       }
    }
    return QStyledItemDelegate::eventFilter(editor, event);
}

} // namespace connect
} // namespace studio
} // namespace gams
