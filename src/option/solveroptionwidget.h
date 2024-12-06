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
#ifndef SOLVEROPTIONWIDGET_H
#define SOLVEROPTIONWIDGET_H

#include <QMenu>
#include <QWidget>
#include <QStyledItemDelegate>

#include "abstractview.h"
#include "common.h"
#include "optioncompleterdelegate.h"
#include "solveroptiontablemodel.h"

namespace gams {
namespace studio {

class MainWindow;

namespace option {

namespace Ui {
class SolverOptionWidget;
}

class OptionTokenizer;

class SolverOptionWidget : public AbstractView
{
    Q_OBJECT

public:
    explicit SolverOptionWidget(const QString &solverName, const QString &optionFilePath, const QString &optDefFileName, const FileKind &kind,
                                const FileId &id, const QString &encodingName, QWidget *parent = nullptr);
    ~SolverOptionWidget() override;

    FileKind fileKind() const;
    FileId fileId() const;

    bool isModified() const;
    void setModified(bool modified);

    bool saveAs(const QString &location);

    bool isInFocus(QWidget* focusWidget) const;
    QString getSelectedOptionName(QWidget* widget) const;

    QString getSolverName() const;
    int getItemCount() const;

    bool isViewCompact() const;

    void selectSearchField() const;
    void setFileChangedExtern(bool value);

    void toggleCommentOption();

signals:
    void modificationChanged(bool modifiedState);
    void itemCountChanged(int newItemCount);
    void compactViewChanged(bool compact);

public slots:
    void showOptionContextMenu(const QPoint &pos);
    void showDefinitionContextMenu(const QPoint &pos);
    void addOptionFromDefinition(const QModelIndex &index);

    void on_dataItemChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles);
    void on_newTableRowDropped(const QModelIndex &index);

    bool saveOptionFile(const QString &location);

    void on_reloadSolverOptionFile(const QString &encodingName);
    void on_selectRow(int logicalIndex);
    void on_selectAndToggleRow(int logicalIndex);
    void on_toggleRowHeader(int logicalIndex);

    void on_compactViewCheckBox_stateChanged(int checkState);
    void on_messageViewCheckBox_stateChanged(int checkState);
    void on_openAsTextButton_clicked(bool checked = false);

    void copyAction();

    void selectAllOptions();
    void deSelectOptions();

    void completeEditingOption(QWidget *editor, QAbstractItemDelegate::EndEditHint hint = QStyledItemDelegate::NoHint);

private slots:
    void showOptionDefinition(bool selectRow = true);
    void showOptionRecurrence();
    void copyDefinitionToClipboard(int column);
    void findAndSelectionOptionFromDefinition();
    void selectAnOption();
    void insertOption();
    void insertComment();
    void deleteCommentsBeforeOption(int row);
    void deleteOption();
    void moveOptionUp();
    void moveOptionDown();

    void resizeColumnsToContents();

private:
    QList<int> getRecurrentOption(const QModelIndex &index);
    QString getOptionTableEntry(int row);
    bool isEditing();

    Ui::SolverOptionWidget *ui;
    FileKind mFileKind;
    FileId mFileId;
    QString mLocation;
    QString mSolverName;

    bool mFileHasChangedExtern = false;

    QString mEncoding = "UTF-8";

    SolverOptionTableModel* mOptionTableModel;
    QMenu mContextMenu;

    bool mModified;
    OptionTokenizer* mOptionTokenizer;
    OptionCompleterDelegate* mOptionCompleter;

    void refreshOptionTableModel(bool hideAllComments);

    void addActions();
    void updateEditActions(bool modified);
    void updateTableColumnSpan();

    bool isThereARow() const;
    bool isThereARowSelection() const;
    bool isEverySelectionARow() const;

    bool init(const QString &optDefFileName);

    MainWindow* getMainWindow();

};


} // namespace option
} // namespace studio
} // namespace gams
#endif // SOLVEROPTIONWIDGET_H
