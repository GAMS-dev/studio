#ifndef OPTIONEDITOR_H
#define OPTIONEDITOR_H

#include <QtWidgets>

#include "commandlineoption.h"
#include "commandlinetokenizer.h"
#include "option.h"
#include "optionsortfilterproxymodel.h"
#include "optionparametermodel.h"
#include "ui_optionconfigurator.h"

namespace gams {
namespace studio {

class OptionEditor : public QWidget
{
    Q_OBJECT
public:
    explicit OptionEditor(CommandLineOption* option, CommandLineTokenizer* tokenizer, QWidget *parent = nullptr);
    ~OptionEditor();

    void setupUi(QWidget* parent);

    QList<OptionItem> getCurrentListOfOptionItems();

signals:
    void optionRunWithParameterChanged(const QString &fileLocation, const QString &parameter);
    void optionTableModelChanged(const QString &commandLineStr);
    void commandLineOptionChanged(QLineEdit* lineEdit, const QString &commandLineStr);
    void commandLineOptionChanged(QLineEdit* lineEdit, const QList<OptionItem> &opionItems);

public slots:
//    void toggleOptionDefinition(bool checked);
    void updateTableModel(QLineEdit* lineEdit, const QString &commandLineStr);
    void updateCommandLineStr(const QString &commandLineStr);
    void updateCommandLineStr(const QList<OptionItem> &opionItems);
    void showOptionContextMenu(const QPoint &pos);

private:
    CommandLineOption* mCommandLineOption;
    CommandLineTokenizer* mTokenizer;

    QVBoxLayout *verticalLayout;
    QSplitter *splitter;
    QTableView *commandLineTableView;
    OptionParameterModel* optionParamModel;
    QWidget *verticalLayoutWidget;
    QVBoxLayout *optionDefinition_VLayout;
    QLineEdit *searchLineEdit;
    QTreeView *optionDefinitionTreeView;
    QHBoxLayout *button_HLayout;

};

} // namespace studio
} // namespace gams

#endif // OPTIONEDITOR_H
