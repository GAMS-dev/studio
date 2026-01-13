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
#ifndef OPTIONWIDGET_H
#define OPTIONWIDGET_H

#include <QMenu>
#include <QToolBar>
#include <QStandardItemModel>

#include "abstractview.h"
#include "editors/systemlogedit.h"
#include "option/optiontokenizer.h"
#include "option/optionsortfilterproxymodel.h"
#include "option/optiondefinitionmodel.h"
#include "option/optiontablemodel.h"
#include "option/optionitemdelegate.h"

namespace gams {
namespace studio {

class MainWindow;

namespace option {
namespace newoption {

namespace Ui {
class OptionWidget;
}

class OptionWidget : public AbstractView
{
    Q_OBJECT
public:
    explicit OptionWidget(bool isFileEditor, QWidget* parent = nullptr);
    ~OptionWidget() override;

protected:
   void initActions();
   void initToolBar();
   void initOptionTableView();
   void initOptionTableControl();
   void initDefintionTreeView();
   void initTabNavigation(bool visible=false);
   void initMessageControl(bool visible=false);

   bool isThereARow() const;
   bool isThereAnIndexSelection() const;
   bool isThereARowSelection() const;
   bool isEverySelectionARow() const;
   bool isEachRowSelected() const;

   MainWindow* getMainWindow() const;

public slots:
    void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected);

    void selectAllOptions();
    void deSelectOptions();

    int getItemCount() const;
    bool isInFocus(QWidget* focusWidget) const;

    QString getSelectedOptionName(QWidget* widget) const;
    QStringList getEnabledContextActions();

    void copyAction();

    void selectSearchField() const;

protected slots:
    virtual void insertOption()   = 0;
    virtual void insertComment()  = 0;
    virtual void deleteCommentsBeforeOption(int row) = 0;
    virtual void deleteOption()   = 0;
    virtual void moveOptionUp()   = 0;
    virtual void moveOptionDown() = 0;

    virtual void on_openAsTextButton_clicked(bool checked = false) {
        Q_UNUSED(checked)
        if (!mIsFileEditor)
            return;
    }
    virtual void on_compactViewCheckBox_stateChanged(int checkState) {
        Q_UNUSED(checkState)
        if (!mIsFileEditor)
            return;
    }

    virtual void on_messageViewCheckBox_stateChanged(int checkState) {
        Q_UNUSED(checkState)
        if (!mIsFileEditor)
            return;
    }

    virtual bool isCommentToggleable() = 0;
    virtual void updateTableColumnSpan() = 0;
    virtual void refreshOptionTableModel(bool hideAllComments=true) = 0;

    virtual void addOptionModelFromDefinition(int row, const QModelIndex &definitionIndex,
                                                       const QModelIndex &parentDefinitionIndex)   = 0;
    virtual void addCommentModelFromDefinition(int row, const QModelIndex &descriptionIndex) = 0;
    virtual void addEOLCommentModelFromDefinition(int row, const QModelIndex &selectedValueIndex,
                                                           const QModelIndex &descriptionIndex    ) = 0;

    void addOptionFromDefinition(const QModelIndex &definitionIndex);

    void showOptionContextMenu(const QPoint &pos);
    void showDefinitionContextMenu(const QPoint &pos);
    void findAndSelectionOptionFromDefinition();

    void completeEditingOption(QWidget *editor, QAbstractItemDelegate::EndEditHint hint = QStyledItemDelegate::NoHint);

    void showOptionDefinition(bool selectRow = true);
    void showOptionRecurrence();

    void on_selectRow(int logicalIndex) const;
    void selectAnOption() const;

    bool isViewCompact() const;

    void updateActionsState();
    void updateActionsState(const QModelIndex &index);
    void updateDefinitionActionsState(const QModelIndex &index);

    QList<int> getRecurrentOption(const QModelIndex &index);

    void copyDefinitionToClipboard(int column);

public slots:
    void on_newTableRowDropped(const QModelIndex &index);

    void on_actionInsert_triggered() { insertOption(); }
    void on_actionInsert_Comment_triggered() { insertComment(); }
    void on_actionDelete_triggered() { deleteOption(); }
    void on_actionMoveUp_triggered() { moveOptionUp(); }
    void on_actionMoveDown_triggered() { moveOptionDown(); }

    void on_actionSelect_Current_Row_triggered();
    void on_actionSelectAll_triggered();
    void on_actionShowRecurrence_triggered();
    void on_actionResize_Columns_To_Contents_triggered();
    void on_actionShow_Option_Definition_triggered();

    void on_actionAdd_This_Parameter_triggered();
    void on_actionRemove_This_Parameter_triggered();

    void currentTableIndexChanged(const QModelIndex &index);

signals:
    void modificationChanged(bool modifiedState);
    void itemCountChanged(int newItemCount);

protected:
    Ui::OptionWidget *ui;
    QToolBar* mToolBar;

    virtual OptionTokenizer* optionTokenizer() const = 0;
    virtual OptionTableModel* optionModel() const = 0;

    virtual OptionSortFilterProxyModel* definitionProxymodel() const = 0;
    virtual void setDefinitionProxyModel(OptionSortFilterProxyModel *model) = 0;

    virtual OptionDefinitionModel* definitionModel() const = 0;
    virtual void setDefinitionModel(OptionDefinitionModel* model) = 0;

    virtual QStandardItemModel* definitionGroupModel() const = 0;
    virtual void setDefinitionGroupModel( QStandardItemModel* model) = 0;

    OptionItemDelegate* optionCompleter() const            { return mOptionCompleter;      }
    void setOptionCompleter(OptionItemDelegate* completer) { mOptionCompleter = completer; }

    OptionItemDelegate* mOptionCompleter;

    bool mIsFileEditor = false;
    SystemLogEdit *mLogEdit = nullptr;
};


} // namepsace newoption
} // namepsace option
} // namespace studio
} // namespace gams

#endif // OPTIONWIDGET_H
