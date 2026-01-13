/**
 * GAMS Studio
 *
 * Copyright (c) 2017-2026 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2026 GAMS Development Corp. <support@gams.com>
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
#include <QKeyEvent>
#include <QCompleter>
#include <QStringListModel>
#include <QApplication>
#include <QMainWindow>

#include "option/optionitemdelegate.h"
#include "option/optiontablemodel.h"

namespace gams {
namespace studio {
namespace option {

OptionItemDelegate::OptionItemDelegate(OptionTokenizer* tokenizer, QObject* parent) :
    QStyledItemDelegate(parent), mOptionTokenizer(tokenizer), mIsLastEditorClosed(false), mLastEditor(nullptr)
{
    mCurrentEditedIndex = QModelIndex();
    connect( this, &OptionItemDelegate::currentEditedIndexChanged, this, &OptionItemDelegate::updateCurrentEditedIndex, Qt::UniqueConnection) ;
}

QWidget* OptionItemDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    Q_UNUSED(option)
    QLineEdit* lineEdit = new QLineEdit(parent);
    QCompleter* completer = new QCompleter(lineEdit);
    if (index.column()==OptionTableModel::COLUMN_KEY) {
        completer->setModel(new QStringListModel(mOptionTokenizer->getOption()->getValidNonDeprecatedKeyList()));
    } else  if (index.column()==OptionTableModel::COLUMN_VALUE) {
        QString key = index.model()->data( index.model()->index(index.row(), OptionTableModel::COLUMN_KEY) ).toString();
        if (!mOptionTokenizer->getOption()->isValid(key))
            key = mOptionTokenizer->getOption()->getNameFromSynonym(key);

        if ( mOptionTokenizer->getOption()->getOptionType(key) == optTypeBoolean &&
            mOptionTokenizer->getOption()->getOptionSubType(key) != optsubNoValue )
            completer->setModel(new QStringListModel( { "0", "1" } ));
        else
            completer->setModel(new QStringListModel(mOptionTokenizer->getOption()->getNonHiddenValuesList(key)) );
    }
    completer->setCompletionMode(QCompleter::UnfilteredPopupCompletion);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    completer->setMaxVisibleItems(10);

    lineEdit->setCompleter(completer);
    lineEdit->adjustSize();

    mLastEditor = lineEdit;
    mIsLastEditorClosed = false;

    emit currentEditedIndexChanged(index);
    return lineEdit;
}

void OptionItemDelegate::destroyEditor(QWidget *editor, const QModelIndex &index) const
{
    Q_UNUSED(editor)
    Q_UNUSED(index)
    mLastEditor = nullptr;
    mIsLastEditorClosed = true;
}

void OptionItemDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QLineEdit* lineEdit = qobject_cast<QLineEdit*>( editor ) ;
    if (lineEdit && index.isValid()) {
        QVariant data = index.model()->data( index.model()->index(index.row(), index.column()) );
        lineEdit->setText(  data.toString() ) ;
        return;
    }
    QStyledItemDelegate::setEditorData(editor, index);
}

void OptionItemDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    QLineEdit* lineEdit = qobject_cast<QLineEdit*>( editor ) ;
    if (lineEdit && index.isValid()) {
        model->setData( index, lineEdit->text() ) ;
        return;
    }
    QStyledItemDelegate::setModelData(editor, model, index);
}

void OptionItemDelegate::commitAndCloseEditor()
{
    QLineEdit *lineEdit = qobject_cast<QLineEdit *>( mLastEditor ? mLastEditor : sender() ) ;
    if (lineEdit) {
        emit commitData(lineEdit);
        if (!mIsLastEditorClosed)
           emit closeEditor(lineEdit);
        emit currentEditedIndexChanged(mCurrentEditedIndex);
    }
}

void OptionItemDelegate::updateCurrentEditedIndex(const QModelIndex &index)
{
    mCurrentEditedIndex = index;
}

QModelIndex OptionItemDelegate::currentEditedIndex() const
{
    return mCurrentEditedIndex;
}

QWidget *OptionItemDelegate::lastEditor() const
{
    return mLastEditor;
}

bool OptionItemDelegate::isLastEditorClosed() const
{
    return mIsLastEditorClosed;
}

bool OptionItemDelegate::eventFilter(QObject* editor, QEvent* event)
{
    if (!editor)
        return false;

    if(event->type()==QEvent::KeyPress) {
       QLineEdit* lineEdit = qobject_cast<QLineEdit *>(editor);
       QKeyEvent* keyEvent = dynamic_cast<QKeyEvent *>(event);
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

} // namespace option
} // namespace studio
} // namespace gams
