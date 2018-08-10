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
#ifndef REFERENCEWIDGET_H
#define REFERENCEWIDGET_H

#include <QWidget>
#include <QList>
#include <QMap>
#include <QTabWidget>

#include "reference.h"
//#include "symbolreferenceitem.h"
#include "symbolreferencewidget.h"

namespace Ui {
class ReferenceViewer;
}

namespace gams {
namespace studio {

class ReferenceViewer : public QWidget
{
    Q_OBJECT

public:
    explicit ReferenceViewer(QString referenceFile, QWidget *parent = nullptr);
    ~ReferenceViewer();

signals:
    void symbolSelectedChanged(SymbolId SymbolId);

private:
    Ui::ReferenceViewer *ui;

    QTabWidget* mTabWidget;
};

} // namespace studio
} // namespace gams

#endif // REFERENCEWIDGET_H
