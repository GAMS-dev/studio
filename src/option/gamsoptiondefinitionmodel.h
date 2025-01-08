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
#ifndef GAMSOPTIONDEFINITIONMODEL_H
#define GAMSOPTIONDEFINITIONMODEL_H

#include "optiondefinitionmodel.h"

namespace gams {
namespace studio {
namespace option {

class GamsOptionDefinitionModel : public OptionDefinitionModel
{
    Q_OBJECT
public:
    GamsOptionDefinitionModel(Option* data, int optionGroup=0, QObject* parent=nullptr);

    QStringList mimeTypes() const override;
    QMimeData* mimeData(const QModelIndexList & indexes) const override;

public slots:
    void modifyOptionDefinitionItem(const gams::studio::option::OptionItem &optionItem);
    void modifyOptionDefinition(const QList<gams::studio::option::OptionItem> &optionItems);

};

} // namespace option
} // namespace studio
} // namespace gams

#endif // GAMSOPTIONDEFINITIONMODEL_H
