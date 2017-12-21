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
        completer->setModel(new QStringListModel(gamsOption->getKeyList()) );
    } else {
        QTableView* tableView = qobject_cast<QTableView *>(this->parent());
        QVariant key = tableView->model()->data( tableView->model()->index(index.row(), 0) );
        if (gamsOption->isValid(key.toString())) {
            completer->setModel(new QStringListModel(gamsOption->getValuesList(key.toString())) );
        } else {
            QString keyStr = gamsOption->getSynonym(key.toString());
            completer->setModel(new QStringListModel(gamsOption->getValuesList(keyStr)) );

        }
    }
    completer->setCompletionMode(QCompleter::UnfilteredPopupCompletion);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    completer->setMaxVisibleItems(10);

    lineEdit->setCompleter(completer);

    connect(lineEdit, &QLineEdit::textChanged, this, &OptionCompleterDelegate::on_lineEdit_textChanged);

//    connect(completer,  static_cast<void (QCompleter::*)(const QString &)>(&QCompleter::activated),
//            this, &OptionCompleterDelegate::activated);
    return lineEdit;
}

void OptionCompleterDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QStyledItemDelegate::setEditorData(editor, index);
}
void OptionCompleterDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    QStyledItemDelegate::setModelData(editor, model, index);
}
void OptionCompleterDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
     QStyledItemDelegate::updateEditorGeometry(editor, option, index);
}

void OptionCompleterDelegate::on_lineEdit_textChanged(const QString &text)
{
    if (text.simplified().isEmpty()) {
        foreach(QWidget* widget, qApp->topLevelWidgets())
            if (QMainWindow*  mainWindow = qobject_cast<QMainWindow *>(widget))
                QApplication::postEvent(mainWindow, new LineEditCompleteEvent((QLineEdit*)sender()));
    }
}

//void OptionCompleterDelegate::activated(const QString &text)
//{
//    qDebug() << QString("activated [%1]").arg(text);
////    emit qobject_cast<OptionParameterModel *>(this->parent())->editCompleted();
//}

} // namespace studio
} // namespace gams
