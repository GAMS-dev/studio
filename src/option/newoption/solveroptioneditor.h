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

#include "option/newoption/optionwidget.h"
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

    inline bool isModified() const         { return mModified;     }
    inline void setModified(bool modified) { mModified = modified; }

    inline void setFileChangedExtern(bool value) { mFileHasChangedExtern = value;  }

    bool saveAs(const QString &location);

    QString getSolverName() const;

    void toggleCommentOption();

signals:
    void compactViewChanged(bool compact);

public slots:
    void on_dataItemChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles);

    bool saveOptionFile(const QString &location);

    void on_reloadSolverOptionFile(const QString &encodingName);
    void on_selectAndToggleRow(int logicalIndex);
    void on_toggleRowHeader(int logicalIndex);

protected slots:
    void on_compactViewCheckBox_stateChanged(int checkState) override;
    void on_messageViewCheckBox_stateChanged(int checkState) override;
    void on_openAsTextButton_clicked(bool checked = false) override;

    void insertOption() override;
    void insertComment() override;
    void deleteCommentsBeforeOption(int row) override;
    void deleteOption() override;
    void moveOptionUp() override;
    void moveOptionDown() override;

    bool isCommentToggleable() override { return true; }

protected:
    OptionTokenizer* optionTokenizer() const override { return mOptionTokenizer; }
    OptionTableModel* optionModel() const override    { return mOptionModel; }

    OptionSortFilterProxyModel* definitionProxymodel() const override        { return mDefinitionProxymodel;  }
    void setDefinitionProxyModel(OptionSortFilterProxyModel *model) override { mDefinitionProxymodel = model; }

    OptionDefinitionModel* definitionModel() const override         { return mDefinitionModel;                                              }
    void setDefinitionModel(OptionDefinitionModel* model) override  { mDefinitionModel = dynamic_cast<SolverOptionDefinitionModel*>(model); }

    QStandardItemModel* definitionGroupModel() const override          { return mDefinitionGroupModel;  }
    void setDefinitionGroupModel( QStandardItemModel* model ) override { mDefinitionGroupModel = model; }

    OptionItemDelegate* optionCompleter() const override            { return mOptionCompleter;      }
    void setOptionCompleter(OptionItemDelegate* completer) override { mOptionCompleter = completer; }

    bool isEditing();

    void updateTableColumnSpan() override;
    void refreshOptionTableModel(bool hideAllComments=true) override;

    void addOptionModelFromDefinition(int row, const QModelIndex &descriptionIndex)    override;
    void addCommentModelFromDefinition(int row, const QModelIndex &descriptionIndex)   override;
    void addEOLCommentModelFromDefinition(int row, const QModelIndex &selectedValueIndex,
                                                   const QModelIndex &descriptionndex) override;

    SolverOptionTableModel* mOptionModel;
    OptionSortFilterProxyModel* mDefinitionProxymodel;
    SolverOptionDefinitionModel* mDefinitionModel;
    QStandardItemModel* mDefinitionGroupModel;

    OptionTokenizer* mOptionTokenizer;
    OptionItemDelegate* mOptionCompleter;

    QString mSolverName;
    QString mEncoding = "UTF-8";
    QString mLocation;
    QString mDefinitionFileName;

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
