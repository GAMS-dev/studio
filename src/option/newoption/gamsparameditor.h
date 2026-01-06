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
#ifndef GAMSPARAMEDITOR_H
#define GAMSPARAMEDITOR_H

#include "option/newoption/optionwidget.h"
#include "option/newoption/gamsparamtablemodel.h"
#include "option/gamsoptiondefinitionmodel.h"

namespace gams {
namespace studio {
namespace option {
namespace newoption {

namespace Ui {
class OptionWidget;
}

class GamsParamEditor : public OptionWidget
{
    Q_OBJECT
public:
    explicit GamsParamEditor(const QString &commandLineParameter,
                             OptionTokenizer* tokenizer,
                             QWidget *parent = nullptr);

    ~GamsParamEditor() override;

    void setEditorExtended(bool extended);
    bool isEditorExtended();
    void focus();
    bool hasFocus();
    bool hasSelection();

    QString getSelectedParameterName(QWidget* widget) const;

public slots:
    void on_ParameterTableModelChanged(const QString &text);
    void on_parameterTableNameChanged(const QString &from, const QString &to);
    void on_parameterValueChanged(const QModelIndex &index);

    void deSelectParameters();

protected slots:
    void insertOption() override;
    void insertComment() override                     {               };
    void deleteCommentsBeforeOption(int row) override { Q_UNUSED(row) };
    void deleteOption() override;
    void moveOptionUp() override;
    void moveOptionDown()  override;

    void on_openAsTextButton_clicked(bool checked = false) override   { Q_UNUSED(checked)    return; };
    void on_compactViewCheckBox_stateChanged(int checkState) override { Q_UNUSED(checkState) return; };
    void on_messageViewCheckBox_stateChanged(int checkState) override { Q_UNUSED(checkState) return; };

    bool isCommentToggleable() override   { return false; };
    void updateTableColumnSpan() override { return;       };
    void refreshOptionTableModel(bool hideAllComments=true) override {
        Q_UNUSED(hideAllComments)
        return;
    };

    void addOptionModelFromDefinition(int row, const QModelIndex &definitionIndex,
                                               const QModelIndex &parentDefinitionIndex) override;
    void addCommentModelFromDefinition(int row, const QModelIndex &descriptionIndex)     override {
        Q_UNUSED(row)  Q_UNUSED(descriptionIndex)
        return;
    }
    void addEOLCommentModelFromDefinition(int row, const QModelIndex &selectedValueIndex,
                                          const QModelIndex &descriptionIndex) override {
        Q_UNUSED(row)  Q_UNUSED(selectedValueIndex)  Q_UNUSED(descriptionIndex)
        return;
    }

    void clearDefintionSelection();
    void clearOptionSelection();

    void parameterItemCommitted(const QModelIndex &index);

protected:
    friend class GamsParameterWidget;

    OptionTokenizer* optionTokenizer() const override { return mOptionTokenizer; }
    OptionTableModel* optionModel() const override    { return mOptionModel; }

    OptionSortFilterProxyModel* definitionProxymodel() const override        { return mDefinitionProxymodel;  }
    void setDefinitionProxyModel(OptionSortFilterProxyModel *model) override { mDefinitionProxymodel = model; }

    OptionDefinitionModel* definitionModel() const override         { return mDefinitionModel;                                            }
    void setDefinitionModel(OptionDefinitionModel* model) override  { mDefinitionModel = dynamic_cast<GamsOptionDefinitionModel*>(model); }

    QStandardItemModel* definitionGroupModel() const override          { return mDefinitionGroupModel;  }
    void setDefinitionGroupModel( QStandardItemModel* model ) override { mDefinitionGroupModel = model; }

    GamsParamTableModel* mOptionModel;
    OptionSortFilterProxyModel* mDefinitionProxymodel;
    GamsOptionDefinitionModel* mDefinitionModel;
    QStandardItemModel* mDefinitionGroupModel;

    OptionTokenizer* mOptionTokenizer;

    bool mExtended = false;
};



} // namepsace newoption
} // namepsace option
} // namespace studio
} // namespace gams

#endif // GAMSPARAMEDITOR_H
