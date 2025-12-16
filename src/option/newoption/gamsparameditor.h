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
#ifndef GAMSPARAMEDITOR_H
#define GAMSPARAMEDITOR_H

#include "optionwidget.h"
#include "option/newoption/optiontablemodel.h"
#include "option/newoption/solveroptiontablemodel.h"
#include "option/solveroptiondefinitionmodel.h"

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
                             QWidget *parent = nullptr);

    ~GamsParamEditor() override;

protected:
    OptionTokenizer* optionTokenizer() const override { return mOptionTokenizer; }
    OptionTableModel* optionModel() const override    { return mOptionModel; }

    OptionSortFilterProxyModel* definitionProxymodel() const override        { return mDefinitionProxymodel;  }
    void setDefintionProxyModel(OptionSortFilterProxyModel *model) override  { mDefinitionProxymodel = model; }

    OptionDefinitionModel* definitionModel() const override { return mDefinitionModel; }

    QStandardItemModel* definitionGroupModel() const override          { return mDefinitionGroupModel;  }
    void setDefinitionGroupModel( QStandardItemModel* model ) override { mDefinitionGroupModel = model; }


    OptionItemDelegate* optionCompleter() const override            { return mOptionCompleter;      }
    void setOptionCompleter(OptionItemDelegate* completer) override { mOptionCompleter = completer; }

    SolverOptionTableModel* mOptionModel;
    OptionSortFilterProxyModel* mDefinitionProxymodel;
    SolverOptionDefinitionModel* mDefinitionModel;
    QStandardItemModel* mDefinitionGroupModel;

    OptionTokenizer* mOptionTokenizer;
    OptionItemDelegate* mOptionCompleter;
};



} // namepsace newoption
} // namepsace option
} // namespace studio
} // namespace gams

#endif // GAMSPARAMEDITOR_H
