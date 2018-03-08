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
#include <QKeyEvent>

#include "lineeditcompleteevent.h"
#include "optioncompleterdelegate.h"
//#include "optionparametermodel.h"

namespace gams {
namespace studio {

OptionCompleterDelegate::OptionCompleterDelegate(CommandLineTokenizer* tokenizer, QObject* parent) :
    QStyledItemDelegate(parent), commandLineTokenizer(tokenizer), gamsOption(tokenizer->getGamsOption())
{
}

QWidget* OptionCompleterDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    QLineEdit* lineEdit = new QLineEdit(parent);
    QCompleter* completer = new QCompleter(lineEdit);
    if (index.column()==0) {
        completer->setModel(new QStringListModel(gamsOption->getValidNonDeprecatedKeyList()));
    } else {
        QVariant key = index.model()->data( index.model()->index(index.row(), 0) );
        if (gamsOption->isValid(key.toString())) {
            completer->setModel(new QStringListModel(gamsOption->getNonHiddenValuesList(key.toString())) );
        } else {
            QString keyStr = gamsOption->getSynonym(key.toString());
            completer->setModel(new QStringListModel(gamsOption->getNonHiddenValuesList(keyStr)) );
        }
    }
    completer->setCompletionMode(QCompleter::UnfilteredPopupCompletion);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    completer->setMaxVisibleItems(10);

    lineEdit->setCompleter(completer);
    lineEdit->adjustSize();

    connect( lineEdit, &QLineEdit::editingFinished, this, &OptionCompleterDelegate::commitAndCloseEditor) ;
    connect(lineEdit, &QLineEdit::textChanged, this, &OptionCompleterDelegate::on_lineEdit_textChanged);
    return lineEdit;
}

void OptionCompleterDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QLineEdit* lineEdit = qobject_cast<QLineEdit*>( editor ) ;
    if (lineEdit) {
        QVariant data = index.model()->data( index.model()->index(index.row(), index.column()) );
        lineEdit->setText(  data.toString() ) ;
        return;
    }
    QStyledItemDelegate::setEditorData(editor, index);
}

void OptionCompleterDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    QLineEdit* lineEdit = qobject_cast<QLineEdit*>( editor ) ;
    if (lineEdit) {
        model->setData( index, lineEdit->text() ) ;
        return;
    }
    QStyledItemDelegate::setModelData(editor, model, index);
}

void OptionCompleterDelegate::on_lineEdit_textChanged(const QString &text)
{
    if (text.simplified().isEmpty()) {
        foreach(QWidget* widget, qApp->topLevelWidgets())
            if (QMainWindow*  mainWindow = qobject_cast<QMainWindow *>(widget))
                QApplication::postEvent(mainWindow, new LineEditCompleteEvent((QLineEdit*)sender()));
    }
}

void OptionCompleterDelegate::commitAndCloseEditor()
{
    QLineEdit *lineEdit = qobject_cast<QLineEdit *>( sender() ) ;

    emit commitData(lineEdit);
    emit closeEditor(lineEdit);
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
       } else if ((keyEvent->key() == Qt::Key_Tab) || (keyEvent->key() == Qt::Key_Enter)) {
             emit commitData(lineEdit);
             emit closeEditor(lineEdit, QAbstractItemDelegate::EditNextItem);
       }
       return false;
    }
   return QStyledItemDelegate::eventFilter(editor, event);
}

} // namespace studio
} // namespace gams
