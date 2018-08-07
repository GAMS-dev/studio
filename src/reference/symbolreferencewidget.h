/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2018 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2018 GAMS Development Corp. <support@gams.com>
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
#ifndef SYMBOLREFERENCEWIDGET_H
#define SYMBOLREFERENCEWIDGET_H

#include <QWidget>
#include "symboldatatype.h"

namespace Ui {
class SymbolReferenceWidget;
}

namespace gams {
namespace studio {

class SymbolReferenceWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SymbolReferenceWidget(SymbolDataType type, QWidget *parent = nullptr);
    ~SymbolReferenceWidget();

private:
    Ui::SymbolReferenceWidget *ui;

    SymbolDataType mType;
};

} // namespace lxiviewer
} // namespace studio

#endif // SYMBOLREFERENCEWIDGET_H
