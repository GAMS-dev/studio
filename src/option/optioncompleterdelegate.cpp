/**
 * GAMS Studio
 *
 * Copyright (c) 2017-2024 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2024 GAMS Development Corp. <support@gams.com>
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
#include "lineeditcompleteevent.h"
#include "optioncompleterdelegate.h"
#include "solveroptiontablemodel.h"

namespace gams {
namespace studio {
namespace option {

OptionCompleterDelegate::OptionCompleterDelegate(OptionTokenizer* tokenizer, QObject* parent) :
    QStyledItemDelegate(parent), mOptionTokenizer(tokenizer), mOption(tokenizer->getOption()), mIsLastEditorClosed(false), mLastEditor(nullptr)
{
    mCurrentEditedIndex = QModelIndex();
    connect( this, &OptionCompleterDelegate::currentEditedIndexChanged, this, &OptionCompleterDelegate::updateCurrentEditedIndex, Qt::UniqueConnection) ;
}

QWidget* OptionCompleterDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    Q_UNUSED(option)
    QLineEdit* lineEdit = new QLineEdit(parent);
    QCompleter* completer = new QCompleter(lineEdit);
    if (index.column()==SolverOptionTableModel::COLUMN_OPTION_KEY) {
        completer->setModel(new QStringListModel(mOption->getValidNonDeprecatedKeyList()));
    } else  if (index.column()==SolverOptionTableModel::COLUMN_OPTION_VALUE) {
        QString key = index.model()->data( index.model()->index(index.row(), SolverOptionTableModel::COLUMN_OPTION_KEY) ).toString();
        if (!mOption->isValid(key))
            key = mOption->getNameFromSynonym(key);

        if ( mOption->getOptionType(key) == optTypeBoolean &&
             mOption->getOptionSubType(key) != optsubNoValue )
            completer->setModel(new QStringListModel( { "0", "1" } ));
        else
            completer->setModel(new QStringListModel(mOption->getNonHiddenValuesList(key)) );
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

void OptionCompleterDelegate::destroyEditor(QWidget *editor, const QModelIndex &index) const
{
    Q_UNUSED(editor)
    Q_UNUSED(index)
    mLastEditor = nullptr;
    mIsLastEditorClosed = true;
}

void OptionCompleterDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QLineEdit* lineEdit = qobject_cast<QLineEdit*>( editor ) ;
    if (lineEdit && index.isValid()) {
        QVariant data = index.model()->data( index.model()->index(index.row(), index.column()) );
        lineEdit->setText(  data.toString() ) ;
        return;
    }
    QStyledItemDelegate::setEditorData(editor, index);
}

void OptionCompleterDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    QLineEdit* lineEdit = qobject_cast<QLineEdit*>( editor ) ;
    if (lineEdit && index.isValid()) {
        model->setData( index, lineEdit->text() ) ;
        return;
    }
    QStyledItemDelegate::setModelData(editor, model, index);
}

void OptionCompleterDelegate::commitAndCloseEditor()
{
    QLineEdit *lineEdit = qobject_cast<QLineEdit *>( mLastEditor ? mLastEditor : sender() ) ;
    if (lineEdit) {
       emit commitData(lineEdit);
        if (!mIsLastEditorClosed)
           emit closeEditor(lineEdit);
        emit currentEditedIndexChanged(QModelIndex());
    }
}

void OptionCompleterDelegate::updateCurrentEditedIndex(const QModelIndex &index)
{
    mCurrentEditedIndex = index;
}

QModelIndex OptionCompleterDelegate::currentEditedIndex() const
{
    return mCurrentEditedIndex;
}

QWidget *OptionCompleterDelegate::lastEditor() const
{
    return mLastEditor;
}

bool OptionCompleterDelegate::isLastEditorClosed() const
{
    return mIsLastEditorClosed;
}

bool OptionCompleterDelegate::eventFilter(QObject* editor, QEvent* event)
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

} // namespace option
} // namespace studio
} // namespace gams
