#ifndef OPTIONCOMPLETERDELEGATE_H
#define OPTIONCOMPLETERDELEGATE_H

#include <QtWidgets>
#include "commandlinetokenizer.h"
#include "option.h"
#include "optionparametermodel.h"

namespace gams {
namespace studio {

class OptionCompleterDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    OptionCompleterDelegate(CommandLineTokenizer* tokenizer, QObject* parent = 0);

    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const;
    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;
//    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelInd*/ex &index) const;

protected:
    virtual bool eventFilter(QObject * editor, QEvent * event) override;

private slots:
    void on_lineEdit_textChanged(const QString &text);
    void commitAndCloseEditor();

private:
    CommandLineTokenizer* commandLineTokenizer;
    Option* gamsOption;
};

} // namespace studio
} // namespace gams

#endif // OPTIONCOMPLETERDELEGATE_H
