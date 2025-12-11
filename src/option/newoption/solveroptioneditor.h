/**
 * GAMS Studio
 *
 * Copyright (c) 2017-2025 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2025 GAMS Development Corp. <support@gams.com>
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
#ifndef SOLVEROPTIONEDITOR_H
#define SOLVEROPTIONEDITOR_H

#include "optionwidget.h"
#include "option/newoption/solveroptiontablemodel.h"
#include "option/solveroptiondefinitionmodel.h"

namespace gams {
namespace studio {
namespace option {
namespace newoption {

namespace Ui {
class OptionWidget;
}

class SolverOptionEditor : public OptionWidget
{
    Q_OBJECT
public:
    SolverOptionEditor(const QString &solverName,
                       const QString &optionFilePath,
                       const QString &optDefFileName,
                       const FileKind &kind,
                       const FileId &id,
                       const QString &encodingName,
                       QWidget *parent = nullptr);

    ~SolverOptionEditor() override;

    inline FileKind fileKind() const { return mFileKind; }
    inline FileId fileId() const     { return mFileId;   }

    inline bool isModified() const   { return mModified; }
    inline void setModified(bool modified) {
        mModified = modified;
        emit modificationChanged( mModified );
    }

    inline void setFileChangedExtern(bool value) {
        mFileHasChangedExtern = value;
    }

    bool saveAs(const QString &location);

    QString getSolverName() const;

    void toggleCommentOption();

signals:
    void modificationChanged(bool modifiedState);
    void itemCountChanged(int newItemCount);
    void compactViewChanged(bool compact);

public slots:
    void showOptionContextMenu(const QPoint &pos) override;
    void showDefinitionContextMenu(const QPoint &pos) override;
    void addOptionFromDefinition(const QModelIndex &index) override;

    void on_dataItemChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles);
    void on_newTableRowDropped(const QModelIndex &index);

    bool saveOptionFile(const QString &location);

    void on_reloadSolverOptionFile(const QString &encodingName);
    void on_selectAndToggleRow(int logicalIndex);
    void on_toggleRowHeader(int logicalIndex);

protected slots:
    void on_compactViewCheckBox_stateChanged(int checkState) override;
    void on_messageViewCheckBox_stateChanged(int checkState) override;
    void on_openAsTextButton_clicked(bool checked = false) override;

    void findAndSelectionOptionFromDefinition() override;

    void insertOption() override;
    void insertComment() override;
    void deleteCommentsBeforeOption(int row) override;
    void deleteOption() override;
    void moveOptionUp() override;
    void moveOptionDown() override;

    bool isCommentToggleable() override { return false; }

protected:
    OptionTableModel* optionModel() override;
    void setOptionTableModel( OptionTableModel* model ) override;

    OptionSortFilterProxyModel* definitionProxymodel() override;
    void setDefintionProxyModel( OptionSortFilterProxyModel* model ) override;

    OptionDefinitionModel* definitionModel() override;
    void setDefinitionModel( OptionDefinitionModel* definitionModel ) override;

    QStandardItemModel* definitionGroupModel() override;
    void setDefinitionGroupModel( QStandardItemModel* model ) override;

    QString getOptionTableEntry(int row);
    bool isEditing();

    void refreshOptionTableModel(bool hideAllComments);
    void updateTableColumnSpan();

private:
    SolverOptionTableModel* mOptionModel;
    OptionSortFilterProxyModel* mDefinitionProxymodel;
    SolverOptionDefinitionModel* mDefinitionModel;
    QStandardItemModel* mDefinitionGroupModel;

    QString mEncoding = "UTF-8";

    QString mSolverName;
    FileKind mFileKind = FileKind::None;
    FileId mFileId = 0;

    bool mModified = false;
    bool mFileHasChangedExtern = false;

};

} // namepsace newoption
} // namepsace option
} // namespace studio
} // namespace gams

#endif // SOLVEROPTIONEDITOR_H
