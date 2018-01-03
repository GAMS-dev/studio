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
 qDebug() << "CompleterDelegate::createEditor() called for" << parent << index << index.data(Qt::EditRole);
    QLineEdit* lineEdit = new QLineEdit(parent);
    QCompleter* completer = new QCompleter(lineEdit);
    if (index.column()==0) {
        completer->setModel(new QStringListModel(gamsOption->getKeyList()) );
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
//    connect(completer,  static_cast<void (QCompleter::*)(const QString &)>(&QCompleter::activated),
//            this, &OptionCompleterDelegate::activated);
    return lineEdit;
}

void OptionCompleterDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    qDebug() << "CompleterDelegate::setEditorData() called for" << index << index.data(Qt::EditRole);

    QLineEdit* lineEdit = qobject_cast<QLineEdit*>( editor ) ;
    if (lineEdit) {
        QVariant data = index.model()->data( index.model()->index(index.row(), index.column()) );
        lineEdit->setText(  data.toString() ) ;
        qDebug() << QString("    [%1]").arg(data.toString());
        return;
    }
    QStyledItemDelegate::setEditorData(editor, index);
}

void OptionCompleterDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    qDebug() << "CompleterDelegate::setModelData() called for" << index << index.data(Qt::EditRole);
    QLineEdit* lineEdit = qobject_cast<QLineEdit*>( editor ) ;
    if (lineEdit) {
        model->setData( index, lineEdit->text() ) ;
        return;
    }
    QStyledItemDelegate::setModelData(editor, model, index);
}

//void OptionCompleterDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
//{
//    qDebug() << "CompleterDelegate::setModelData() called for" << index << index.data(Qt::EditRole);
//     QStyledItemDelegate::updateEditorGeometry(editor, option, index);
//}

void OptionCompleterDelegate::on_lineEdit_textChanged(const QString &text)
{
    qDebug() << QString("on_lineEdit_textChanged(%1)").arg(text);

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

//void OptionCompleterDelegate::activated(const QString &text)
//{
//    qDebug() << QString("activated [%1]").arg(text);
////    emit qobject_cast<OptionParameterModel *>(this->parent())->editCompleted();
//}

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
