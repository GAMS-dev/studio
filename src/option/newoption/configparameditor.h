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
#ifndef CONFIGPARAMEDITOR_H
#define CONFIGPARAMEDITOR_H

#include "option/newoption/optionwidget.h"
#include "option/newoption/configtablemodel.h"
#include "option/configoptiondefinitionmodel.h"

namespace gams {
namespace studio {
namespace option {
namespace newoption {

namespace Ui {
class OptionWidget;
}

class ConfigParamEditor : public OptionWidget
{
    Q_OBJECT
public:
    ConfigParamEditor(const QList<ParamConfigItem *> &initParamItems,
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
    void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected);

    void showOptionContextMenu(const QPoint &pos) override;
    void showDefinitionContextMenu(const QPoint &pos) override;
    void addOptionFromDefinition(const QModelIndex &index) override;

    void parameterItemCommitted(QWidget *editor);
    void on_reloadGamsUserConfigFile(const QList<ParamConfigItem  *> &initParams);

//    QList<ParamConfigItem *> parameterConfigItems();

    void on_dataItemChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles);
    void on_newTableRowDropped(const QModelIndex &index);

    QString getParameterTableEntry(int row);

protected slots:
    void insertOption() override;
    void insertComment() override                     {               };
    void deleteCommentsBeforeOption(int row) override { Q_UNUSED(row) };
    void deleteOption() override;
    void moveOptionUp() override;
    void moveOptionDown() override;

    void findAndSelectionOptionFromDefinition() override;

protected:
    OptionTokenizer* optionTokenizer() const override { return mOptionTokenizer; }
    OptionTableModel* optionModel() const override    { return mParameterTableModel; }

    OptionSortFilterProxyModel* definitionProxymodel() const override        { return mDefinitionProxymodel;  }
    void setDefintionProxyModel(OptionSortFilterProxyModel *model) override  { mDefinitionProxymodel = model; }

    OptionDefinitionModel* definitionModel() const override { return mDefinitionModel; }

    QStandardItemModel* definitionGroupModel() const override          { return mDefinitionGroupModel;  }
    void setDefinitionGroupModel( QStandardItemModel* model ) override { mDefinitionGroupModel = model; }


    OptionItemDelegate* optionCompleter() const override            { return mOptionCompleter;      }
    void setOptionCompleter(OptionItemDelegate* completer) override { mOptionCompleter = completer; }

    ConfigTableModel* mParameterTableModel;
    OptionSortFilterProxyModel* mDefinitionProxymodel;
    ConfigOptionDefinitionModel* mDefinitionModel;
    QStandardItemModel* mDefinitionGroupModel;

    OptionTokenizer* mOptionTokenizer;
    OptionItemDelegate* mOptionCompleter;

    QString mEncoding = "UTF-8";
    FileKind mFileKind = FileKind::None;
    FileId mFileId = 0;

    bool mModified = false;
    bool mFileHasChangedExtern = false;
};


} // namepsace newoption
} // namepsace option
} // namespace studio
} // namespace gams

#endif // CONFIGPARAMEDITOR_H
