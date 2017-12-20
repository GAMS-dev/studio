#include "optioncompleterdelegate.h"

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

        completer->setModel(new QStringListModel(gamsOption->getValuesList("action")) );
    }
    completer->setCompletionMode(QCompleter::PopupCompletion);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
//    completer->setMaxVisibleItems(10);

    lineEdit->setCompleter(completer);
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

} // namespace studio
} // namespace gams
