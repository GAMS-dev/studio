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
#ifndef CONFIGPARAMEDITOR_H
#define CONFIGPARAMEDITOR_H

#include "option/optionwidget.h"
#include "option/configparamtablemodel.h"
#include "option/configoptiondefinitionmodel.h"

namespace gams {
namespace studio {
namespace option {

namespace Ui {
class OptionWidget;
}

class ConfigParamEditor : public OptionWidget
{
    Q_OBJECT
public:
    explicit ConfigParamEditor(const QList<ConfigItem *> &initParamItems,
                               const QString &encodingName,
                               QWidget *parent = nullptr);
    ~ConfigParamEditor() override;

    inline bool isModified() const   { return mModified; }
    inline void setModified(bool modified) {
        mModified = modified;
    }

    bool isInFocus(QWidget* focusWidget) const;
    QList<QHeaderView*> headers();
    QString getSelectedParameterName(QWidget* widget) const;

public slots:
    void parameterItemCommitted(QWidget *editor);
    void on_reloadGamsUserConfigFile(const QList<ConfigItem  *> &initParams);

    QList<ConfigItem *> parameterConfigItems();

    void on_dataItemChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles);

protected slots:
    void insertOption() override;
    void insertComment() override                     { return;               };
    void deleteCommentsBeforeOption(int row) override { Q_UNUSED(row) return; };
    void deleteOption() override;
    void moveOptionUp() override;
    void moveOptionDown() override;

    bool isCommentToggleable() override   { return false; }
    void updateTableColumnSpan() override { return;       }
    void refreshOptionTableModel(bool hideAllComments=true) override {
        Q_UNUSED(hideAllComments)
        return;
    };

    void addOptionModelFromDefinition(int row, const QModelIndex &definitionIndex,
                                               const QModelIndex &parentDefinitionIndex) override;
    void addCommentModelFromDefinition(int row, const QModelIndex &descriptionIndex)   override {
        Q_UNUSED(row)  Q_UNUSED(descriptionIndex)
        return;
    }
    void addEOLCommentModelFromDefinition(int row, const QModelIndex &selectedValueIndex,
                                                   const QModelIndex &descriptionIndex) override {
        Q_UNUSED(row)  Q_UNUSED(selectedValueIndex)  Q_UNUSED(descriptionIndex)
        return;
    }

protected:
    OptionTokenizer* optionTokenizer() const override { return mOptionTokenizer; }
    OptionTableModel* optionModel() const override    { return mParameterTableModel; }

    OptionSortFilterProxyModel* definitionProxymodel() const override        { return mDefinitionProxymodel;  }
    void setDefinitionProxyModel(OptionSortFilterProxyModel *model) override { mDefinitionProxymodel = model; }

    OptionDefinitionModel* definitionModel() const override         { return mDefinitionModel;                                              }
    void setDefinitionModel(OptionDefinitionModel* model) override  { mDefinitionModel = dynamic_cast<ConfigOptionDefinitionModel*>(model); }

    QStandardItemModel* definitionGroupModel() const override          { return mDefinitionGroupModel;  }
    void setDefinitionGroupModel( QStandardItemModel* model ) override { mDefinitionGroupModel = model; }

    ConfigParamTableModel* mParameterTableModel;
    OptionSortFilterProxyModel* mDefinitionProxymodel;
    ConfigOptionDefinitionModel* mDefinitionModel;
    QStandardItemModel* mDefinitionGroupModel;

    OptionTokenizer* mOptionTokenizer;

    QString mEncoding = "UTF-8";
    FileKind mFileKind = FileKind::None;
    FileId mFileId = 0;

    bool mModified = false;
    bool mFileHasChangedExtern = false;
};


} // namepsace option
} // namespace studio
} // namespace gams

#endif // CONFIGPARAMEDITOR_H
